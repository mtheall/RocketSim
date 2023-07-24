#include "Arena.h"

#include "../../RocketSim.h"

#include "../../../libsrc/bullet3-3.24/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "../../../libsrc/bullet3-3.24/BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"
#include "../../../libsrc/bullet3-3.24/BulletCollision/CollisionShapes/btBoxShape.h"
#include "../../../libsrc/bullet3-3.24/BulletCollision/CollisionShapes/btSphereShape.h"

RSAPI void Arena::SetMutatorConfig(const MutatorConfig& mutatorConfig) {

	bool
		ballRadiusChanged = mutatorConfig.ballRadius != this->_mutatorConfig.ballRadius,
		ballMassChanged = mutatorConfig.ballMass != this->_mutatorConfig.ballMass,
		carMassChanged = mutatorConfig.carMass != this->_mutatorConfig.carMass,
		gravityChanged = mutatorConfig.gravity != this->_mutatorConfig.gravity;

	this->_mutatorConfig = mutatorConfig;

	_bulletWorld.setGravity(mutatorConfig.gravity * UU_TO_BT);
	
	if (ballRadiusChanged) {
		// We'll need to remake the ball
		_bulletWorld.removeCollisionObject(&ball->_rigidBody);
		ball->_BulletSetup(&_bulletWorld, mutatorConfig);
	} else if (ballMassChanged) {
		btVector3 newBallInertia;
		ball->_collisionShape.calculateLocalInertia(mutatorConfig.ballMass, newBallInertia);
		ball->_rigidBody.setMassProps(mutatorConfig.ballMass, newBallInertia);
	}

	if (carMassChanged) {
		for (Car* car : _cars) {
			btVector3 newCarInertia;
			car->_childHitboxShape.calculateLocalInertia(mutatorConfig.carMass, newCarInertia);
			car->_rigidBody.setMassProps(mutatorConfig.ballMass, newCarInertia);
		}
	}

	// Update ball rigidbody physics values for world contact
	// NOTE: Cars don't use their rigidbody physics values for world contact
	ball->_rigidBody.setFriction(mutatorConfig.ballWorldFriction);
	ball->_rigidBody.setRestitution(mutatorConfig.ballWorldRestitution);
	ball->_rigidBody.setDamping(mutatorConfig.ballDrag, 0);
}

Car* Arena::AddCar(Team team, const CarConfig& config) {
	Car* car = Car::_AllocateCar();
	
	car->config = config;
	car->team = team;
	
	_AddCarFromPtr(car);

	car->_BulletSetup(&_bulletWorld, _mutatorConfig);
	car->Respawn(-1, _mutatorConfig.carSpawnBoostAmount);

	return car;
}

bool Arena::_AddCarFromPtr(Car* car) {

	car->id = ++_lastCarID;

	if (_carIDMap.find(car->id) == _carIDMap.end()) {
		assert(!_cars.count(car));
		
		_carIDMap[car->id] = car;
		_cars.insert(car);
		return true;

	} else {
		return false;
	}
}

bool Arena::RemoveCar(uint32_t id) {
	auto itr = _carIDMap.find(id);

	if (itr != _carIDMap.end()) {
		Car* car = itr->second;
		_carIDMap.erase(itr);
		_cars.erase(car);
		_bulletWorld.removeCollisionObject(&car->_rigidBody);
		if (ownsCars)
			delete car;
		return true;
	} else {
		return false;
	}
}

Car* Arena::GetCar(uint32_t id) {
	return _carIDMap[id];
}

void Arena::SetBallTouchCallback(BallTouchEventFn callbackFunc, void* userInfo) {
	_ballTouchCallback.func = callbackFunc;
	_ballTouchCallback.userInfo = userInfo;
}

void Arena::SetBoostPickupCallback(BoostPickupEventFn callbackFunc, void* userInfo) {
	if (gameMode == GameMode::THE_VOID)
		RS_ERR_CLOSE("Cannot set a boost pickup callback when on THE_VOID gamemode!");

	_boostPickupCallback.func = callbackFunc;
	_boostPickupCallback.userInfo = userInfo;
}

