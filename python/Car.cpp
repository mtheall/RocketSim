#include "Module.h"

namespace RocketSim::Python
{
PyTypeObject *Car::Type = nullptr;

PyMemberDef Car::Members[] = {
    {.name      = "boost_pickups",
        .type   = TypeHelper<decltype (Car::boostPickups)>::type,
        .offset = offsetof (Car, boostPickups),
        .flags  = 0,
        .doc    = "Boost pickups"},
    {.name      = "demos",
        .type   = TypeHelper<decltype (Car::demos)>::type,
        .offset = offsetof (Car, demos),
        .flags  = 0,
        .doc    = "Demos"},
    {.name      = "goals",
        .type   = TypeHelper<decltype (Car::goals)>::type,
        .offset = offsetof (Car, goals),
        .flags  = 0,
        .doc    = "Goals"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef Car::Methods[] = {
    {.ml_name = "demolish", .ml_meth = (PyCFunction)&Car::Demolish, .ml_flags = METH_NOARGS, .ml_doc = "Demolish"},
    {.ml_name     = "get_config",
        .ml_meth  = (PyCFunction)&Car::GetConfig,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get car config"},
    {.ml_name     = "get_controls",
        .ml_meth  = (PyCFunction)&Car::GetControls,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get car controls"},
    {.ml_name     = "get_forward_dir",
        .ml_meth  = (PyCFunction)&Car::GetForwardDir,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get forward direction"},
    {.ml_name     = "get_right_dir",
        .ml_meth  = (PyCFunction)&Car::GetRightDir,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get right direction"},
    {.ml_name     = "get_state",
        .ml_meth  = (PyCFunction)&Car::GetState,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get car state"},
    {.ml_name     = "get_up_dir",
        .ml_meth  = (PyCFunction)&Car::GetUpDir,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get up direction"},
    {.ml_name = "respawn", .ml_meth = (PyCFunction)&Car::Respawn, .ml_flags = METH_VARARGS, .ml_doc = "Respawn"},
    {.ml_name     = "set_controls",
        .ml_meth  = (PyCFunction)&Car::SetControls,
        .ml_flags = METH_VARARGS,
        .ml_doc   = "Set car controls"},
    {.ml_name     = "set_state",
        .ml_meth  = (PyCFunction)&Car::SetState,
        .ml_flags = METH_VARARGS,
        .ml_doc   = "Set car state"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef Car::GetSet[] = {
    GETONLY_ENTRY (Car, id),
    GETONLY_ENTRY (Car, team),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot Car::Slots[] = {
    {Py_tp_new, (void *)&Car::NewStub},
    {Py_tp_init, nullptr},
    {Py_tp_dealloc, (void *)&Car::Dealloc},
    {Py_tp_members, &Car::Members},
    {Py_tp_methods, &Car::Methods},
    {Py_tp_getset, &Car::GetSet},
    {0, nullptr},
};

PyType_Spec Car::Spec = {
    .name      = "RocketSim.Car",
    .basicsize = sizeof (Car),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = Car::Slots,
};

Car *Car::New () noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (Type, Py_tp_alloc);

	auto self = PyRef<Car>::stealObject (tp_alloc (Type, 0));
	if (!self)
		return nullptr;

	new (&self->arena) std::shared_ptr<::Arena>{};
	self->car   = nullptr;
	self->goals = 0;

	return self.gift ();
}

PyObject *Car::NewStub (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	PyErr_SetString (PyExc_TypeError, "cannot create 'RocketSim.Car' instances");
	return nullptr;
}

void Car::Dealloc (Car *self_) noexcept
{
	self_->arena.~shared_ptr ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Car::InternalPickle (Car *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	if (!DictSetValue (dict.borrow (), "id", PyLong_FromUnsignedLong (self_->car->id)))
		return nullptr;

	if (!DictSetValue (dict.borrow (), "state", CarState::NewFromCarState (self_->car->GetState ()).giftObject ()))
		return nullptr;

	if (!DictSetValue (dict.borrow (), "config", CarConfig::NewFromCarConfig (self_->car->config).giftObject ()))
		return nullptr;

	if (!DictSetValue (
	        dict.borrow (), "controls", CarControls::NewFromCarControls (self_->car->controls).giftObject ()))
		return nullptr;

	if (self_->car->team != ::Team::BLUE &&
	    !DictSetValue (dict.borrow (), "team", PyLong_FromLong (static_cast<long> (self_->car->team))))
		return nullptr;

	if (self_->goals && !DictSetValue (dict.borrow (), "goals", PyLong_FromUnsignedLong (self_->goals)))
		return nullptr;

	if (self_->demos && !DictSetValue (dict.borrow (), "demos", PyLong_FromUnsignedLong (self_->demos)))
		return nullptr;

	if (self_->boostPickups &&
	    !DictSetValue (dict.borrow (), "boost_pickups", PyLong_FromUnsignedLong (self_->boostPickups)))
		return nullptr;

	return dict.gift ();
}

PyObject *Car::InternalUnpickle (std::shared_ptr<::Arena> arena_, Car *self_, PyObject *dict_) noexcept
{
	auto const dummy = PyObjectRef::steal (PyTuple_New (0));
	if (!dummy)
		return nullptr;

	static char idKwd[]           = "id";
	static char teamKwd[]         = "team";
	static char stateKwd[]        = "state";
	static char configKwd[]       = "config";
	static char controlsKwd[]     = "controls";
	static char goalsKwd[]        = "goals";
	static char demosKwd[]        = "demos";
	static char boostPickupsKwd[] = "boost_pickups";

	static char *dict[] = {
	    idKwd, teamKwd, stateKwd, configKwd, controlsKwd, goalsKwd, demosKwd, boostPickupsKwd, nullptr};

	PyObject *state       = nullptr; // borrowed references
	PyObject *config      = nullptr;
	PyObject *controls    = nullptr;
	unsigned long id      = 0;
	unsigned goals        = 0;
	unsigned demos        = 0;
	unsigned boostPickups = 0;
	int team              = static_cast<int> (::Team::BLUE);
	if (!PyArg_ParseTupleAndKeywords (dummy.borrow (),
	        dict_,
	        "|kiO!O!O!III",
	        dict,
	        &id,
	        &team,
	        CarState::Type,
	        &state,
	        CarConfig::Type,
	        &config,
	        CarControls::Type,
	        &controls,
	        &goals,
	        &demos,
	        &boostPickups))
		return nullptr;

	if (id == 0)
		return PyErr_Format (PyExc_ValueError, "Invalid id '%lu'", id);

	if (arena_->_carIDMap.contains (id))
		return PyErr_Format (PyExc_ValueError, "Car with id '%lu' already exists", id);

	if (static_cast<::Team> (team) != ::Team::BLUE && static_cast<::Team> (team) != ::Team::ORANGE)
		return PyErr_Format (PyExc_ValueError, "Invalid team '%d'", team);

	if (!state)
		return PyErr_Format (PyExc_ValueError, "Car state missing");

	if (!config)
		return PyErr_Format (PyExc_ValueError, "Car config missing");

	if (!controls)
		return PyErr_Format (PyExc_ValueError, "Car controls missing");

	arena_->_lastCarID = id - 1;

	self_->car = arena_->AddCar (static_cast<::Team> (team), CarConfig::ToCarConfig (PyCast<CarConfig> (config)));

	self_->arena        = arena_;
	self_->goals        = goals;
	self_->demos        = demos;
	self_->boostPickups = boostPickups;

	self_->car->SetState (CarState::ToCarState (PyCast<CarState> (state)));
	self_->car->controls = CarControls::ToCarControls (PyCast<CarControls> (controls));

	Py_RETURN_NONE;
}

PyObject *Car::Getid (Car *self_, void *) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	return PyLong_FromUnsignedLong (self_->car->id);
}

PyObject *Car::Getteam (Car *self_, void *) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	return PyLong_FromLong (static_cast<long> (self_->car->team));
}

PyObject *Car::Demolish (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	self_->car->Demolish ();

	Py_RETURN_NONE;
}

PyObject *Car::GetConfig (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto config = CarConfig::NewFromCarConfig (self_->car->config);
	if (!config)
		return nullptr;

	return config.giftObject ();
}

PyObject *Car::GetControls (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto controls = CarControls::NewFromCarControls (self_->car->controls);
	if (!controls)
		return nullptr;

	return controls.giftObject ();
}

PyObject *Car::GetForwardDir (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto dir = Vec::NewFromVec (self_->car->GetForwardDir ());
	if (!dir)
		return nullptr;

	return dir.giftObject ();
}

PyObject *Car::GetRightDir (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto dir = Vec::NewFromVec (self_->car->GetRightDir ());
	if (!dir)
		return nullptr;

	return dir.giftObject ();
}

PyObject *Car::GetState (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto state = CarState::NewFromCarState (self_->car->GetState ());
	if (!state)
		return nullptr;

	return state.giftObject ();
}

PyObject *Car::GetUpDir (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto dir = Vec::NewFromVec (self_->car->GetUpDir ());
	if (!dir)
		return nullptr;

	return dir.giftObject ();
}

PyObject *Car::Respawn (Car *self_, PyObject *args_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	int seed = -1;
	if (!PyArg_ParseTuple (args_, "|i", &seed))
		return nullptr;

	self_->car->Respawn (seed);

	Py_RETURN_NONE;
}

PyObject *Car::SetControls (Car *self_, PyObject *args_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	CarControls *controls; // borrowed reference
	if (!PyArg_ParseTuple (args_, "O!", CarControls::Type, &controls))
		return nullptr;

	self_->car->controls = CarControls::ToCarControls (controls);

	Py_RETURN_NONE;
}

PyObject *Car::SetState (Car *self_, PyObject *args_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	CarState *state; // borrowed reference
	if (!PyArg_ParseTuple (args_, "O!", CarState::Type, &state))
		return nullptr;

	self_->car->SetState (CarState::ToCarState (state));

	Py_RETURN_NONE;
}
}