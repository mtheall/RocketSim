#include "Module.h"

#include "Array.h"

#include <algorithm>
#include <bit>
#include <cinttypes>
#include <cmath>
#include <cstdlib>
#include <new>
#include <tuple>
#include <unordered_map>

namespace
{
std::uint64_t makeKey (float a_, float b_) noexcept
{
#if __cpp_lib_bit_cast
	auto const a = std::bit_cast<std::uint32_t> (a_);
	auto const b = std::bit_cast<std::uint32_t> (b_);
#else
	static_assert (sizeof (float) == sizeof (std::uint32_t));
	std::uint32_t a, b;
	std::memcpy (&a, &a_, sizeof (a));
	std::memcpy (&b, &b_, sizeof (b));
#endif
	return (static_cast<std::uint64_t> (a) << 32) | b;
}

std::unordered_map<std::uint64_t, unsigned> const boostMapping = {
    // clang-format off
    { makeKey (    0.0f, -4240.0f),  0 },
    { makeKey (-1792.0f, -4184.0f),  1 },
    { makeKey ( 1792.0f, -4184.0f),  2 },
    { makeKey (-3072.0f, -4096.0f),  3 },
    { makeKey ( 3072.0f, -4096.0f),  4 },
    { makeKey (- 940.0f, -3308.0f),  5 },
    { makeKey (  940.0f, -3308.0f),  6 },
    { makeKey (    0.0f, -2816.0f),  7 },
    { makeKey (-3584.0f, -2484.0f),  8 },
    { makeKey ( 3584.0f, -2484.0f),  9 },
    { makeKey (-1788.0f, -2300.0f), 10 },
    { makeKey ( 1788.0f, -2300.0f), 11 },
    { makeKey (-2048.0f, -1036.0f), 12 },
    { makeKey (    0.0f, -1024.0f), 13 },
    { makeKey ( 2048.0f, -1036.0f), 14 },
    { makeKey (-3584.0f,     0.0f), 15 },
    { makeKey (-1024.0f,     0.0f), 16 },
    { makeKey ( 1024.0f,     0.0f), 17 },
    { makeKey ( 3584.0f,     0.0f), 18 },
    { makeKey (-2048.0f,  1036.0f), 19 },
    { makeKey (    0.0f,  1024.0f), 20 },
    { makeKey ( 2048.0f,  1036.0f), 21 },
    { makeKey (-1788.0f,  2300.0f), 22 },
    { makeKey ( 1788.0f,  2300.0f), 23 },
    { makeKey (-3584.0f,  2484.0f), 24 },
    { makeKey ( 3584.0f,  2484.0f), 25 },
    { makeKey (    0.0f,  2816.0f), 26 },
    { makeKey (- 940.0f,  3310.0f), 27 },
    { makeKey (  940.0f,  3308.0f), 28 },
    { makeKey (-3072.0f,  4096.0f), 29 },
    { makeKey ( 3072.0f,  4096.0f), 30 },
    { makeKey (-1792.0f,  4184.0f), 31 },
    { makeKey ( 1792.0f,  4184.0f), 32 },
    { makeKey (    0.0f,  4240.0f), 33 },
    // clang-format on
};

int getBoostPadIndex (::BoostPad const *pad_) noexcept
{
	auto const it = boostMapping.find (makeKey (pad_->pos.x, pad_->pos.y));
	if (it == std::end (boostMapping))
		return -1;

	return it->second;
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned row_, unsigned col_, btVector3 const &value_) noexcept
{
	array_ (row_, col_ + 0) = value_.x ();
	array_ (row_, col_ + 1) = value_.y ();
	array_ (row_, col_ + 2) = value_.z ();
}

void assign (RocketSim::Python::PyArrayRef &array_,
    unsigned col_,
    btVector3 const &value_,
    bool angular_ = false) noexcept
{
	assign (array_, 0, col_, value_);

	if (!angular_)
		assign (array_, 1, col_, btVector3 (-value_.x (), -value_.y (), value_.z ()));
	else
		assign (array_, 1, col_, value_);
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned col_, btQuaternion const &value_) noexcept
{
	array_ (0, col_ + 0) = value_.w ();
	array_ (0, col_ + 1) = value_.x ();
	array_ (0, col_ + 2) = value_.y ();
	array_ (0, col_ + 3) = value_.z ();

	array_ (1, col_ + 0) = value_.z ();
	array_ (1, col_ + 1) = value_.y ();
	array_ (1, col_ + 2) = -value_.x ();
	array_ (1, col_ + 3) = -value_.w ();
}

btVector3 calcPYR (btMatrix3x3 const &mat_) noexcept
{
	btVector3 result;
	mat_.getEulerYPR (result.y (), result.x (), result.z ());
	result.z () *= -1.0f;
	return result;
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned col_, btMatrix3x3 const &mat_) noexcept
{
	auto const forward = mat_.getColumn (0);
	auto const right   = mat_.getColumn (1);
	auto const up      = mat_.getColumn (2);

	assign (array_, col_ + 0, forward);
	assign (array_, col_ + 3, right);
	assign (array_, col_ + 6, up);

	assign (array_, 0, col_ + 9, calcPYR (mat_));
	assign (array_, 1, col_ + 9, calcPYR (btMatrix3x3 (-1, 0, 0, 0, -1, 0, 0, 0, 1) * mat_));
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned col_, btRigidBody *rigidBody_) noexcept
{
	assign (array_, col_ + 0, rigidBody_->getWorldTransform ().getOrigin () * BT_TO_UU);
	assign (array_, col_ + 3, rigidBody_->getOrientation ());
	assign (array_, col_ + 7, rigidBody_->getLinearVelocity () * BT_TO_UU);
	assign (array_, col_ + 10, rigidBody_->getAngularVelocity (), true);
	assign (array_, col_ + 13, rigidBody_->getWorldTransform ().getBasis ());
}
}

