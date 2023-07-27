#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *WheelPairConfig::Type = nullptr;

PyMemberDef WheelPairConfig::Members[] = {
    {.name      = "wheel_radius",
        .type   = TypeHelper<decltype (::WheelPairConfig::wheelRadius)>::type,
        .offset = offsetof (WheelPairConfig, config) + offsetof (::WheelPairConfig, wheelRadius),
        .flags  = 0,
        .doc    = "Wheel radius"},
    {.name      = "suspension_rest_length",
        .type   = TypeHelper<decltype (::WheelPairConfig::suspensionRestLength)>::type,
        .offset = offsetof (WheelPairConfig, config) + offsetof (::WheelPairConfig, suspensionRestLength),
        .flags  = 0,
        .doc    = "Suspension rest length"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef WheelPairConfig::Methods[] = {
    {.ml_name     = "__getstate__",
        .ml_meth  = (PyCFunction)&WheelPairConfig::Pickle,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "__setstate__",
        .ml_meth  = (PyCFunction)&WheelPairConfig::Unpickle,
        .ml_flags = METH_O,
        .ml_doc   = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&WheelPairConfig::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.WheelPairConfig
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&WheelPairConfig::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.WheelPairConfig
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef WheelPairConfig::GetSet[] = {
    GETSET_ENTRY (WheelPairConfig, connection_point_offset, "Connection point offset"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot WheelPairConfig::Slots[] = {
    {Py_tp_new, (void *)&WheelPairConfig::New},
    {Py_tp_init, (void *)&WheelPairConfig::Init},
    {Py_tp_dealloc, (void *)&WheelPairConfig::Dealloc},
    {Py_tp_members, &WheelPairConfig::Members},
    {Py_tp_methods, &WheelPairConfig::Methods},
    {Py_tp_getset, &WheelPairConfig::GetSet},
    {Py_tp_doc, (void *)R"(Wheel pair config
__init__(self,
	wheel_radius: float = 0.0,
	suspension_rest_length: float = 0.0,
	connection_point_offset: RocketSim.Vec = RocketSim.Vec()))"},
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
	auto const self =
	    PyRef<WheelPairConfig>::stealObject (WheelPairConfig::New (WheelPairConfig::Type, nullptr, nullptr));
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

::WheelPairConfig WheelPairConfig::ToWheelPairConfig (WheelPairConfig *self_) noexcept
{
	auto config = self_->config;

	config.connectionPointOffset = Vec::ToVec (self_->connectionPointOffset);

	return config;
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
	static char wheelRadiusKwd[]           = "wheel_radius";
	static char suspensionRestLengthKwd[]  = "suspension_rest_length";
	static char connectionPointOffsetKwd[] = "connection_point_offset";

	static char *dict[] = {wheelRadiusKwd, suspensionRestLengthKwd, connectionPointOffsetKwd, nullptr};

	::WheelPairConfig config{};

	PyObject *connectionPointOffset = nullptr; // borrowed
	if (!PyArg_ParseTupleAndKeywords (args_,
	        kwds_,
	        "|ffO!",
	        dict,
	        &config.wheelRadius,
	        &config.suspensionRestLength,
	        Vec::Type,
	        &connectionPointOffset))
		return -1;

	if (connectionPointOffset)
		config.connectionPointOffset = Vec::ToVec (PyCast<Vec> (connectionPointOffset));

	if (!InitFromWheelPairConfig (self_, config))
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

PyObject *WheelPairConfig::Pickle (WheelPairConfig *self_) noexcept
{
	return Py_BuildValue ("{sfsfsO}",
	    "wheel_radius",
	    self_->config.wheelRadius,
	    "suspension_rest_length",
	    self_->config.suspensionRestLength,
	    "connection_point_offset",
	    self_->connectionPointOffset);
}

PyObject *WheelPairConfig::Unpickle (WheelPairConfig *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *WheelPairConfig::Copy (WheelPairConfig *self_) noexcept
{
	auto config = PyRef<WheelPairConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->connectionPointOffset, reinterpret_cast<PyObject *> (self_->connectionPointOffset));

	config->config = ToWheelPairConfig (self_);

	return config.giftObject ();
}

PyObject *WheelPairConfig::DeepCopy (WheelPairConfig *self_, PyObject *memo_) noexcept
{
	auto config = PyRef<WheelPairConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->connectionPointOffset, PyDeepCopy (self_->connectionPointOffset, memo_));
	if (!config->connectionPointOffset)
		return nullptr;

	config->config = ToWheelPairConfig (self_);

	return config.giftObject ();
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
