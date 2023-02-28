#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *CarState::Type = nullptr;

PyMemberDef CarState::Members[] = {
    {.name      = "is_on_ground",
        .type   = T_BOOL,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isOnGround),
        .flags  = 0,
        .doc    = "Is on ground"},
    {.name      = "has_jumped",
        .type   = T_BOOL,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasJumped),
        .flags  = 0,
        .doc    = "Has jumped"},
    {.name      = "has_double_jumped",
        .type   = T_BOOL,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasDoubleJumped),
        .flags  = 0,
        .doc    = "Has double jumped"},
    {.name      = "has_flipped",
        .type   = T_BOOL,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasFlipped),
        .flags  = 0,
        .doc    = "Has flipped"},
    {.name      = "jump_time",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, jumpTime),
        .flags  = 0,
        .doc    = "Jump time"},
    {.name      = "flip_time",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, flipTime),
        .flags  = 0,
        .doc    = "Flip time"},
    {.name      = "is_jumping",
        .type   = T_BOOL,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isJumping),
        .flags  = 0,
        .doc    = "Is jumping"},
    {.name      = "air_time_since_jump",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, airTimeSinceJump),
        .flags  = 0,
        .doc    = "Air time since jump"},
    {.name      = "boost",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, boost),
        .flags  = 0,
        .doc    = "Boost"},
    {.name      = "time_spent_boosting",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, timeSpentBoosting),
        .flags  = 0,
        .doc    = "Time spent boosting"},
    {.name      = "is_supersonic",
        .type   = T_BOOL,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isSupersonic),
        .flags  = 0,
        .doc    = "Is supersonic"},
    {.name      = "supersonic_time",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, supersonicTime),
        .flags  = 0,
        .doc    = "Supersonic time"},
    {.name      = "handbrake_val",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, handbrakeVal),
        .flags  = 0,
        .doc    = "Handbrake val"},
    {.name      = "is_auto_flipping",
        .type   = T_BOOL,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isAutoFlipping),
        .flags  = 0,
        .doc    = "Is auto flipping"},
    {.name      = "auto_flip_timer",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, autoFlipTimer),
        .flags  = 0,
        .doc    = "Auto flip timer"},
    {.name      = "auto_flip_torque_scale",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, autoFlipTorqueScale),
        .flags  = 0,
        .doc    = "Auto flip torque scale"},
    {.name      = "has_world_contact",
        .type   = T_FLOAT,
        .offset = offsetof (CarState, state) + offsetof (::CarState, worldContact.hasContact),
        .flags  = 0,
        .doc    = "Has world contact"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef CarState::Methods[] = {
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef CarState::GetSet[] = {
    GETSET_ENTRY (CarState, pos),
    GETSET_ENTRY (CarState, angles),
    GETSET_ENTRY (CarState, vel),
    GETSET_ENTRY (CarState, ang_vel),
    GETSET_ENTRY (CarState, last_rel_dodge_torque),
    GETSET_ENTRY (CarState, last_controls),
    GETSET_ENTRY (CarState, world_contact_normal),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot CarState::Slots[] = {
    {Py_tp_new, (void *)&CarState::New},
    {Py_tp_init, (void *)&CarState::Init},
    {Py_tp_dealloc, (void *)&CarState::Dealloc},
    {Py_tp_members, &CarState::Members},
    {Py_tp_methods, &CarState::Methods},
    {Py_tp_getset, &CarState::GetSet},
    {0, nullptr},
};

PyType_Spec CarState::Spec = {
    .name      = "RocketSim.CarState",
    .basicsize = sizeof (CarState),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = CarState::Slots,
};

PyObject *CarState::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<CarState>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->state)::CarState ();

	self->pos                = nullptr;
	self->angles             = nullptr;
	self->vel                = nullptr;
	self->angVel             = nullptr;
	self->lastRelDodgeTorque = nullptr;
	self->lastControls       = nullptr;
	self->worldContactNormal = nullptr;

	return self.giftObject ();
}

int CarState::Init (CarState *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto pos                = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto angles             = PyRef<Angle>::stealObject (Angle::New (Angle::Type, nullptr, nullptr));
	auto vel                = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto angVel             = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto lastRelDodgeTorque = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto lastControls       = PyRef<CarControls>::stealObject (CarControls::New (CarControls::Type, nullptr, nullptr));
	auto worldContactNormal = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));

	if (!pos || !angles || !vel || !angVel || !lastRelDodgeTorque || !lastControls || !worldContactNormal)
		return -1;

	self_->state = ::CarState{};

	PyRef<Vec>::assign (self_->pos, pos.borrowObject ());
	PyRef<Angle>::assign (self_->angles, angles.borrowObject ());
	PyRef<Vec>::assign (self_->vel, vel.borrowObject ());
	PyRef<Vec>::assign (self_->angVel, angVel.borrowObject ());
	PyRef<Vec>::assign (self_->lastRelDodgeTorque, lastRelDodgeTorque.borrowObject ());
	PyRef<CarControls>::assign (self_->lastControls, lastControls.borrowObject ());
	PyRef<Vec>::assign (self_->worldContactNormal, worldContactNormal.borrowObject ());

	self_->pos->vec                = self_->state.pos;
	self_->angles->angle           = self_->state.angles;
	self_->vel->vec                = self_->state.vel;
	self_->angVel->vec             = self_->state.angVel;
	self_->lastRelDodgeTorque->vec = self_->state.lastRelDodgeTorque;
	self_->lastControls->controls  = self_->state.lastControls;
	self_->worldContactNormal->vec = self_->state.worldContact.contactNormal;

	return 0;
}

void CarState::Dealloc (CarState *self_) noexcept
{
	Py_XDECREF (self_->pos);
	Py_XDECREF (self_->angles);
	Py_XDECREF (self_->vel);
	Py_XDECREF (self_->angVel);
	Py_XDECREF (self_->lastRelDodgeTorque);
	Py_XDECREF (self_->lastControls);

	self_->state.~CarState ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
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

PyObject *CarState::Getangles (CarState *self_, void *) noexcept
{
	return PyRef<Angle>::incRef (self_->angles).giftObject ();
}

int CarState::Setangles (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'angle' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Angle::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Angle");
		return -1;
	}

	PyRef<Angle>::assign (self_->angles, value_);

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
}