void Arena::SetCarBumpCallback(CarBumpEventFn callbackFunc, void* userInfo) {
	_carBumpCallback.func = callbackFunc;
	_carBumpCallback.userInfo = userInfo;
}

void Arena::SetGoalScoreCallback(GoalScoreEventFn callbackFunc, void* userInfo) {
	if (gameMode == GameMode::THE_VOID)
		RS_ERR_CLOSE("Cannot set a goal score callback when on THE_VOID gamemode!");

	_goalScoreCallback.func = callbackFunc;
	_goalScoreCallback.userInfo = userInfo;
}

void Arena::ResetToRandomKickoff(int seed) {
	using namespace RLConst;

	// TODO: Make shuffling of kickoff setup more efficient (?)

	static thread_local std::vector<int> kickoffOrder;
	if (kickoffOrder.empty()) {
		for (int i = 0; i < CAR_SPAWN_LOCATION_AMOUNT; i++)
			kickoffOrder.push_back(i);
	}

	if (seed == -1) {
		std::default_random_engine& randEngine = Math::GetRandEngine();
		std::shuffle(kickoffOrder.begin(), kickoffOrder.end(), randEngine);
	} else {
		std::default_random_engine randEngine = std::default_random_engine(seed);
		std::shuffle(kickoffOrder.begin(), kickoffOrder.end(), randEngine);
	}

	std::vector<Car*> blueCars, orangeCars;
	for (Car* car : _cars)
		((car->team == Team::BLUE) ? blueCars : orangeCars).push_back(car);

	int kickoffPositionAmount = RS_MAX(blueCars.size(), orangeCars.size());
	for (int i = 0; i < kickoffPositionAmount; i++) {
		CarSpawnPos spawnPos = CAR_SPAWN_LOCATIONS[kickoffOrder[i]];

		for (int teamIndex = 0; teamIndex < 2; teamIndex++) {
			bool isBlue = (teamIndex == 0);
			std::vector<Car*> teamCars = isBlue ? blueCars : orangeCars;

			if (i < teamCars.size()) {
				CarState spawnState;
				spawnState.pos = { spawnPos.x, spawnPos.y, CAR_SPAWN_REST_Z };
				
				Angle angle = Angle(spawnPos.yawAng, 0, 0);
				spawnState.isOnGround = true;

				if (!isBlue) {
					spawnState.pos *= { -1, -1, 1 };
					angle.yaw += M_PI;
				}

				spawnState.rotMat = angle.ToRotMat();

				teamCars[i]->SetState(spawnState);
			}
		}
	}

	ball->SetState(BallState());

	if (gameMode == GameMode::SOCCAR) {
		for (BoostPad* boostPad : _boostPads)
			boostPad->SetState(BoostPadState());
	}
}

bool Arena::_BulletContactAddedCallback(
	btManifoldPoint& contactPoint,
	const btCollisionObjectWrapper* objA, int partID_A, int indexA,
	const btCollisionObjectWrapper* objB, int partID_B, int indexB) {

	auto
		bodyA = objA->m_collisionObject,
		bodyB = objB->m_collisionObject;

	bool shouldSwap = false;
	if ((bodyA->getUserIndex() != -1) && (bodyB->getUserIndex() != -1)) {
		// If both bodies have a user index, the lower user index should be A
		shouldSwap = bodyA->getUserIndex() > bodyB->getUserIndex();
	} else {
		// If only one body has a user index, make sure that body is A
		shouldSwap = (bodyB->getUserIndex() != -1);
	}

	if (shouldSwap)
		std::swap(bodyA, bodyB);

	int
		userIndexA = bodyA->getUserIndex(),
		userIndexB = bodyB->getUserIndex();

	bool carInvolved = (userIndexA == BT_USERINFO_TYPE_CAR);
	if (carInvolved) {

		Car* car = (Car*)bodyA->getUserPointer();
		Arena* arenaInst = (Arena*)car->_bulletVehicle.m_dynamicsWorld->getWorldUserInfo();

		if (userIndexB == BT_USERINFO_TYPE_BALL) {
			// Car + Ball
			arenaInst->
				_BtCallback_OnCarBallCollision(car, (Ball*)bodyB->getUserPointer(), contactPoint, shouldSwap);
		} else if (userIndexB == BT_USERINFO_TYPE_CAR) {
			// Car + Car
			arenaInst->
				_BtCallback_OnCarCarCollision(car, (Car*)bodyB->getUserPointer(), contactPoint);
		} else {
			// Car + World
			arenaInst->
				_BtCallback_OnCarWorldCollision(car, (btCollisionObject*)bodyB->getUserPointer(), contactPoint);
		}
	}
	
	btAdjustInternalEdgeContacts(
		contactPoint, 
		(shouldSwap ? objA : objB), (shouldSwap ? objB : objA),
		(shouldSwap ? partID_A : partID_B), (shouldSwap ? indexA : indexB)
	);
	return true;
}

