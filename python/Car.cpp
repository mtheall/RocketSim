#include "Module.h"

namespace RocketSim::Python
{
PyTypeObject *Car::Type = nullptr;

PyMethodDef Car::Methods[] = {
    {.ml_name     = "get_config",
        .ml_meth  = (PyCFunction)&Car::GetConfig,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get car config"},
    {.ml_name     = "get_controls",
        .ml_meth  = (PyCFunction)&Car::GetControls,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get car controls"},
    {.ml_name     = "get_state",
        .ml_meth  = (PyCFunction)&Car::GetState,
        .ml_flags = METH_NOARGS,
        .ml_doc   = "Get car state"},
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
    {Py_tp_new, nullptr},
    {Py_tp_init, nullptr},
    {Py_tp_dealloc, (void *)&Car::Dealloc},
    {Py_tp_methods, &Car::Methods},
    {Py_tp_getset, &Car::GetSet},
    {0, nullptr},
};

PyType_Spec Car::Spec = {
    .name      = "RocketSim.Car",
    .basicsize = sizeof (Car),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots     = Car::Slots,
};

Car *Car::New () noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (Type, Py_tp_alloc);

	auto self = PyRef<Car>::stealObject (tp_alloc (Type, 0));
	if (!self)
		return nullptr;

	new (&self->arena) std::shared_ptr<::Arena>{};
	self->car = nullptr;

	return self.gift ();
}

void Car::Dealloc (Car *self_) noexcept
{
	self_->arena.~shared_ptr ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Car::Getid (Car *self_, void *) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	return PyLong_FromLong (self_->car->id);
}

PyObject *Car::Getteam (Car *self_, void *) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	return PyLong_FromLong (static_cast<int> (self_->car->team));
}

PyObject *Car::GetConfig (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto config = PyRef<CarConfig>::stealObject (CarConfig::New (CarConfig::Type, nullptr, nullptr));
	if (!config || CarConfig::Init (config.borrow (), nullptr, nullptr) < 0)
		return nullptr;

	// copy values from config
	config->config                                  = self_->car->config;
	config->hitboxSize->vec                         = config->config.hitboxSize;
	config->hitboxPosOffset->vec                    = config->config.hitboxPosOffset;
	config->frontWheels->config                     = config->config.frontWheels;
	config->frontWheels->connectionPointOffset->vec = config->config.frontWheels.connectionPointOffset;
	config->backWheels->config                      = config->config.backWheels;
	config->backWheels->connectionPointOffset->vec  = config->config.backWheels.connectionPointOffset;

	return config.giftObject ();
}

PyObject *Car::GetControls (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto controls = PyRef<CarControls>::stealObject (CarControls::New (CarControls::Type, nullptr, nullptr));
	if (!controls)
		return nullptr;

	// copy values from controls
	controls->controls = self_->car->controls;

	return controls.giftObject ();
}

PyObject *Car::GetState (Car *self_) noexcept
{
	if (!self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car does not belong to any arena");
		return nullptr;
	}

	auto state = PyRef<CarState>::stealObject (CarState::New (CarState::Type, nullptr, nullptr));
	if (!state || CarState::Init (state.borrow (), nullptr, nullptr) != 0)
		return nullptr;

	// copy values from state
	state->state                   = self_->car->GetState ();
	state->pos->vec                = state->state.pos;
	state->angles->angle           = state->state.angles;
	state->vel->vec                = state->state.vel;
	state->angVel->vec             = state->state.angVel;
	state->lastRelDodgeTorque->vec = state->state.lastRelDodgeTorque;
	state->lastControls->controls  = state->state.lastControls;
	state->worldContactNormal->vec = state->state.worldContact.contactNormal;

	return state.giftObject ();
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

	// copy values to controls
	self_->car->controls = controls->controls;

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

	// copy values to state
	::CarState newState                 = state->state;
	newState.pos                        = state->pos->vec;
	newState.angles                     = state->angles->angle;
	newState.vel                        = state->vel->vec;
	newState.angVel                     = state->angVel->vec;
	newState.lastRelDodgeTorque         = state->lastRelDodgeTorque->vec;
	newState.lastControls               = state->lastControls->controls;
	newState.worldContact.contactNormal = state->worldContactNormal->vec;

	self_->car->SetState (newState);

	Py_RETURN_NONE;
}
}
