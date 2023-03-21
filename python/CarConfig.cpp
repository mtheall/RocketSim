#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *CarConfig::Type = nullptr;

PyMemberDef CarConfig::Members[] = {
    {.name      = "dodge_deadzone",
        .type   = T_FLOAT,
        .offset = offsetof (CarConfig, config) + offsetof (::CarConfig, dodgeDeadzone),
        .flags  = 0,
        .doc    = "Dodge deadzone"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyGetSetDef CarConfig::GetSet[] = {
    GETSET_ENTRY (CarConfig, hitbox_size),
    GETSET_ENTRY (CarConfig, hitbox_pos_offset),
    GETSET_ENTRY (CarConfig, front_wheels),
    GETSET_ENTRY (CarConfig, back_wheels),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot CarConfig::Slots[] = {
    {Py_tp_new, (void *)&CarConfig::New},
    {Py_tp_init, (void *)&CarConfig::Init},
    {Py_tp_dealloc, (void *)&CarConfig::Dealloc},
    {Py_tp_members, &CarConfig::Members},
    {Py_tp_getset, &CarConfig::GetSet},
    {0, nullptr},
};

PyType_Spec CarConfig::Spec = {
    .name      = "RocketSim.CarConfig",
    .basicsize = sizeof (CarConfig),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = CarConfig::Slots,
};

PyObject *CarConfig::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<CarConfig>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->config)::CarConfig ();

	self->hitboxSize      = nullptr;
	self->hitboxPosOffset = nullptr;
	self->frontWheels     = nullptr;
	self->backWheels      = nullptr;

	return self.giftObject ();
}

int CarConfig::Init (CarConfig *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	self_->config = ::CarConfig{};

	auto hitboxSize      = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto hitboxPosOffset = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto frontWheels =
	    PyRef<WheelPairConfig>::stealObject (WheelPairConfig::New (WheelPairConfig::Type, nullptr, nullptr));
	auto backWheels =
	    PyRef<WheelPairConfig>::stealObject (WheelPairConfig::New (WheelPairConfig::Type, nullptr, nullptr));

	if (!hitboxSize || !hitboxPosOffset || !frontWheels || !backWheels ||
	    WheelPairConfig::Init (frontWheels.borrow (), nullptr, nullptr) < 0 ||
	    WheelPairConfig::Init (backWheels.borrow (), nullptr, nullptr) < 0)
		return -1;

	PyRef<Vec>::assign (self_->hitboxSize, hitboxSize.borrowObject ());
	PyRef<Vec>::assign (self_->hitboxPosOffset, hitboxPosOffset.borrowObject ());
	PyRef<WheelPairConfig>::assign (self_->frontWheels, frontWheels.borrowObject ());
	PyRef<WheelPairConfig>::assign (self_->backWheels, backWheels.borrowObject ());

	return 0;
}

void CarConfig::Dealloc (CarConfig *self_) noexcept
{
	Py_XDECREF (self_->hitboxSize);
	Py_XDECREF (self_->hitboxPosOffset);
	Py_XDECREF (self_->frontWheels);
	Py_XDECREF (self_->backWheels);

	self_->config.~CarConfig ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *CarConfig::Gethitbox_size (CarConfig *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->hitboxSize).giftObject ();
}

int CarConfig::Sethitbox_size (CarConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'hitbox_size' attribute of 'RocketSim.CarConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->hitboxSize, value_);

	return 0;
}

PyObject *CarConfig::Gethitbox_pos_offset (CarConfig *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->hitboxPosOffset).giftObject ();
}

int CarConfig::Sethitbox_pos_offset (CarConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_TypeError, "can't delete 'hitbox_pos_offset' attribute of 'RocketSim.CarConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->hitboxPosOffset, value_);

	return 0;
}

PyObject *CarConfig::Getfront_wheels (CarConfig *self_, void *) noexcept
{
	return PyRef<WheelPairConfig>::incRef (self_->frontWheels).giftObject ();
}

int CarConfig::Setfront_wheels (CarConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'front_wheels' attribute of 'RocketSim.CarConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, WheelPairConfig::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.WheelPairConfig");
		return -1;
	}

	PyRef<WheelPairConfig>::assign (self_->frontWheels, value_);

	return 0;
}

PyObject *CarConfig::Getback_wheels (CarConfig *self_, void *) noexcept
{
	return PyRef<WheelPairConfig>::incRef (self_->backWheels).giftObject ();
}

int CarConfig::Setback_wheels (CarConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'back_wheels' attribute of 'RocketSim.CarConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, WheelPairConfig::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.WheelPairConfig");
		return -1;
	}

	PyRef<WheelPairConfig>::assign (self_->backWheels, value_);

	return 0;
}
}