void Arena::_BtCallback_OnCarBallCollision(Car* car, Ball* ball, btManifoldPoint& manifoldPoint, bool ballIsBodyA) {
	using namespace RLConst;

	auto carState = car->GetState();
	auto ballState = ball->GetState();

	// Manually override manifold friction/restitution
	manifoldPoint.m_combinedFriction = CARBALL_COLLISION_FRICTION;
	manifoldPoint.m_combinedRestitution = CARBALL_COLLISION_RESTITUTION;

	auto& ballHitInfo = car->_internalState.ballHitInfo;

	ballHitInfo.isValid = true;

	ballHitInfo.relativePosOnBall = (ballIsBodyA ? manifoldPoint.m_localPointA : manifoldPoint.m_localPointB) * BT_TO_UU;
	ballHitInfo.tickCountWhenHit = this->tickCount;

	ballHitInfo.ballPos = ballState.pos;
	ballHitInfo.extraHitVel = Vec();

	if (_ballTouchCallback.func)
		_ballTouchCallback.func(this, car, _ballTouchCallback.userInfo);

	// Once we do an extra car-ball impulse, we need to wait at least 1 tick to do it again
	if ((tickCount > ballHitInfo.tickCountWhenExtraImpulseApplied + 1) || (ballHitInfo.tickCountWhenExtraImpulseApplied > tickCount)) {
		ballHitInfo.tickCountWhenExtraImpulseApplied = this->tickCount;
	} else {
		// Don't do multiple extra impulses in a row
		return;
	}

	btVector3 carForward = car->GetForwardDir();
	btVector3 relPos = ballState.pos - carState.pos;
	btVector3 relVel = ballState.vel - carState.vel;

	float relSpeed = RS_MIN(relVel.length(), BALL_CAR_EXTRA_IMPULSE_MAXDELTAVEL_UU);

	if (relSpeed > 0) {
		btVector3 hitDir = (relPos * btVector3(1, 1, BALL_CAR_EXTRA_IMPULSE_Z_SCALE)).safeNormalized();
		btVector3 forwardDirAdjustment = carForward * hitDir.dot(carForward) * (1 - BALL_CAR_EXTRA_IMPULSE_FORWARD_SCALE);
		hitDir = (hitDir - forwardDirAdjustment).safeNormalized();
		btVector3 addedVel = (hitDir * relSpeed) * BALL_CAR_EXTRA_IMPULSE_FACTOR_CURVE.GetOutput(relSpeed) * _mutatorConfig.ballHitExtraForceScale;
		ballHitInfo.extraHitVel = addedVel;

		// Velocity won't be actually added until the end of this tick
		ball->_velocityImpulseCache += addedVel * UU_TO_BT;
	}
}

