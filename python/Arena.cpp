#include "Module.h"

#include "../src/RocketSim.h"

#include <mutex>
#include <new>

namespace
{
std::once_flag init;
}

namespace RocketSim::Python
{
PyTypeObject *Arena::Type = nullptr;

PyMemberDef Arena::Members[] = {
    {.name = "ball", .type = T_OBJECT_EX, .offset = offsetof (Arena, ball), .flags = READONLY, .doc = "Ball"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef Arena::Methods[] = {
    {.ml_name = "add_car", .ml_meth = (PyCFunction)&Arena::AddCar, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name = "clone", .ml_meth = (PyCFunction)&Arena::Clone, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name     = "get_car_from_id",
        .ml_meth  = (PyCFunction)&Arena::GetCarFromId,
        .ml_flags = METH_VARARGS,
        .ml_doc   = nullptr},
    {.ml_name = "get_cars", .ml_meth = (PyCFunction)&Arena::GetCars, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name     = "get_boost_pads",
        .ml_meth  = (PyCFunction)&Arena::GetBoostPads,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name = "remove_car", .ml_meth = (PyCFunction)&Arena::RemoveCar, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name = "reset_kickoff", .ml_meth = (PyCFunction)&Arena::ResetKickoff, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name = "set_goal_score_callback", .ml_meth = (PyCFunction)&Arena::SetGoalScoreCallback, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
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
	self->cars                      = new (std::nothrow) std::map<std::uint32_t, PyRef<Car>>{};
	self->boostPads                 = new (std::nothrow) std::vector<PyRef<BoostPad>>{};
	self->ball                      = nullptr;
	self->goalScoreCallback         = nullptr;
	self->goalScoreCallbackUserData = nullptr;

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

	std::call_once (init, &RocketSim::Init, "collision_meshes");

	try
	{
		auto arena = std::shared_ptr<::Arena> (::Arena::Create (static_cast<GameMode> (gameMode), tickRate));
		if (!arena)
			throw -1;

		auto ball = PyRef<Ball>::steal (Ball::New ());
		if (!ball)
			throw -1;

		auto boostPads = std::vector<PyRef<BoostPad>>{};
		boostPads.reserve (arena->GetBoostPads ().size ());
		for (auto const &pad : arena->GetBoostPads ())
		{
			auto &back = boostPads.emplace_back (PyRef<BoostPad>::steal (BoostPad::New ()));
			if (!back)
				throw -1;

			back->pad = pad;
		}

		// no exceptions thrown after this point
		self_->arena = arena;
		self_->cars->clear ();
		*self_->boostPads = std::move (boostPads);

		PyRef<Ball>::assign (self_->ball, ball.borrowObject ());
		self_->ball->arena = self_->arena;
		self_->ball->ball  = self_->arena->ball;

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
	Py_XDECREF (self_->goalScoreCallback);
	Py_XDECREF (self_->goalScoreCallbackUserData);

	auto const tp_free = (freefunc)PyType_GetSlot ((PyTypeObject *)Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Arena::AddCar (Arena *self_, PyObject *args_) noexcept
{
	int team;
	int configId = 0;
	if (!PyArg_ParseTuple (args_, "i|i", &team, &configId))
		return nullptr;

	if (team < 0 || team > 1)
	{
		PyErr_SetString (PyExc_RuntimeError, "Invalid team");
		return nullptr;
	}

	if (configId < 0 || configId > 5)
	{
		PyErr_SetString (PyExc_RuntimeError, "Invalid car configuration");
		return nullptr;
	}

	static ::CarConfig const *const configs[] = {
	    &CAR_CONFIG_OCTANE,
	    &CAR_CONFIG_DOMINUS,
	    &CAR_CONFIG_PLANK,
	    &CAR_CONFIG_BREAKOUT,
	    &CAR_CONFIG_HYBRID,
	    &CAR_CONFIG_MERC,
	};

	auto car = PyRef<Car>::steal (Car::New ());
	if (!car)
		return nullptr;

	::Car *rsCar = nullptr;

	try
	{
		rsCar = self_->arena->AddCar (static_cast<Team> (team), *configs[configId]);

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

PyObject *Arena::Clone (Arena *self_) noexcept
{
	auto clone = PyRef<Arena>::stealObject (New (Type, nullptr, nullptr));
	if (!clone)
		return nullptr;

	try
	{
		auto arena = std::shared_ptr<::Arena> (self_->arena->Clone (true));
		if (!arena)
			return PyErr_NoMemory ();

		auto ball = PyRef<Ball>::steal (Ball::New ());
		if (!ball)
			return PyErr_NoMemory ();

		auto boostPads = std::vector<PyRef<BoostPad>>{};
		boostPads.reserve (arena->GetBoostPads ().size ());
		for (auto const &pad : arena->GetBoostPads ())
		{
			auto &back = boostPads.emplace_back (PyRef<BoostPad>::steal (BoostPad::New ()));
			if (!back)
				return PyErr_NoMemory ();

			back->pad = pad;
		}

		auto cars = std::map<std::uint32_t, PyRef<Car>>{};
		for (auto const &car : arena->GetCars ())
		{
			auto &carRef = cars[car->id];

			carRef = PyRef<Car>::steal (Car::New ());
			if (!carRef)
				return PyErr_NoMemory ();

			carRef->arena = arena;
			carRef->car   = car;
		}

		// no exceptions thrown after this point
		clone->arena      = arena;
		*clone->boostPads = std::move (boostPads);
		*clone->cars      = std::move (cars);

		PyRef<Ball>::assign (clone->ball, ball.borrowObject ());
		clone->ball->arena = clone->arena;
		clone->ball->ball  = clone->arena->ball;

		PyObjectRef::assign (clone->goalScoreCallback, self_->goalScoreCallback);
		PyObjectRef::assign (clone->goalScoreCallbackUserData, self_->goalScoreCallbackUserData);

		if (clone->goalScoreCallback != Py_None)
			clone->arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, clone.borrow ());

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
	if (!PyArg_ParseTuple (args_, "I", &id))
		return nullptr;

	auto it = self_->cars->find (id);
	if (it == std::end (*self_->cars) || !it->second)
		return PyErr_Format (PyExc_KeyError, "%u", id);

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
	for (auto &pad : *self_->boostPads)
	{
		auto ref = pad.newObjectRef ();

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

	// detach car from arena
	car->arena.reset ();

	Py_RETURN_NONE;
}

PyObject *Arena::ResetKickoff (Arena *self_, PyObject *args_) noexcept
{
	int seed = -1;
	if (!PyArg_ParseTuple (args_, "|i", &seed))
		return nullptr;

	self_->arena->ResetToRandomKickoff (seed);

	Py_RETURN_NONE;
}

PyObject *Arena::SetGoalScoreCallback (Arena *self_, PyObject *args_) noexcept
{
	PyObject *callback; // borrowed references
	PyObject *userData;
	if (!PyArg_ParseTuple (args_, "OO", &callback, &userData))
		return nullptr;

	if (!PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object");
		return nullptr;
	}

	PyObjectRef::assign (self_->goalScoreCallback, callback);
	PyObjectRef::assign (self_->goalScoreCallbackUserData, userData);

	self_->arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, self_);

	Py_RETURN_NONE;
}

PyObject *Arena::Step (Arena *self_, PyObject *args_) noexcept
{
	int ticksToSimulate = 1;
	if (!PyArg_ParseTuple (args_, "|i", &ticksToSimulate))
		return nullptr;

	self_->arena->Step (ticksToSimulate);

	Py_RETURN_NONE;
}

void Arena::HandleGoalScoreCallback (::Arena *arena_, Team scoringTeam_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena*> (userData_);

	if (self->goalScoreCallback == Py_None)
		return;

	auto const ref      = PyRef<Arena>::incRef (self);
	auto const callback = PyObjectRef::incRef (self->goalScoreCallback);
	auto const userData = PyObjectRef::incRef (self->goalScoreCallbackUserData);

	auto const team = static_cast<std::underlying_type_t<Team>> (scoringTeam_);
	auto const args = PyObjectRef::steal (Py_BuildValue ("OiO", ref.borrow (), team, userData.borrow ()));
	if (!args)
		return;

	Py_XDECREF (PyObject_Call (callback.borrow (), args.borrow (), nullptr));
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
