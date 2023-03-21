#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *WheelPairConfig::Type = nullptr;

PyMemberDef WheelPairConfig::Members[] = {
    {.name      = "wheel_radius",
        .type   = T_FLOAT,
        .offset = offsetof (WheelPairConfig, config) + offsetof (::WheelPairConfig, wheelRadius),
        .flags  = 0,
        .doc    = "Wheel radius"},
    {.name      = "suspension_rest_length",
        .type   = T_FLOAT,
        .offset = offsetof (WheelPairConfig, config) + offsetof (::WheelPairConfig, suspensionRestLength),
        .flags  = 0,
        .doc    = "Suspension rest length"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef WheelPairConfig::Methods[] = {
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef WheelPairConfig::GetSet[] = {
    GETSET_ENTRY (WheelPairConfig, connection_point_offset),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot WheelPairConfig::Slots[] = {
    {Py_tp_new, (void *)&WheelPairConfig::New},
    {Py_tp_init, (void *)&WheelPairConfig::Init},
    {Py_tp_dealloc, (void *)&WheelPairConfig::Dealloc},
    {Py_tp_members, &WheelPairConfig::Members},
    {Py_tp_methods, &WheelPairConfig::Methods},
    {Py_tp_getset, &WheelPairConfig::GetSet},
    {0, nullptr},
};

PyType_Spec WheelPairConfig::Spec = {
    .name      = "RocketSim.WheelPairConfig",
    .basicsize = sizeof (WheelPairConfig),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = WheelPairConfig::Slots,
};

PyRef<WheelPairConfig> WheelPairConfig::NewFromWheelPairConfig (::WheelPairConfig const &config_) noexcept
{
	auto const self = PyRef<WheelPairConfig>::stealObject (WheelPairConfig::New (WheelPairConfig::Type, nullptr, nullptr));
	if (!self || !InitFromWheelPairConfig (self.borrow (), config_))
		return nullptr;

	return self;
}

bool WheelPairConfig::InitFromWheelPairConfig (WheelPairConfig *const self_, ::WheelPairConfig const &config_) noexcept
{
	auto connectionPointOffset = Vec::NewFromVec (config_.connectionPointOffset);
	if (!connectionPointOffset)
		return false;

	PyRef<Vec>::assign (self_->connectionPointOffset, connectionPointOffset.borrowObject ());

	self_->config = config_;

	return true;
}

PyObject *WheelPairConfig::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<WheelPairConfig>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->config)::WheelPairConfig ();

	self->connectionPointOffset = nullptr;

	return self.giftObject ();
}

int WheelPairConfig::Init (WheelPairConfig *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (!InitFromWheelPairConfig (self_, ::WheelPairConfig{}))
		return -1;

	return 0;
}

void WheelPairConfig::Dealloc (WheelPairConfig *self_) noexcept
{
	Py_XDECREF (self_->connectionPointOffset);

	self_->config.~WheelPairConfig ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *WheelPairConfig::Getconnection_point_offset (WheelPairConfig *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->connectionPointOffset).giftObject ();
}

int WheelPairConfig::Setconnection_point_offset (WheelPairConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_TypeError, "can't delete 'connection_point_offset' attribute of 'RocketSim.WheelPairConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->connectionPointOffset, value_);

	return 0;
}
}