void Arena::_BtCallback_OnCarCarCollision(Car* car1, Car* car2, btManifoldPoint& manifoldPoint) {
	using namespace RLConst;

	// Manually override manifold friction/restitution
	manifoldPoint.m_combinedFriction = RLConst::CARCAR_COLLISION_FRICTION;
	manifoldPoint.m_combinedRestitution = RLConst::CARCAR_COLLISION_RESTITUTION;

	// Test collision both ways
	for (int i = 0; i < 2; i++) {

		bool isSwapped = (i == 1);
		if (isSwapped)
			std::swap(car1, car2);

		CarState
			state = car1->GetState(),
			otherState = car2->GetState();

		if (state.isDemoed || otherState.isDemoed)
			return;

		if ((state.carContact.otherCarID == car2->id) && (state.carContact.cooldownTimer > 0))
			continue; // In cooldown

		Vec deltaPos = (otherState.pos - state.pos);
		if (state.vel.Dot(deltaPos) > 0) { // Going towards other car

			Vec velDir = state.vel.Normalized();
			Vec dirToOtherCar = deltaPos.Normalized();

			float speedTowardsOtherCar = state.vel.Dot(dirToOtherCar);
			float otherCarAwaySpeed = otherState.vel.Dot(velDir);

			if (speedTowardsOtherCar > otherCarAwaySpeed) { // Going towards other car faster than they are going away

				Vec localPoint = isSwapped ? manifoldPoint.m_localPointB : manifoldPoint.m_localPointA;
				bool hitWithBumper = (localPoint.x * BT_TO_UU) > BUMP_MIN_FORWARD_DIST;
				if (hitWithBumper) {

					bool isDemo;
					switch (_mutatorConfig.demoMode) {
					case DemoMode::ON_CONTACT:
						isDemo = true; // BOOM
						break;
					case DemoMode::DISABLED:
						isDemo = false;
						break;
					default:
						isDemo = state.isSupersonic;
					}

					if (isDemo && !_mutatorConfig.enableTeamDemos)
						isDemo = car1->team != car2->team;

					if (isDemo) {
						car2->Demolish(_mutatorConfig.respawnDelay);
					} else {
						bool groundHit = car2->_internalState.isOnGround;

						float baseScale =
							(groundHit ? BUMP_VEL_AMOUNT_GROUND_CURVE : BUMP_VEL_AMOUNT_AIR_CURVE).GetOutput(speedTowardsOtherCar);

						Vec hitUpDir =
							(otherState.isOnGround ? (Vec)car2->GetUpDir() : Vec(0, 0, 1));

						Vec bumpImpulse =
							velDir * baseScale +
							hitUpDir * BUMP_UPWARD_VEL_AMOUNT_CURVE.GetOutput(speedTowardsOtherCar)
							* _mutatorConfig.bumpForceScale;

						car2->_velocityImpulseCache += bumpImpulse * UU_TO_BT;
					}

					car1->_internalState.carContact.otherCarID = car2->id;
					car1->_internalState.carContact.cooldownTimer = _mutatorConfig.bumpCooldownTime;

					if (_carBumpCallback.func)
						_carBumpCallback.func(this, car1, car2, isDemo, _carBumpCallback.userInfo);
				}
			}
		}
	}
}

void Arena::_BtCallback_OnCarWorldCollision(Car* car, btCollisionObject* world, btManifoldPoint& manifoldPoint) {
	car->_internalState.worldContact.hasContact = true;
	car->_internalState.worldContact.contactNormal = manifoldPoint.m_normalWorldOnB;

	// Manually override manifold friction/restitution
	manifoldPoint.m_combinedFriction = _mutatorConfig.carWorldFriction;
	manifoldPoint.m_combinedRestitution = _mutatorConfig.carWorldRestitution;
}

