#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *CarState::Type = nullptr;

PyMemberDef CarState::Members[] = {
    {.name      = "update_counter",
        .type   = TypeHelper<decltype (::CarState::updateCounter)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, updateCounter),
        .flags  = 0,
        .doc    = "Update counter (ticks since last state set)"},
    {.name      = "is_on_ground",
        .type   = TypeHelper<decltype (::CarState::isOnGround)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isOnGround),
        .flags  = 0,
        .doc    = "Is on ground"},
    {.name      = "has_jumped",
        .type   = TypeHelper<decltype (::CarState::hasJumped)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasJumped),
        .flags  = 0,
        .doc    = "Has jumped"},
    {.name      = "has_double_jumped",
        .type   = TypeHelper<decltype (::CarState::hasDoubleJumped)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasDoubleJumped),
        .flags  = 0,
        .doc    = "Has double jumped"},
    {.name      = "has_flipped",
        .type   = TypeHelper<decltype (::CarState::hasFlipped)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasFlipped),
        .flags  = 0,
        .doc    = "Has flipped"},
    {.name      = "jump_time",
        .type   = TypeHelper<decltype (::CarState::jumpTime)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, jumpTime),
        .flags  = 0,
        .doc    = "Jump time"},
    {.name      = "flip_time",
        .type   = TypeHelper<decltype (::CarState::flipTime)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, flipTime),
        .flags  = 0,
        .doc    = "Flip time"},
    {.name      = "is_flipping",
        .type   = TypeHelper<decltype (::CarState::isFlipping)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isFlipping),
        .flags  = 0,
        .doc    = "Is flipping"},
    {.name      = "is_jumping",
        .type   = TypeHelper<decltype (::CarState::isJumping)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isJumping),
        .flags  = 0,
        .doc    = "Is jumping"},
    {.name      = "air_time_since_jump",
        .type   = TypeHelper<decltype (::CarState::airTimeSinceJump)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, airTimeSinceJump),
        .flags  = 0,
        .doc    = "Air time since jump"},
    {.name      = "boost",
        .type   = TypeHelper<decltype (::CarState::boost)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, boost),
        .flags  = 0,
        .doc    = "Boost"},
    {.name      = "time_spent_boosting",
        .type   = TypeHelper<decltype (::CarState::timeSpentBoosting)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, timeSpentBoosting),
        .flags  = 0,
        .doc    = "Time spent boosting"},
    {.name      = "is_supersonic",
        .type   = TypeHelper<decltype (::CarState::isSupersonic)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isSupersonic),
        .flags  = 0,
        .doc    = "Is supersonic"},
    {.name      = "supersonic_time",
        .type   = TypeHelper<decltype (::CarState::supersonicTime)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, supersonicTime),
        .flags  = 0,
        .doc    = "Supersonic time"},
    {.name      = "handbrake_val",
        .type   = TypeHelper<decltype (::CarState::handbrakeVal)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, handbrakeVal),
        .flags  = 0,
        .doc    = "Handbrake val"},
    {.name      = "is_auto_flipping",
        .type   = TypeHelper<decltype (::CarState::isAutoFlipping)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isAutoFlipping),
        .flags  = 0,
        .doc    = "Is auto flipping"},
    {.name      = "auto_flip_timer",
        .type   = TypeHelper<decltype (::CarState::autoFlipTimer)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, autoFlipTimer),
        .flags  = 0,
        .doc    = "Auto flip timer"},
    {.name      = "auto_flip_torque_scale",
        .type   = TypeHelper<decltype (::CarState::autoFlipTorqueScale)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, autoFlipTorqueScale),
        .flags  = 0,
        .doc    = "Auto flip torque scale"},
    {.name      = "has_world_contact",
        .type   = TypeHelper<decltype (::CarState::worldContact.hasContact)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, worldContact.hasContact),
        .flags  = 0,
        .doc    = "Has world contact"},
    {.name      = "car_contact_id",
        .type   = TypeHelper<decltype (::CarState::carContact.otherCarID)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, carContact.otherCarID),
        .flags  = 0,
        .doc    = "Car contact other car id"},
    {.name      = "car_contact_cooldown_timer",
        .type   = TypeHelper<decltype (::CarState::carContact.cooldownTimer)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, carContact.cooldownTimer),
        .flags  = 0,
        .doc    = "Car contact cooldown timer"},
    {.name      = "is_demoed",
        .type   = TypeHelper<decltype (::CarState::isDemoed)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isDemoed),
        .flags  = 0,
        .doc    = "Is demoed"},
    {.name      = "demo_respawn_timer",
        .type   = TypeHelper<decltype (::CarState::demoRespawnTimer)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, demoRespawnTimer),
        .flags  = 0,
        .doc    = "Demo respawn timer"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef CarState::Methods[] = {
    {.ml_name = "__getstate__", .ml_meth = (PyCFunction)&CarState::Pickle, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&CarState::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&CarState::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.CarState
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&CarState::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.CarState
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef CarState::GetSet[] = {
    GETSET_ENTRY (CarState, pos, "Position"),
    GETSET_ENTRY (CarState, rot_mat, "Rotation matrix"),
    GETSET_ENTRY (CarState, vel, "Velocity"),
    GETSET_ENTRY (CarState, ang_vel, "Angular velocity"),
    GETSET_ENTRY (CarState, last_rel_dodge_torque, "Last relative dodge torque"),
    GETSET_ENTRY (CarState, last_controls, "Last controls"),
    GETSET_ENTRY (CarState, world_contact_normal, "World contact normal"),
    GETSET_ENTRY (CarState, ball_hit_info, "Ball hit info"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot CarState::Slots[] = {
    {Py_tp_new, (void *)&CarState::New},
    {Py_tp_init, (void *)&CarState::Init},
    {Py_tp_dealloc, (void *)&CarState::Dealloc},
    {Py_tp_members, &CarState::Members},
    {Py_tp_methods, &CarState::Methods},
    {Py_tp_getset, &CarState::GetSet},
    {Py_tp_doc, (void *)R"(Car state
__init__(self
	pos: RocketSim.Vec = RocketSim.Vec(z = 17.0),
	rot_mat: RocketSim.RotMat = RocketSim.RotMat(),
	vel: RocketSim.Vec = RocketSim.Vec(),
	ang_vel: RocketSim.Vec = RocketSim.Vec(),
	is_on_ground: bool = True,
	has_jumped: bool = False,
	has_double_jumped: bool = False,
	has_flipped: bool = False,
	last_rel_dodge_torque: RocketSim.Vec = RocketSim.Vec(),
	jump_time: float = 0.0,
	flip_time: float = 0.0,
	is_flipping: bool = False,
	is_jumping: bool = False,
	air_time_since_jump: float = 0.0,
	boost: float = 33.3,
	time_spent_boosting: float = 0.0,
	is_supersonic: bool = False,
	supersonic_time: float = 0.0,
	handbrake_val: float = 0.0,
	is_auto_flipping: bool = False,
	auto_flip_timer: float = 0.0,
	has_world_contact: bool = False,
	world_contact_normal: RocketSim.Vec = RocketSim.Vec(),
	car_contact_id: int = 0,
	car_contact_cooldown_timer: float = 0.0,
	is_demoed: bool = False,
	demo_respawn_timer: float = 0.0,
	ball_hit_info: RocketSim.BallHitInfo = RocketSim.BallHitInfo(),
	last_controls: RocketSim.CarControls = RocketSim.CarControls(),
	update_counter: int = 0))"},
    {0, nullptr},
};

PyType_Spec CarState::Spec = {
    .name      = "RocketSim.CarState",
    .basicsize = sizeof (CarState),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = CarState::Slots,
};

PyRef<CarState> CarState::NewFromCarState (::CarState const &state_) noexcept
{
	auto const self = PyRef<CarState>::stealObject (CarState::New (CarState::Type, nullptr, nullptr));
	if (!self || !InitFromCarState (self.borrow (), state_))
		return nullptr;

	return self;
}

bool CarState::InitFromCarState (CarState *const self_, ::CarState const &state_) noexcept
{
	auto pos                = Vec::NewFromVec (state_.pos);
	auto rotMat             = RotMat::NewFromRotMat (state_.rotMat);
	auto vel                = Vec::NewFromVec (state_.vel);
	auto angVel             = Vec::NewFromVec (state_.angVel);
	auto lastRelDodgeTorque = Vec::NewFromVec (state_.lastRelDodgeTorque);
	auto lastControls       = CarControls::NewFromCarControls (state_.lastControls);
	auto worldContactNormal = Vec::NewFromVec (state_.worldContact.contactNormal);
	auto ballHitInfo        = BallHitInfo::NewFromBallHitInfo (state_.ballHitInfo);

	if (!pos || !rotMat || !vel || !angVel || !lastRelDodgeTorque || !lastControls || !worldContactNormal ||
	    !ballHitInfo)
		return false;

	PyRef<Vec>::assign (self_->pos, pos.borrowObject ());
	PyRef<RotMat>::assign (self_->rotMat, rotMat.borrowObject ());
	PyRef<Vec>::assign (self_->vel, vel.borrowObject ());
	PyRef<Vec>::assign (self_->angVel, angVel.borrowObject ());
	PyRef<Vec>::assign (self_->lastRelDodgeTorque, lastRelDodgeTorque.borrowObject ());
	PyRef<CarControls>::assign (self_->lastControls, lastControls.borrowObject ());
	PyRef<Vec>::assign (self_->worldContactNormal, worldContactNormal.borrowObject ());
	PyRef<BallHitInfo>::assign (self_->ballHitInfo, ballHitInfo.borrowObject ());

	self_->state = state_;

	return true;
}

::CarState CarState::ToCarState (CarState *self_) noexcept
{
	auto state = self_->state;

	state.pos                        = Vec::ToVec (self_->pos);
	state.rotMat                     = RotMat::ToRotMat (self_->rotMat);
	state.vel                        = Vec::ToVec (self_->vel);
	state.angVel                     = Vec::ToVec (self_->angVel);
	state.lastRelDodgeTorque         = Vec::ToVec (self_->lastRelDodgeTorque);
	state.lastControls               = CarControls::ToCarControls (self_->lastControls);
	state.worldContact.contactNormal = Vec::ToVec (self_->worldContactNormal);
	state.ballHitInfo                = BallHitInfo::ToBallHitInfo (self_->ballHitInfo);

	return state;
}

PyObject *CarState::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<CarState>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->state)::CarState ();

	self->pos                = nullptr;
	self->rotMat             = nullptr;
	self->vel                = nullptr;
	self->angVel             = nullptr;
	self->lastRelDodgeTorque = nullptr;
	self->lastControls       = nullptr;
	self->worldContactNormal = nullptr;
	self->ballHitInfo        = nullptr;

	return self.giftObject ();
}

int CarState::Init (CarState *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char posKwd[]                     = "pos";
	static char rotMatKwd[]                  = "rot_mat";
	static char velKwd[]                     = "vel";
	static char angVelKwd[]                  = "ang_vel";
	static char isOnGroundKwd[]              = "is_on_ground";
	static char hasJumpedKwd[]               = "has_jumped";
	static char hasDoubleJumpedKwd[]         = "has_double_jumped";
	static char hasFlippedKwd[]              = "has_flipped";
	static char lastRelDodgeTorqueKwd[]      = "last_rel_dodge_torque";
	static char jumpTimeKwd[]                = "jump_time";
	static char flipTimeKwd[]                = "flip_time";
	static char isFlippingKwd[]              = "is_flipping";
	static char isJumpingKwd[]               = "is_jumping";
	static char airTimeSinceJumpKwd[]        = "air_time_since_jump";
	static char boostKwd[]                   = "boost";
	static char timeSpentBoostingKwd[]       = "time_spent_boosting";
	static char isSupersonicKwd[]            = "is_supersonic";
	static char supersonicTimeKwd[]          = "supersonic_time";
	static char handbrakeValKwd[]            = "handbrake_val";
	static char isAutoFlippingKwd[]          = "is_auto_flipping";
	static char autoFlipTimerKwd[]           = "auto_flip_timer";
	static char hasWorldContactKwd[]         = "has_world_contact";
	static char worldContactNormalKwd[]      = "world_contact_normal";
	static char carContactIDKwd[]            = "car_contact_id";
	static char carContactCooldownTimerKwd[] = "car_contact_cooldown_timer";
	static char isDemoedKwd[]                = "is_demoed";
	static char demoRespawnTimerKwd[]        = "demo_respawn_timer";
	static char ballHitInfoKwd[]             = "ball_hit_info";
	static char lastControlsKwd[]            = "last_controls";
	static char updateCounterKwd[]           = "update_counter";

	static char *dict[] = {posKwd,
	    rotMatKwd,
	    velKwd,
	    angVelKwd,
	    isOnGroundKwd,
	    hasJumpedKwd,
	    hasDoubleJumpedKwd,
	    hasFlippedKwd,
	    lastRelDodgeTorqueKwd,
	    jumpTimeKwd,
	    flipTimeKwd,
	    isFlippingKwd,
	    isJumpingKwd,
	    airTimeSinceJumpKwd,
	    boostKwd,
	    timeSpentBoostingKwd,
	    isSupersonicKwd,
	    supersonicTimeKwd,
	    handbrakeValKwd,
	    isAutoFlippingKwd,
	    autoFlipTimerKwd,
	    hasWorldContactKwd,
	    worldContactNormalKwd,
	    carContactIDKwd,
	    carContactCooldownTimerKwd,
	    isDemoedKwd,
	    demoRespawnTimerKwd,
	    ballHitInfoKwd,
	    lastControlsKwd,
	    updateCounterKwd,
	    nullptr};

	::CarState state{};

	PyObject *pos                = nullptr; // borrowed references
	PyObject *rotMat             = nullptr;
	PyObject *vel                = nullptr;
	PyObject *angVel             = nullptr;
	PyObject *lastRelDodgeTorque = nullptr;
	PyObject *lastControls       = nullptr;
	PyObject *worldContactNormal = nullptr;
	PyObject *ballHitInfo        = nullptr;

	int isOnGround      = state.isOnGround;
	int hasJumped       = state.hasJumped;
	int hasDoubleJumped = state.hasDoubleJumped;
	int hasFlipped      = state.hasFlipped;
	int isFlipping      = state.isFlipping;
	int isJumping       = state.isJumping;
	int isSupersonic    = state.isSupersonic;
	int isAutoFlipping  = state.isAutoFlipping;
	int hasWorldContact = state.worldContact.hasContact;
	int isDemoed        = state.isDemoed;

	unsigned long carContactID       = state.carContact.otherCarID;
	unsigned long long updateCounter = state.updateCounter;
	if (!PyArg_ParseTupleAndKeywords (args_,
	        kwds_,
	        "|O!O!O!O!ppppO!ffppfffpffpfpO!kfpfO!O!K",
	        dict,
	        Vec::Type,
	        &pos,
	        RotMat::Type,
	        &rotMat,
	        Vec::Type,
	        &vel,
	        Vec::Type,
	        &angVel,
	        &isOnGround,
	        &hasJumped,
	        &hasDoubleJumped,
	        &hasFlipped,
	        Vec::Type,
	        &lastRelDodgeTorque,
	        &state.jumpTime,
	        &state.flipTime,
	        &isFlipping,
	        &isJumping,
	        &state.airTimeSinceJump,
	        &state.boost,
	        &state.timeSpentBoosting,
	        &isSupersonic,
	        &state.supersonicTime,
	        &state.handbrakeVal,
	        &isAutoFlipping,
	        &state.autoFlipTimer,
	        &hasWorldContact,
	        Vec::Type,
	        &worldContactNormal,
	        &carContactID,
	        &state.carContact.cooldownTimer,
	        &isDemoed,
	        &state.demoRespawnTimer,
	        BallHitInfo::Type,
	        &ballHitInfo,
	        CarControls::Type,
	        &lastControls,
	        &updateCounter))
		return -1;

	if (pos)
		state.pos = Vec::ToVec (PyCast<Vec> (pos));

	if (rotMat)
		state.rotMat = RotMat::ToRotMat (PyCast<RotMat> (rotMat));

	if (vel)
		state.vel = Vec::ToVec (PyCast<Vec> (vel));

	if (angVel)
		state.angVel = Vec::ToVec (PyCast<Vec> (angVel));

	if (lastRelDodgeTorque)
		state.lastRelDodgeTorque = Vec::ToVec (PyCast<Vec> (lastRelDodgeTorque));

	if (worldContactNormal)
		state.worldContact.contactNormal = Vec::ToVec (PyCast<Vec> (worldContactNormal));

	if (ballHitInfo)
		state.ballHitInfo = BallHitInfo::ToBallHitInfo (PyCast<BallHitInfo> (ballHitInfo));

	if (lastControls)
		state.lastControls = CarControls::ToCarControls (PyCast<CarControls> (lastControls));

	state.isOnGround              = isOnGround;
	state.hasJumped               = hasJumped;
	state.hasDoubleJumped         = hasDoubleJumped;
	state.hasFlipped              = hasFlipped;
	state.isFlipping              = isFlipping;
	state.isJumping               = isJumping;
	state.isSupersonic            = isSupersonic;
	state.isAutoFlipping          = isAutoFlipping;
	state.worldContact.hasContact = hasWorldContact;
	state.isDemoed                = isDemoed;
	state.updateCounter           = updateCounter;

	state.carContact.otherCarID = carContactID;

	if (!InitFromCarState (self_, state))
		return -1;

	return 0;
}

void CarState::Dealloc (CarState *self_) noexcept
{
	Py_XDECREF (self_->pos);
	Py_XDECREF (self_->rotMat);
	Py_XDECREF (self_->vel);
	Py_XDECREF (self_->angVel);
	Py_XDECREF (self_->lastRelDodgeTorque);
	Py_XDECREF (self_->lastControls);
	Py_XDECREF (self_->worldContactNormal);
	Py_XDECREF (self_->ballHitInfo);

	self_->state.~CarState ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *CarState::Pickle (CarState *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	::CarState const model{};
	auto const state = ToCarState (self_);

	if (state.updateCounter != model.updateCounter &&
	    !DictSetValue (dict.borrow (), "update_counter", PyLong_FromUnsignedLongLong (state.updateCounter)))
		return nullptr;

	if (state.pos != model.pos && !DictSetValue (dict.borrow (), "pos", PyNewRef (self_->pos)))
		return nullptr;

	if ((state.rotMat.forward != model.rotMat.forward || state.rotMat.right != model.rotMat.right ||
	        state.rotMat.up != model.rotMat.up) &&
	    !DictSetValue (dict.borrow (), "rot_mat", PyNewRef (self_->rotMat)))
		return nullptr;

	if (state.vel != model.vel && !DictSetValue (dict.borrow (), "vel", PyNewRef (self_->vel)))
		return nullptr;

	if (state.angVel != model.angVel && !DictSetValue (dict.borrow (), "ang_vel", PyNewRef (self_->angVel)))
		return nullptr;

	if (state.isOnGround != model.isOnGround &&
	    !DictSetValue (dict.borrow (), "is_on_ground", PyBool_FromLong (state.isOnGround)))
		return nullptr;

	if (state.hasJumped != model.hasJumped &&
	    !DictSetValue (dict.borrow (), "has_jumped", PyBool_FromLong (state.hasJumped)))
		return nullptr;

	if (state.hasDoubleJumped != model.hasDoubleJumped &&
	    !DictSetValue (dict.borrow (), "has_double_jumped", PyBool_FromLong (state.hasDoubleJumped)))
		return nullptr;

	if (state.hasFlipped != model.hasFlipped &&
	    !DictSetValue (dict.borrow (), "has_flipped", PyBool_FromLong (state.hasFlipped)))
		return nullptr;

	if (state.lastRelDodgeTorque != model.lastRelDodgeTorque &&
	    !DictSetValue (dict.borrow (), "last_rel_dodge_torque", PyNewRef (self_->lastRelDodgeTorque)))
		return nullptr;

	if (state.jumpTime != model.jumpTime &&
	    !DictSetValue (dict.borrow (), "jump_time", PyFloat_FromDouble (state.jumpTime)))
		return nullptr;

	if (state.flipTime != model.flipTime &&
	    !DictSetValue (dict.borrow (), "flip_time", PyFloat_FromDouble (state.flipTime)))
		return nullptr;

	if (state.isFlipping != model.isFlipping &&
	    !DictSetValue (dict.borrow (), "is_flipping", PyBool_FromLong (state.isFlipping)))
		return nullptr;

	if (state.isJumping != model.isJumping &&
	    !DictSetValue (dict.borrow (), "is_jumping", PyBool_FromLong (state.isJumping)))
		return nullptr;

	if (state.airTimeSinceJump != model.airTimeSinceJump &&
	    !DictSetValue (dict.borrow (), "air_time_since_jump", PyFloat_FromDouble (state.airTimeSinceJump)))
		return nullptr;

	if (state.boost != model.boost && !DictSetValue (dict.borrow (), "boost", PyFloat_FromDouble (state.boost)))
		return nullptr;

	if (state.timeSpentBoosting != model.timeSpentBoosting &&
	    !DictSetValue (dict.borrow (), "time_spent_boosting", PyFloat_FromDouble (state.timeSpentBoosting)))
		return nullptr;

	if (state.isSupersonic != model.isSupersonic &&
	    !DictSetValue (dict.borrow (), "is_supersonic", PyBool_FromLong (state.isSupersonic)))
		return nullptr;

	if (state.supersonicTime != model.supersonicTime &&
	    !DictSetValue (dict.borrow (), "supersonic_time", PyFloat_FromDouble (state.supersonicTime)))
		return nullptr;

	if (state.handbrakeVal != model.handbrakeVal &&
	    !DictSetValue (dict.borrow (), "handbrake_val", PyFloat_FromDouble (state.handbrakeVal)))
		return nullptr;

	if (state.isAutoFlipping != model.isAutoFlipping &&
	    !DictSetValue (dict.borrow (), "is_auto_flipping", PyBool_FromLong (state.isAutoFlipping)))
		return nullptr;

	if (state.autoFlipTimer != model.autoFlipTimer &&
	    !DictSetValue (dict.borrow (), "auto_flip_timer", PyFloat_FromDouble (state.autoFlipTimer)))
		return nullptr;

	if (state.worldContact.hasContact != model.worldContact.hasContact &&
	    !DictSetValue (dict.borrow (), "has_world_contact", PyBool_FromLong (state.worldContact.hasContact)))
		return nullptr;

	if (state.worldContact.contactNormal != model.worldContact.contactNormal &&
	    !DictSetValue (dict.borrow (), "world_contact_normal", PyNewRef (self_->worldContactNormal)))
		return nullptr;

	if (state.carContact.otherCarID != model.carContact.otherCarID &&
	    !DictSetValue (dict.borrow (), "car_contact_id", PyLong_FromUnsignedLong (state.carContact.otherCarID)))
		return nullptr;

	if (state.carContact.cooldownTimer != model.carContact.cooldownTimer &&
	    !DictSetValue (
	        dict.borrow (), "car_contact_cooldown_timer", PyFloat_FromDouble (state.carContact.cooldownTimer)))
		return nullptr;

	if (state.isDemoed != model.isDemoed &&
	    !DictSetValue (dict.borrow (), "is_demoed", PyBool_FromLong (state.isDemoed)))
		return nullptr;

	if (state.demoRespawnTimer != model.demoRespawnTimer &&
	    !DictSetValue (dict.borrow (), "demo_respawn_timer", PyFloat_FromDouble (state.demoRespawnTimer)))
		return nullptr;

	if ((state.ballHitInfo.relativePosOnBall != model.ballHitInfo.relativePosOnBall ||
	        state.ballHitInfo.ballPos != model.ballHitInfo.ballPos ||
	        state.ballHitInfo.extraHitVel != model.ballHitInfo.extraHitVel ||
	        state.ballHitInfo.tickCountWhenHit != model.ballHitInfo.tickCountWhenHit ||
	        state.ballHitInfo.tickCountWhenExtraImpulseApplied != model.ballHitInfo.tickCountWhenExtraImpulseApplied) &&
	    !DictSetValue (dict.borrow (), "ball_hit_info", PyNewRef (self_->ballHitInfo)))
		return nullptr;

	if ((state.lastControls.throttle != model.lastControls.throttle ||
	        state.lastControls.steer != model.lastControls.steer ||
	        state.lastControls.pitch != model.lastControls.pitch || state.lastControls.yaw != model.lastControls.yaw ||
	        state.lastControls.roll != model.lastControls.roll ||
	        state.lastControls.boost != model.lastControls.boost ||
	        state.lastControls.jump != model.lastControls.jump ||
	        state.lastControls.handbrake != model.lastControls.handbrake) &&
	    !DictSetValue (dict.borrow (), "last_controls", PyNewRef (self_->lastControls)))
		return nullptr;

	return dict.gift ();
}

PyObject *CarState::Unpickle (CarState *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *CarState::Copy (CarState *self_) noexcept
{
	auto state = PyRef<CarState>::stealObject (New (Type, nullptr, nullptr));
	if (!state)
		return nullptr;

	PyRef<Vec>::assign (state->pos, reinterpret_cast<PyObject *> (self_->pos));
	PyRef<RotMat>::assign (state->rotMat, reinterpret_cast<PyObject *> (self_->rotMat));
	PyRef<Vec>::assign (state->vel, reinterpret_cast<PyObject *> (self_->vel));
	PyRef<Vec>::assign (state->angVel, reinterpret_cast<PyObject *> (self_->angVel));
	PyRef<Vec>::assign (state->lastRelDodgeTorque, reinterpret_cast<PyObject *> (self_->lastRelDodgeTorque));
	PyRef<CarControls>::assign (state->lastControls, reinterpret_cast<PyObject *> (self_->lastControls));
	PyRef<Vec>::assign (state->worldContactNormal, reinterpret_cast<PyObject *> (self_->worldContactNormal));
	PyRef<BallHitInfo>::assign (state->ballHitInfo, reinterpret_cast<PyObject *> (self_->ballHitInfo));

	state->state = ToCarState (self_);

	return state.giftObject ();
}

PyObject *CarState::DeepCopy (CarState *self_, PyObject *memo_) noexcept
{
	auto state = PyRef<CarState>::stealObject (New (Type, nullptr, nullptr));
	if (!state)
		return nullptr;

	PyRef<Vec>::assign (state->pos, PyDeepCopy (self_->pos, memo_));
	if (!state->pos)
		return nullptr;

	PyRef<RotMat>::assign (state->rotMat, PyDeepCopy (self_->rotMat, memo_));
	if (!state->rotMat)
		return nullptr;

	PyRef<Vec>::assign (state->vel, PyDeepCopy (self_->vel, memo_));
	if (!state->vel)
		return nullptr;

	PyRef<Vec>::assign (state->angVel, PyDeepCopy (self_->angVel, memo_));
	if (!state->angVel)
		return nullptr;

	PyRef<Vec>::assign (state->lastRelDodgeTorque, PyDeepCopy (self_->lastRelDodgeTorque, memo_));
	if (!state->lastRelDodgeTorque)
		return nullptr;

	PyRef<CarControls>::assign (state->lastControls, PyDeepCopy (self_->lastControls, memo_));
	if (!state->lastControls)
		return nullptr;

	PyRef<Vec>::assign (state->worldContactNormal, PyDeepCopy (self_->worldContactNormal, memo_));
	if (!state->worldContactNormal)
		return nullptr;

	PyRef<BallHitInfo>::assign (state->ballHitInfo, PyDeepCopy (self_->ballHitInfo, memo_));
	if (!state->ballHitInfo)
		return nullptr;

	state->state = ToCarState (self_);

	return state.giftObject ();
}

PyObject *CarState::Getpos (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->pos).giftObject ();
}

int CarState::Setpos (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'pos' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->pos, value_);

	return 0;
}

PyObject *CarState::Getrot_mat (CarState *self_, void *) noexcept
{
	return PyRef<RotMat>::incRef (self_->rotMat).giftObject ();
}

int CarState::Setrot_mat (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'rot_mat' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, RotMat::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.RotMat");
		return -1;
	}

	PyRef<RotMat>::assign (self_->rotMat, value_);

	return 0;
}

PyObject *CarState::Getvel (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->vel).giftObject ();
}