namespace RocketSim::Python
{
PyTypeObject *Arena::Type = nullptr;

PyMemberDef Arena::Members[] = {
    {.name = "ball", .type = T_OBJECT_EX, .offset = offsetof (Arena, ball), .flags = READONLY, .doc = "Ball"},
    {.name      = "blue_score",
        .type   = T_UINT,
        .offset = offsetof (Arena, blueScore),
        .flags  = READONLY,
        .doc    = "Blue score"},
    {.name      = "orange_score",
        .type   = T_UINT,
        .offset = offsetof (Arena, orangeScore),
        .flags  = READONLY,
        .doc    = "Orange score"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef Arena::Methods[] = {
    {.ml_name = "add_car", .ml_meth = (PyCFunction)&Arena::AddCar, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name = "clone", .ml_meth = (PyCFunction)&Arena::Clone, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name     = "get_car_from_id",
        .ml_meth  = (PyCFunction)&Arena::GetCarFromId,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name = "get_cars", .ml_meth = (PyCFunction)&Arena::GetCars, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name     = "get_boost_pads",
        .ml_meth  = (PyCFunction)&Arena::GetBoostPads,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "get_gym_state",
        .ml_meth  = (PyCFunction)&Arena::GetGymState,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "get_mutator_config",
        .ml_meth  = (PyCFunction)&Arena::GetMutatorConfig,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name = "remove_car", .ml_meth = (PyCFunction)&Arena::RemoveCar, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name     = "is_ball_probably_going_in",
        .ml_meth  = (PyCFunction)&Arena::IsBallProbablyGoingIn,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "reset_kickoff",
        .ml_meth  = (PyCFunction)&Arena::ResetKickoff,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "set_ball_touch_callback",
        .ml_meth  = (PyCFunction)&Arena::SetBallTouchCallback,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "set_boost_pickup_callback",
        .ml_meth  = (PyCFunction)&Arena::SetBoostPickupCallback,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "set_car_bump_callback",
        .ml_meth  = (PyCFunction)&Arena::SetCarBumpCallback,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "set_goal_score_callback",
        .ml_meth  = (PyCFunction)&Arena::SetGoalScoreCallback,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "set_mutator_config",
        .ml_meth  = (PyCFunction)&Arena::SetMutatorConfig,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name = "step", .ml_meth = (PyCFunction)&Arena::Step, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef Arena::GetSet[] = {
    GETONLY_ENTRY (Arena, game_mode),
    GETONLY_ENTRY (Arena, tick_count),
    GETONLY_ENTRY (Arena, tick_rate),
    GETONLY_ENTRY (Arena, tick_time),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot Arena::Slots[] = {
    {Py_tp_new, (void *)&Arena::New},
    {Py_tp_init, (void *)&Arena::Init},
    {Py_tp_dealloc, (void *)&Arena::Dealloc},
    {Py_tp_methods, &Arena::Methods},
    {Py_tp_members, &Arena::Members},
    {Py_tp_getset, &Arena::GetSet},
    {0, nullptr},
};

PyType_Spec Arena::Spec = {
    .name      = "RocketSim.Arena",
    .basicsize = sizeof (Arena),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = Arena::Slots,
};

PyObject *Arena::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<Arena>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->arena) std::shared_ptr<::Arena>{};
	self->cars                        = new (std::nothrow) std::unordered_map<std::uint32_t, PyRef<Car>>{};
	self->boostPads                   = new (std::nothrow) std::unordered_map<::BoostPad *, PyRef<BoostPad>>{};
	self->ball                        = nullptr;
	self->ballTouchCallback           = nullptr;
	self->ballTouchCallbackUserData   = nullptr;
	self->boostPickupCallback         = nullptr;
	self->boostPickupCallbackUserData = nullptr;
	self->carBumpCallback             = nullptr;
	self->carBumpCallbackUserData     = nullptr;
	self->goalScoreCallback           = nullptr;
	self->goalScoreCallbackUserData   = nullptr;
	self->blueScore                   = 0;
	self->orangeScore                 = 0;
	self->lastGoalTick                = 0;
	self->stepException               = 0;

	if (!self->cars || !self->boostPads)
	{
		self->arena.~shared_ptr ();
		delete self->cars;
		delete self->boostPads;

		return PyErr_NoMemory ();
	}

	return self.giftObject ();
}

int Arena::Init (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	int gameMode;
	float tickRate = 120.0f;

	if (!PyArg_ParseTuple (args_, "i|f", &gameMode, &tickRate))
		return -1;

	if (gameMode != static_cast<int> (::GameMode::SOCCAR) && gameMode != static_cast<int> (::GameMode::THE_VOID))
	{
		PyErr_SetString (PyExc_RuntimeError, "Invalid game mode");
		return -1;
	}

	if (tickRate < 15.0f || tickRate > 120.0f)
	{
		PyErr_SetString (PyExc_RuntimeError, "Invalid tick rate");
		return -1;
	}

	// default initialization if it hasn't been done yet
	InitInternal (nullptr);

	try
	{
		auto arena = std::shared_ptr<::Arena> (::Arena::Create (static_cast<::GameMode> (gameMode), tickRate));
		if (!arena)
			throw -1;

		arena->SetCarBumpCallback (&Arena::HandleCarBumpCallback, self_);

		if (arena->gameMode != ::GameMode::THE_VOID)
		{
			arena->SetBoostPickupCallback (&Arena::HandleBoostPickupCallback, self_);
			arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, self_);
		}

		auto ball = PyRef<Ball>::steal (Ball::New ());
		if (!ball)
			throw -1;

		auto boostPads = std::unordered_map<::BoostPad *, PyRef<BoostPad>>{};
		for (auto const &pad : arena->GetBoostPads ())
		{
			auto ref = PyRef<BoostPad>::steal (BoostPad::New ());
			if (!ref)
				throw -1;

			ref->pad = pad;
			boostPads.emplace (pad, std::move (ref));
		}

		// no exceptions thrown after this point
		self_->arena = arena;
		self_->cars->clear ();
		*self_->boostPads = std::move (boostPads);

		PyRef<Ball>::assign (self_->ball, ball.borrowObject ());
		self_->ball->arena = self_->arena;
		self_->ball->ball  = self_->arena->ball;

		PyObjectRef::assign (self_->ballTouchCallback, Py_None);
		PyObjectRef::assign (self_->ballTouchCallbackUserData, Py_None);
		PyObjectRef::assign (self_->boostPickupCallback, Py_None);
		PyObjectRef::assign (self_->boostPickupCallbackUserData, Py_None);
		PyObjectRef::assign (self_->carBumpCallback, Py_None);
		PyObjectRef::assign (self_->carBumpCallbackUserData, Py_None);
		PyObjectRef::assign (self_->goalScoreCallback, Py_None);
		PyObjectRef::assign (self_->goalScoreCallbackUserData, Py_None);

		return 0;
	}
	catch (...)
	{
		PyErr_NoMemory ();
		return -1;
	}
}

void Arena::Dealloc (Arena *self_) noexcept
{
	self_->arena.~shared_ptr ();
	delete self_->cars;
	delete self_->boostPads;
	Py_XDECREF (self_->ball);
	Py_XDECREF (self_->ballTouchCallback);
	Py_XDECREF (self_->ballTouchCallbackUserData);
	Py_XDECREF (self_->boostPickupCallback);
	Py_XDECREF (self_->boostPickupCallbackUserData);
	Py_XDECREF (self_->carBumpCallback);
	Py_XDECREF (self_->carBumpCallbackUserData);
	Py_XDECREF (self_->goalScoreCallback);
	Py_XDECREF (self_->goalScoreCallbackUserData);

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Arena::AddCar (Arena *self_, PyObject *args_) noexcept
{
	int team;
	PyObject *config = nullptr; // borrowed reference
	if (!PyArg_ParseTuple (args_, "i|O", &team, &config))
		return nullptr;

	if (team != static_cast<int> (::Team::BLUE) && team != static_cast<int> (::Team::ORANGE))
	{
		PyErr_SetString (PyExc_RuntimeError, "Invalid team");
		return nullptr;
	}

	::CarConfig carConfig = CAR_CONFIG_OCTANE;
	if (config && Py_IS_TYPE (config, CarConfig::Type))
	{
		carConfig = reinterpret_cast<CarConfig const *> (config)->config;
	}
	else if (config)
	{
		auto const configId = PyLong_AsLong (config);
		if (PyErr_Occurred ())
			return nullptr;

		if (!CarConfig::FromIndex (static_cast<CarConfig::Index> (configId), carConfig))
			return nullptr;
	}

	auto car = PyRef<Car>::steal (Car::New ());
	if (!car)
		return nullptr;

	::Car *rsCar = nullptr;

	try
	{
		rsCar = self_->arena->AddCar (static_cast<::Team> (team), carConfig);

		auto &carRef = (*self_->cars)[rsCar->id];
		if (carRef) // this probably shouldn't happen
			throw 0;

		// no exceptions thrown after this point
		car->arena = self_->arena;
		car->car   = rsCar;
		carRef     = car;

		return car.giftObject ();
	}
	catch (...)
	{
		if (rsCar)
			self_->arena->RemoveCar (rsCar);

		return PyErr_NoMemory ();
	}
}

PyObject *Arena::Clone (Arena *self_, PyObject *args_) noexcept
{
	int copyCallbacks = false;
	if (!PyArg_ParseTuple (args_, "|p", &copyCallbacks))
		return nullptr;

	auto clone = PyRef<Arena>::stealObject (New (Type, nullptr, nullptr));
	if (!clone)
		return nullptr;

	try
	{
		auto arena = std::shared_ptr<::Arena> (self_->arena->Clone (false));
		if (!arena)
			return PyErr_NoMemory ();

		auto ball = PyRef<Ball>::steal (Ball::New ());
		if (!ball)
			return PyErr_NoMemory ();

		auto boostPads = std::unordered_map<::BoostPad *, PyRef<BoostPad>>{};
		for (auto const &pad : arena->GetBoostPads ())
		{
			auto ref = PyRef<BoostPad>::steal (BoostPad::New ());
			if (!ref)
				return PyErr_NoMemory ();

			ref->pad = pad;
			boostPads.emplace (pad, std::move (ref));
		}

		auto cars = std::unordered_map<std::uint32_t, PyRef<Car>>{};
		for (auto const &car : arena->GetCars ())
		{
			auto &carRef = cars[car->id];

			carRef = PyRef<Car>::steal (Car::New ());
			if (!carRef)
				return PyErr_NoMemory ();

			carRef->arena = arena;
			carRef->car   = car;

			auto it = self_->cars->find (car->id);
			if (it != std::end (*self_->cars))
			{
				carRef->goals        = it->second->goals;
				carRef->demos        = it->second->demos;
				carRef->boostPickups = it->second->boostPickups;
			}
			else
			{
				carRef->goals        = 0;
				carRef->demos        = 0;
				carRef->boostPickups = 0;
			}
		}

		// no exceptions thrown after this point
		clone->arena      = arena;
		*clone->boostPads = std::move (boostPads);
		*clone->cars      = std::move (cars);

		PyRef<Ball>::assign (clone->ball, ball.borrowObject ());
		clone->ball->arena = clone->arena;
		clone->ball->ball  = clone->arena->ball;

		if (copyCallbacks)
		{
			PyObjectRef::assign (clone->ballTouchCallback, self_->ballTouchCallback);
			PyObjectRef::assign (clone->ballTouchCallbackUserData, self_->ballTouchCallbackUserData);
			PyObjectRef::assign (clone->boostPickupCallback, self_->boostPickupCallback);
			PyObjectRef::assign (clone->boostPickupCallbackUserData, self_->boostPickupCallbackUserData);
			PyObjectRef::assign (clone->carBumpCallback, self_->carBumpCallback);
			PyObjectRef::assign (clone->carBumpCallbackUserData, self_->carBumpCallbackUserData);
			PyObjectRef::assign (clone->goalScoreCallback, self_->goalScoreCallback);
			PyObjectRef::assign (clone->goalScoreCallbackUserData, self_->goalScoreCallbackUserData);
		}
		else
		{
			PyObjectRef::assign (clone->ballTouchCallback, Py_None);
			PyObjectRef::assign (clone->ballTouchCallbackUserData, Py_None);
			PyObjectRef::assign (clone->boostPickupCallback, Py_None);
			PyObjectRef::assign (clone->boostPickupCallbackUserData, Py_None);
			PyObjectRef::assign (clone->carBumpCallback, Py_None);
			PyObjectRef::assign (clone->carBumpCallbackUserData, Py_None);
			PyObjectRef::assign (clone->goalScoreCallback, Py_None);
			PyObjectRef::assign (clone->goalScoreCallbackUserData, Py_None);
		}

		clone->blueScore    = self_->blueScore;
		clone->orangeScore  = self_->orangeScore;
		clone->lastGoalTick = self_->lastGoalTick;

		if (clone->ballTouchCallback != Py_None)
			clone->arena->SetBallTouchCallback (&Arena::HandleBallTouchCallback, clone.borrow ());
		clone->arena->SetBoostPickupCallback (&Arena::HandleBoostPickupCallback, clone.borrow ());

		if (clone->arena->gameMode != ::GameMode::THE_VOID)
		{
			clone->arena->SetCarBumpCallback (&Arena::HandleCarBumpCallback, clone.borrow ());
			clone->arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, clone.borrow ());
		}

		return clone.giftObject ();
	}
	catch (...)
	{
		return PyErr_NoMemory ();
	}
}

PyObject *Arena::GetCarFromId (Arena *self_, PyObject *args_) noexcept
{
	unsigned id;
	PyObject *defaultResult = nullptr; // borrowed reference
	if (!PyArg_ParseTuple (args_, "I|O", &id, &defaultResult))
		return nullptr;

	auto it = self_->cars->find (id);
	if (it == std::end (*self_->cars) || !it->second)
	{
		if (!defaultResult)
			return PyErr_Format (PyExc_KeyError, "%u", id);

		return PyObjectRef::incRef (defaultResult).giftObject ();
	}

	return it->second.newObjectRef ();
}

PyObject *Arena::GetCars (Arena *self_) noexcept
{
	auto list = PyObjectRef::steal (PyList_New (self_->cars->size ()));
	if (!list)
		return nullptr;

	unsigned index = 0;
	for (auto &[id, car] : *self_->cars)
	{
		auto ref = car.newObjectRef ();

		// steals ref on success
		if (PyList_SetItem (list.borrow (), index++, ref) < 0)
		{
			// this should never happen
			Py_XDECREF (ref);
			return nullptr;
		}
	}

	return list.gift ();
}

PyObject *Arena::GetBoostPads (Arena *self_) noexcept
{
	auto list = PyObjectRef::steal (PyList_New (self_->boostPads->size ()));
	if (!list)
		return nullptr;

	unsigned index = 0;
	for (auto &[pad, ref] : *self_->boostPads)
	{
		auto newRef = ref.newObjectRef ();

		// steals ref on success
		if (PyList_SetItem (list.borrow (), index++, newRef) < 0)
		{
			// this should never happen
			Py_XDECREF (newRef);
			return nullptr;
		}
	}

	return list.gift ();
}

PyObject *Arena::GetGymState (Arena *self_) noexcept
{
	auto tuple = PyObjectRef::steal (PyTuple_New (3 + self_->cars->size ()));
	if (!tuple)
		return nullptr;

	std::uint64_t ballLastCarHitTick = 0;
	std::uint32_t ballLastCarHitId   = 0;
	for (auto const &[id, car] : *self_->cars)
	{
		if (!car->car->_internalState.ballHitInfo.isValid)
			continue;

		if (car->car->_internalState.ballHitInfo.tickCountWhenHit <= ballLastCarHitTick)
			continue;

		ballLastCarHitTick = car->car->_internalState.ballHitInfo.tickCountWhenHit;
		ballLastCarHitId   = car->car->id;
	}

	{
		auto gameData = PyArrayRef (4);
		if (!gameData)
			return nullptr;

		gameData (0) = static_cast<int> (self_->arena->gameMode);
		gameData (1) = ballLastCarHitId;
		gameData (2) = self_->blueScore;
		gameData (3) = self_->orangeScore;

		PyTuple_SetItem (tuple.borrow (), 0, gameData.giftObject ());
	}

	{
		auto const &boostPads   = self_->arena->GetBoostPads ();
		auto const numBoostPads = self_->arena->GetBoostPads ().size ();

		auto boostPadState = PyArrayRef (2, numBoostPads);
		if (!boostPadState)
			return nullptr;

		for (unsigned i = 0; i < numBoostPads; ++i)
		{
			auto const &pad = boostPads[i];

			auto const idx = getBoostPadIndex (pad);
			if (idx < 0)
				continue; // shouldn't happen

			auto const inv = numBoostPads - idx - 1;

			boostPadState (0, idx) = pad->_internalState.isActive;
			boostPadState (1, inv) = pad->_internalState.isActive;
		}

		PyTuple_SetItem (tuple.borrow (), 1, boostPadState.giftObject ());
	}

	{
		auto ballState = PyArrayRef (2, 25);
		if (!ballState)
			return nullptr;

		auto const ball = self_->arena->ball;

		assign (ballState, 0, ball->_rigidBody);

		PyTuple_SetItem (tuple.borrow (), 2, ballState.giftObject ());
	}

	unsigned carIndex = 0;
	for (auto const &[id, car] : *self_->cars)
	{
		auto carState = PyArrayRef (2, 36);
		if (!carState)
			return nullptr;

		auto const &state = car->car->_internalState;

		auto const hitLastStep = state.ballHitInfo.isValid &&
		                         state.ballHitInfo.tickCountWhenHit > self_->lastGoalTick &&
		                         state.ballHitInfo.tickCountWhenHit + 8 >= self_->arena->tickCount;

		for (unsigned i = 0; i < 2; ++i)
		{
			carState (i, 0)  = car->car->id;
			carState (i, 1)  = static_cast<int> (car->car->team);
			carState (i, 2)  = car->goals;
			carState (i, 3)  = 0; // todo saves
			carState (i, 4)  = 0; // todo shots
			carState (i, 5)  = car->demos;
			carState (i, 6)  = car->boostPickups;
			carState (i, 7)  = state.isDemoed;
			carState (i, 8)  = state.isOnGround;
			carState (i, 9)  = hitLastStep;
			carState (i, 10) = state.boost;
		}

		assign (carState, 11, car->car->_rigidBody);

		PyTuple_SetItem (tuple.borrow (), 3 + carIndex, carState.giftObject ());
		++carIndex;
	}

	return tuple.giftObject ();
}

PyObject *Arena::GetMutatorConfig (Arena *self_) noexcept
{
	auto config = MutatorConfig::NewFromMutatorConfig (self_->arena->GetMutatorConfig ());
	if (!config)
		return nullptr;

	return config.giftObject ();
}

PyObject *Arena::RemoveCar (Arena *self_, PyObject *args_) noexcept
{
	Car *car;
	if (!PyArg_ParseTuple (args_, "O!", Car::Type, &car))
		return nullptr;

	if (car->arena != self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car is not in this arena");
		return nullptr;
	}

	self_->arena->RemoveCar (car->car);
	self_->cars->erase (car->car->id);

	// detach car from arena
	car->arena.reset ();

	Py_RETURN_NONE;
}

PyObject *Arena::IsBallProbablyGoingIn (Arena *self_, PyObject *args_) noexcept
{
	float maxTime = 0.2f;
	if (!PyArg_ParseTuple (args_, "|f", &maxTime))
		return nullptr;

	return PyBool_FromLong (self_->arena->IsBallProbablyGoingIn (maxTime));
}

PyObject *Arena::ResetKickoff (Arena *self_, PyObject *args_) noexcept
{
	int seed = -1;
	if (!PyArg_ParseTuple (args_, "|i", &seed))
		return nullptr;

	self_->arena->ResetToRandomKickoff (seed);

	Py_RETURN_NONE;
}

PyObject *Arena::SetBallTouchCallback (Arena *self_, PyObject *args_) noexcept
{
	PyObject *callback; // borrowed references
	PyObject *userData = nullptr;
	if (!PyArg_ParseTuple (args_, "O|O", &callback, &userData))
		return nullptr;

	if (callback == Py_None)
		self_->arena->SetBallTouchCallback (nullptr, nullptr);
	else if (PyCallable_Check (callback))
		self_->arena->SetBallTouchCallback (&Arena::HandleBallTouchCallback, self_);
	else
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (Py_BuildValue ("OO", callback, userData ? userData : Py_None));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->ballTouchCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->ballTouchCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetBoostPickupCallback (Arena *self_, PyObject *args_) noexcept
{
	if (self_->arena->gameMode == ::GameMode::THE_VOID)
		return PyErr_Format (PyExc_RuntimeError, "Cannot set a boost pickup callback when on THE_VOID gamemode!");

	PyObject *callback; // borrowed references
	PyObject *userData = nullptr;
	if (!PyArg_ParseTuple (args_, "O|O", &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (Py_BuildValue ("OO", callback, userData ? userData : Py_None));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->boostPickupCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->boostPickupCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetCarBumpCallback (Arena *self_, PyObject *args_) noexcept
{
	PyObject *callback; // borrowed references
	PyObject *userData = nullptr;
	if (!PyArg_ParseTuple (args_, "O|O", &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (Py_BuildValue ("OO", callback, userData ? userData : Py_None));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->carBumpCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->carBumpCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetGoalScoreCallback (Arena *self_, PyObject *args_) noexcept
{
	if (self_->arena->gameMode == ::GameMode::THE_VOID)
		return PyErr_Format (PyExc_RuntimeError, "Cannot set a goal score callback when on THE_VOID gamemode!");

	PyObject *callback; // borrowed references
	PyObject *userData = nullptr;
	if (!PyArg_ParseTuple (args_, "O|O", &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (Py_BuildValue ("OO", callback, userData ? userData : Py_None));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->goalScoreCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->goalScoreCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetMutatorConfig (Arena *self_, PyObject *args_) noexcept
{
	MutatorConfig *config;
	if (!PyArg_ParseTuple (args_, "O!", MutatorConfig::Type, &config))
		return nullptr;

	self_->arena->SetMutatorConfig (MutatorConfig::ToMutatorConfig (config));

	Py_RETURN_NONE;
}

PyObject *Arena::Step (Arena *self_, PyObject *args_) noexcept
{
	int ticksToSimulate = 1;
	if (!PyArg_ParseTuple (args_, "|i", &ticksToSimulate))
		return nullptr;

	self_->stepException = false;
	self_->arena->Step (ticksToSimulate);
	if (self_->stepException)
		return nullptr;

	Py_RETURN_NONE;
}

void Arena::HandleBallTouchCallback (::Arena *arena_, ::Car *car_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepException)
		return;

	if (self->ballTouchCallback == Py_None)
		return;

	auto it = self->cars->find (car_->id);
	if (it == std::end (*self->cars) || !it->second)
	{
		PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", car_->id);
		self->stepException = true;
		return;
	}

	auto const car = it->second;

	auto const ref      = PyRef<Arena>::incRef (self);
	auto const callback = PyObjectRef::incRef (self->ballTouchCallback);
	auto const userData = PyObjectRef::incRef (self->ballTouchCallbackUserData);

	auto const args = PyObjectRef::steal (Py_BuildValue ("OOO", ref.borrow (), car.borrow (), userData.borrow ()));
	if (!args)
	{
		self->stepException = true;
		return;
	}

	auto const result = PyObjectRef::steal (PyObject_Call (callback.borrow (), args.borrow (), nullptr));
	if (!result)
		self->stepException = true;
}

void Arena::HandleBoostPickupCallback (::Arena *arena_, ::Car *car_, ::BoostPad *boostPad_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepException)
		return;

	auto it = self->cars->find (car_->id);
	if (it == std::end (*self->cars) || !it->second)
	{
		// this should never happen
		PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", car_->id);
		self->stepException = true;
		return;
	}

	auto it2 = self->boostPads->find (boostPad_);
	if (it2 == std::end (*self->boostPads) || !it->second)
	{
		// this should never happen
		PyErr_Format (PyExc_KeyError, "Boost pad not found");
		self->stepException = true;
		return;
	}

	auto car = it->second;
	++car->boostPickups;

	if (self->boostPickupCallback == Py_None)
		return;

	auto boostPad = it2->second;

	auto const ref      = PyRef<Arena>::incRef (self);
	auto const callback = PyObjectRef::incRef (self->boostPickupCallback);
	auto const userData = PyObjectRef::incRef (self->boostPickupCallbackUserData);

	auto const args = PyObjectRef::steal (
	    Py_BuildValue ("OOOO", ref.borrowObject (), car.borrowObject (), boostPad.borrowObject (), userData.borrow ()));
	if (!args)
	{
		self->stepException = true;
		return;
	}

	auto const result = PyObjectRef::steal (PyObject_Call (callback.borrow (), args.borrow (), nullptr));
	if (!result)
		self->stepException = true;
}

void Arena::HandleCarBumpCallback (::Arena *arena_,
    ::Car *bumper_,
    ::Car *victim_,
    bool isDemo_,
    void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepException)
		return;

	auto it = self->cars->find (bumper_->id);
	if (it == std::end (*self->cars) || !it->second)
	{
		PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", bumper_->id);
		self->stepException = true;
		return;
	}

	auto bumper = it->second;
	if (isDemo_)
		++bumper->demos;

	if (self->carBumpCallback == Py_None)
		return;

	it = self->cars->find (victim_->id);
	if (it == std::end (*self->cars) || !it->second)
	{
		PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", victim_->id);
		self->stepException = true;
		return;
	}

	auto const victim = it->second;

	auto const ref      = PyRef<Arena>::incRef (self);
	auto const callback = PyObjectRef::incRef (self->carBumpCallback);
	auto const userData = PyObjectRef::incRef (self->carBumpCallbackUserData);

	auto const args = PyObjectRef::steal (Py_BuildValue (
	    "OOOOO", ref.borrow (), bumper.borrow (), victim.borrow (), PyBool_FromLong (isDemo_), userData.borrow ()));
	if (!args)
	{
		self->stepException = true;
		return;
	}

	auto const result = PyObjectRef::steal (PyObject_Call (callback.borrow (), args.borrow (), nullptr));
	if (!result)
		self->stepException = true;
}

void Arena::HandleGoalScoreCallback (::Arena *arena_, ::Team scoringTeam_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepException)
		return;

	// avoid continuously counting goals until the ball exits goal zone
	if (self->lastGoalTick + 1 != self->arena->tickCount)
	{
		if (scoringTeam_ == ::Team::BLUE)
			++self->blueScore;
		else
			++self->orangeScore;

		// find which car scored
		Car *best = nullptr;
		for (auto const &[id, car] : *self->cars)
		{
			if (scoringTeam_ != car->car->team)
				continue;

			if (!car->car->_internalState.ballHitInfo.isValid ||
			    car->car->_internalState.ballHitInfo.tickCountWhenHit < self->lastGoalTick)
				continue;

			if (!best || best->car->_internalState.ballHitInfo.tickCountWhenHit <
			                 car->car->_internalState.ballHitInfo.tickCountWhenHit)
				best = car.borrow ();
		}

		if (best)
			++best->goals;
	}

	self->lastGoalTick = self->arena->tickCount;

	if (self->goalScoreCallback == Py_None)
		return;

	auto const ref      = PyRef<Arena>::incRef (self);
	auto const callback = PyObjectRef::incRef (self->goalScoreCallback);
	auto const userData = PyObjectRef::incRef (self->goalScoreCallbackUserData);

	auto const team = static_cast<int> (scoringTeam_);
	auto const args = PyObjectRef::steal (Py_BuildValue ("OiO", ref.borrow (), team, userData.borrow ()));
	if (!args)
	{
		self->stepException = true;
		return;
	}

	auto const result = PyObjectRef::steal (PyObject_Call (callback.borrow (), args.borrow (), nullptr));
	if (!result)
		self->stepException = true;
}

PyObject *Arena::Getgame_mode (Arena *self_, void *) noexcept
{
	return PyLong_FromLong (static_cast<int> (self_->arena->gameMode));
}

PyObject *Arena::Gettick_count (Arena *self_, void *) noexcept
{
	return PyLong_FromLong (self_->arena->tickCount);
}

PyObject *Arena::Gettick_rate (Arena *self_, void *) noexcept
{
	return PyFloat_FromDouble (self_->arena->GetTickRate ());
}

PyObject *Arena::Gettick_time (Arena *self_, void *) noexcept
{
	return PyFloat_FromDouble (self_->arena->tickTime);
}
}