Arena::Arena(GameMode gameMode, float tickRate) {

	// Tickrate must be from 15 to 120tps
	assert(tickRate >= 15 && tickRate <= 120);

	RocketSim::AssertInitialized("Cannot create Arena, ");

	this->gameMode = gameMode;
	this->tickTime = 1 / tickRate;
	this->_mutatorConfig = MutatorConfig();

	{ // Initialize world

		btDefaultCollisionConstructionInfo collisionConfigConstructionInfo = {};
		_bulletWorldParams.collisionConfig.setup(collisionConfigConstructionInfo);

		_bulletWorldParams.collisionDispatcher.setup(&_bulletWorldParams.collisionConfig);
		_bulletWorldParams.constraintSolver = btSequentialImpulseConstraintSolver();

		_bulletWorldParams.overlappingPairCache = new (btAlignedAlloc(sizeof(btHashedOverlappingPairCache), 16)) btHashedOverlappingPairCache();
		_bulletWorldParams.broadphase = btDbvtBroadphase(_bulletWorldParams.overlappingPairCache);

		_bulletWorld.setup(
			&_bulletWorldParams.collisionDispatcher,
			&_bulletWorldParams.broadphase,
			&_bulletWorldParams.constraintSolver,
			&_bulletWorldParams.collisionConfig
		);

		_bulletWorld.setGravity(_mutatorConfig.gravity * UU_TO_BT);

		// Adjust solver configuration to be closer to older Bullet (Rocket League's Bullet is from somewhere between 2013 and 2015)
		auto& solverInfo = _bulletWorld.getSolverInfo();
		solverInfo.m_splitImpulsePenetrationThreshold = 1.0e30;
		solverInfo.m_erp2 = 0.8f;
	}

	if (gameMode == GameMode::SOCCAR) {
		_SetupArenaCollisionShapes();

#ifndef RS_NO_SUSPCOLGRID
		_suspColGrid = RocketSim::GetDefaultSuspColGrid();
		_suspColGrid.defaultWorldCollisionRB = &_worldCollisionRBs[0];
#endif

		// Give arena collision shapes the proper restitution/friction values
		for (size_t i = 0; i < _worldCollisionRBAmount; i++) {
			btRigidBody* rb = &_worldCollisionRBs[i];
			// TODO: Move to RLConst
			rb->setRestitution(0.3f);
			rb->setFriction(0.6f);
			rb->setRollingFriction(0.f);
		}
	} else {
		_worldCollisionRBs = NULL;
		_worldCollisionRBAmount = 0;
		
		_worldCollisionBvhShapes = NULL;
		_worldCollisionPlaneShapes = NULL;
	}

	{ // Initialize ball
		ball = Ball::_AllocBall();

		ball->_BulletSetup(&_bulletWorld, _mutatorConfig);
		ball->SetState(BallState());
	}

	if (gameMode == GameMode::SOCCAR) { // Initialize boost pads
		using namespace RLConst::BoostPads;

		_boostPads.reserve(LOCS_AMOUNT_BIG + LOCS_AMOUNT_SMALL);

		for (int i = 0; i < (LOCS_AMOUNT_BIG + LOCS_AMOUNT_SMALL); i++) {
			bool isBig = i < LOCS_AMOUNT_BIG;

			btVector3 pos = isBig ? LOCS_BIG[i] : LOCS_SMALL[i - LOCS_AMOUNT_BIG];

			BoostPad* pad = BoostPad::_AllocBoostPad();
			pad->_Setup(isBig, pos);

			_boostPads.push_back(pad);
			_boostPadGrid.Add(pad);
		}
	}

	// Set internal tick callback
	_bulletWorld.setWorldUserInfo(this);

	gContactAddedCallback = &Arena::_BulletContactAddedCallback;
}

Arena* Arena::Create(GameMode gameMode, float tickRate) {
	return new Arena(gameMode, tickRate);
}

void Arena::Serialize(DataStreamOut& out) {
	out.WriteMultiple(gameMode, tickTime, tickCount, _lastCarID);

	{ // Serialize cars
		out.Write<uint32_t>(_cars.size());
		for (auto car : _cars) {
			out.Write(car->team);
			out.Write(car->id);
			car->Serialize(out);
		}
	}

	if (gameMode == GameMode::SOCCAR) { // Serialize boost pads
		out.Write<uint32_t>(_boostPads.size());
		for (auto pad : _boostPads)
			pad->GetState().Serialize(out);
	}

	{ // Serialize ball
		ball->GetState().Serialize(out);
	}

	{ // Serialize mutators
		_mutatorConfig.Serialize(out);
	}
}