int CarState::Setvel (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'vel' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->vel, value_);

	return 0;
}

PyObject *CarState::Getang_vel (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->angVel).giftObject ();
}

int CarState::Setang_vel (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'ang_vel' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->angVel, value_);

	return 0;
}

PyObject *CarState::Getlast_rel_dodge_torque (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->lastRelDodgeTorque).giftObject ();
}

int CarState::Setlast_rel_dodge_torque (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_TypeError, "can't delete 'last_rel_dodge_torque' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->lastRelDodgeTorque, value_);

	return 0;
}

PyObject *CarState::Getlast_controls (CarState *self_, void *) noexcept
{
	return PyRef<CarControls>::incRef (self_->lastControls).giftObject ();
}

int CarState::Setlast_controls (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'last_controls' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, CarControls::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.CarControls");
		return -1;
	}

	PyRef<CarControls>::assign (self_->lastControls, value_);

	return 0;
}

PyObject *CarState::Getworld_contact_normal (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->worldContactNormal).giftObject ();
}

int CarState::Setworld_contact_normal (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_TypeError, "can't delete 'world_contact_normal' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->worldContactNormal, value_);

	return 0;
}

PyObject *CarState::Getball_hit_info (CarState *self_, void *) noexcept
{
	return PyRef<BallHitInfo>::incRef (self_->ballHitInfo).giftObject ();
}

int CarState::Setball_hit_info (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'ball_hit_info' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, BallHitInfo::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.BallHitInfo");
		return -1;
	}

	PyRef<BallHitInfo>::assign (self_->ballHitInfo, value_);

	return 0;
}
}