Arena* Arena::DeserializeNew(DataStreamIn& in) {
	constexpr char ERROR_PREFIX[] = "Arena::Deserialize(): ";

	GameMode gameMode;
	float tickTime;
	uint64_t tickCount;
	uint32_t lastCarID;

	in.ReadMultiple(gameMode, tickTime, tickCount, lastCarID);

	Arena* newArena = new Arena(gameMode, 1.f / tickTime);
	newArena->tickCount = tickCount;
	
	{ // Deserialize cars
		uint32_t carAmount = in.Read<uint32_t>();
		for (int i = 0; i < carAmount; i++) {
			Team team;
			uint32_t id;
			in.Read(team);
			in.Read(id);

#ifndef RS_MAX_SPEED
			if (newArena->_carIDMap.count(id))
				RS_ERR_CLOSE(ERROR_PREFIX << "Failed to load, got repeated car ID of " << id << ".");
#endif

			Car* newCar = newArena->DeserializeNewCar(in, team);

			// Force ID
			newArena->_carIDMap.erase(newCar->id);
			newArena->_carIDMap[id] = newCar;
			newCar->id = id;
		}

		newArena->_lastCarID = lastCarID;
	}

	// Deserialize boost pads
	if (gameMode == GameMode::SOCCAR) {
		uint32_t boostPadAmount = in.Read<uint32_t>();

#ifndef RS_MAX_SPEED
		if (boostPadAmount != newArena->_boostPads.size())
			RS_ERR_CLOSE(ERROR_PREFIX << "Failed to load, " <<
				"different boost pad amount written in file (" << boostPadAmount << "/" << newArena->_boostPads.size() << ")");
#endif

		for (auto pad : newArena->_boostPads) {
			BoostPadState padState = BoostPadState();
			padState.Deserialize(in);
			pad->SetState(padState);
		}
	}

	{ // Deserialize ball
		BallState ballState = BallState();
		ballState.Deserialize(in);
		newArena->ball->SetState(ballState);
	}

	{ // Serialize mutators
		newArena->_mutatorConfig.Deserialize(in);
		newArena->SetMutatorConfig(newArena->_mutatorConfig);
	}

	return newArena;
}

Arena* Arena::Clone(bool copyCallbacks) {
	Arena* newArena = new Arena(this->gameMode, this->GetTickRate());
	
	if (copyCallbacks)
	{
		newArena->_ballTouchCallback   = this->_ballTouchCallback;
		newArena->_boostPickupCallback = this->_boostPickupCallback;
		newArena->_carBumpCallback     = this->_carBumpCallback;
		newArena->_goalScoreCallback   = this->_goalScoreCallback;
	}
	
	newArena->ball->SetState(this->ball->GetState());
	newArena->ball->_velocityImpulseCache = this->ball->_velocityImpulseCache;

	for (Car* car : this->_cars) {
		Car* newCar = newArena->AddCar(car->team, car->config);
		
		newCar->SetState(car->GetState());
		newCar->id = car->id;
		newCar->controls = car->controls;
		newCar->_velocityImpulseCache = car->_velocityImpulseCache;
	}

	
	assert(this->_boostPads.size() == newArena->_boostPads.size());
	for (int i = 0; i < this->_boostPads.size(); i++)
		newArena->_boostPads[i]->SetState(this->_boostPads[i]->GetState());

	newArena->tickCount = this->tickCount;
	newArena->_lastCarID = this->_lastCarID;

	return newArena;
}

Car* Arena::DeserializeNewCar(DataStreamIn& in, Team team) {
	Car* car = Car::_AllocateCar();
	car->_Deserialize(in);
	car->team = team;

	_AddCarFromPtr(car);

	car->_BulletSetup(&_bulletWorld, _mutatorConfig);
	car->SetState(car->_internalState);

	return car;
}

void Arena::Step(int ticksToSimulate) {
	for (int i = 0; i < ticksToSimulate; i++) {

		_bulletWorld.setWorldUserInfo(this);

		{ // Ball zero-vel sleeping
			if (ball->_rigidBody.m_linearVelocity.length2() == 0 && ball->_rigidBody.m_angularVelocity.length2() == 0) {
				ball->_rigidBody.setActivationState(ISLAND_SLEEPING);
			} else {
				ball->_rigidBody.setActivationState(ACTIVE_TAG);
			}
		}

		if (gameMode == GameMode::SOCCAR) {
#ifndef RS_NO_SUSPCOLGRID
			{ // Add dynamic bodies to suspension grid
				for (Car* car : _cars) {
					btVector3 min, max;
					car->_rigidBody.getAabb(min, max);
					_suspColGrid.UpdateDynamicCollisions(min, max, false);
				}

				btVector3 min, max;
				ball->_rigidBody.getAabb(min, max);
				_suspColGrid.UpdateDynamicCollisions(min, max, false);
			}
#endif
		}

		for (Car* car : _cars) {
			SuspensionCollisionGrid* suspColGridPtr;
#ifdef RS_NO_SUSPCOLGRID
			suspColGridPtr = NULL;
#else
			if (gameMode == GameMode::SOCCAR) {
				suspColGridPtr = &_suspColGrid;
			} else {
				suspColGridPtr = NULL;
			}
#endif
			car->_PreTickUpdate(tickTime, _mutatorConfig, suspColGridPtr);
		}

		if (gameMode == GameMode::SOCCAR) {
#ifndef RS_NO_SUSPCOLGRID
			{ // Remove dynamic bodies from suspension grid
				for (Car* car : _cars) {
					btVector3 min, max;
					car->_rigidBody.getAabb(min, max);
					_suspColGrid.UpdateDynamicCollisions(min, max, true);
				}

				btVector3 min, max;
				ball->_rigidBody.getAabb(min, max);
				_suspColGrid.UpdateDynamicCollisions(min, max, true);
			}	
#endif
		}

		if (gameMode == GameMode::SOCCAR) {
			for (BoostPad* pad : _boostPads)
				pad->_PreTickUpdate(tickTime);
		}

		// Update world
		_bulletWorld.stepSimulation(tickTime, 0, tickTime);

		for (Car* car : _cars) {
			car->_PostTickUpdate(tickTime, _mutatorConfig);
			car->_FinishPhysicsTick(_mutatorConfig);
			if (gameMode == GameMode::SOCCAR) {
				_boostPadGrid.CheckCollision(car);
			}
		}

		if (gameMode == GameMode::SOCCAR) {
			for (BoostPad* pad : _boostPads)
			{
				if (pad->_PostTickUpdate(tickTime, _mutatorConfig) && _boostPickupCallback.func)
					_boostPickupCallback.func(this, pad->_internalState.curLockedCar, pad, _boostPickupCallback.userInfo);
			}
		}

		ball->_FinishPhysicsTick(_mutatorConfig);

		if (gameMode == GameMode::SOCCAR) {
			if (_goalScoreCallback.func != NULL) { // Potentially fire goal score callback
				float ballPosY = ball->_rigidBody.m_worldTransform.m_origin.y() * BT_TO_UU;
				if (abs(ballPosY) > RLConst::SOCCAR_BALL_SCORE_THRESHOLD_Y) {
					// Orange goal is at positive Y, so if the ball's Y is positive, it's in orange goal and thus blue scored
					Team scoringTeam = (ballPosY > 0) ? Team::BLUE : Team::ORANGE;
					_goalScoreCallback.func(this, scoringTeam, _goalScoreCallback.userInfo);
				}
			}
		}

		tickCount++;
	}
}

bool Arena::IsBallProbablyGoingIn(float maxTime) {
	if (gameMode == GameMode::SOCCAR) {
		Vec ballPos = ball->_rigidBody.m_worldTransform.m_origin * UU_TO_BT;
		Vec ballVel = ball->_rigidBody.m_linearVelocity * UU_TO_BT;

		if (ballVel.y < FLT_EPSILON)
			return false;

		float scoreDirSgn = RS_SGN(ballVel.y);
		float goalScoreY = (RLConst::SOCCAR_GOAL_SCORE_BASE_THRESHOLD_Y + _mutatorConfig.ballRadius) * scoreDirSgn;
		float distToGoal = abs(ballPos.y - scoreDirSgn);

		float timeToGoal = distToGoal / abs(ballVel.y);

		if (timeToGoal > maxTime)
			return false;
		
		// Roughly account for drag
		timeToGoal *= 1 + powf(1 - _mutatorConfig.ballDrag, timeToGoal);

		Vec extrapPosWhenScore = ballPos + (ballVel * timeToGoal) + (_mutatorConfig.gravity * timeToGoal * timeToGoal) / 2;
		
		// From: https://github.com/RLBot/RLBot/wiki/Useful-Game-Values
		constexpr float
			APPROX_GOAL_HALF_WIDTH = 892.755f,
			APPROX_GOAL_HEIGHT = 642.775;

		float SCORE_MARGIN = _mutatorConfig.ballRadius * 0.64f;

		if (extrapPosWhenScore.z > APPROX_GOAL_HEIGHT + SCORE_MARGIN)
			return false; // Too high

		if (abs(extrapPosWhenScore.x) > APPROX_GOAL_HALF_WIDTH + SCORE_MARGIN)
			return false; // Too far to the side

		// Ok it's probably gonna score, or at least be very close
		return true;
	} else {
		return false;
	}
}

Arena::~Arena() {

	// Remove all from bullet world constraints
	while (_bulletWorld.getNumConstraints() > 0)
		_bulletWorld.removeConstraint(0);

	// Manually remove all collision objects
	// Otherwise we run into issues regarding deconstruction order
	while (_bulletWorld.getNumCollisionObjects() > 0)
		_bulletWorld.removeCollisionObject(_bulletWorld.getCollisionObjectArray()[0]);

	// Remove all cars
	if (ownsCars) {
		for (Car* car : _cars)
			delete car;
	}

	// Remove the ball
	if (ownsBall) {
		Ball::_DestroyBall(ball);
	}

	if (gameMode == GameMode::SOCCAR) {
		if (ownsBoostPads) {
			// Remove all boost pads
			for (BoostPad* boostPad : _boostPads)
				delete boostPad;
		}

		delete[] _worldCollisionRBs;
		delete[] _worldCollisionPlaneShapes;
		delete[] _worldCollisionBvhShapes;
	}

	_bulletWorldParams.overlappingPairCache->~btHashedOverlappingPairCache();
	btAlignedFree(_bulletWorldParams.overlappingPairCache);
}

void Arena::_SetupArenaCollisionShapes() {
	// TODO: This is just for soccar arena (for now)
	assert(gameMode == GameMode::SOCCAR);

	auto collisionMeshes = RocketSim::GetArenaCollisionShapes();
	_worldCollisionBvhShapes = new btBvhTriangleMeshShape[collisionMeshes.size()];

	constexpr size_t PLANE_COUNT = 4;
	_worldCollisionPlaneShapes = new btStaticPlaneShape[PLANE_COUNT];

	_worldCollisionRBAmount = collisionMeshes.size() + PLANE_COUNT;
	_worldCollisionRBs = new btRigidBody[_worldCollisionRBAmount];

	for (size_t i = 0; i < collisionMeshes.size(); i++) {
		_AddStaticCollisionShape(i, i, collisionMeshes[i], _worldCollisionBvhShapes);

		// Don't free the BVH when we deconstruct this arena
		_worldCollisionBvhShapes[i].m_ownsBvh = false;
	}

	{ // Add arena collision planes (floor/walls/ceiling)
		using namespace RLConst;

		// Floor
		auto floorShape = btStaticPlaneShape(btVector3(0, 0, 1), 0);
		_AddStaticCollisionShape(
			collisionMeshes.size() + 0,
			0,
			&floorShape, _worldCollisionPlaneShapes
		);

		// Ceiling
		auto ceilingShape = btStaticPlaneShape(btVector3(0, 0, -1), 0);
		_AddStaticCollisionShape(
			collisionMeshes.size() + 1,
			1,
			&ceilingShape, _worldCollisionPlaneShapes,
			Vec( 0, 0, ARENA_HEIGHT ) * UU_TO_BT
		);

		// Side walls
		auto leftWallShape = btStaticPlaneShape(btVector3(1, 0, 0), 0);
		_AddStaticCollisionShape(
			collisionMeshes.size() + 2,
			2,
			&leftWallShape, _worldCollisionPlaneShapes,
			Vec( -ARENA_EXTENT_X, 0, ARENA_HEIGHT / 2 ) * UU_TO_BT
		);
		auto rightWallShape = btStaticPlaneShape(btVector3(-1, 0, 0), 0);
		_AddStaticCollisionShape(
			collisionMeshes.size() + 3,
			3,
			&rightWallShape, _worldCollisionPlaneShapes,
			Vec(ARENA_EXTENT_X, 0, ARENA_HEIGHT / 2) * UU_TO_BT
		);
	}
}
