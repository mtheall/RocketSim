#include "Module.h"

namespace RocketSim::Python
{
PyTypeObject *CarControls::Type = nullptr;

PyMemberDef CarControls::Members[] = {
    {.name      = "throttle",
        .type   = T_FLOAT,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, throttle),
        .flags  = 0,
        .doc    = "Throttle"},
    {.name      = "steer",
        .type   = T_FLOAT,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, steer),
        .flags  = 0,
        .doc    = "Steer"},
    {.name      = "pitch",
        .type   = T_FLOAT,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, pitch),
        .flags  = 0,
        .doc    = "Pitch"},
    {.name      = "yaw",
        .type   = T_FLOAT,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, yaw),
        .flags  = 0,
        .doc    = "Yaw"},
    {.name      = "roll",
        .type   = T_FLOAT,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, roll),
        .flags  = 0,
        .doc    = "Roll"},
    {.name      = "boost",
        .type   = T_BOOL,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, boost),
        .flags  = 0,
        .doc    = "Boost"},
    {.name      = "jump",
        .type   = T_BOOL,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, jump),
        .flags  = 0,
        .doc    = "Jump"},
    {.name      = "handbrake",
        .type   = T_BOOL,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, handbrake),
        .flags  = 0,
        .doc    = "Handbrake"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef CarControls::Methods[] = {
    {.ml_name     = "clamp_fix",
        .ml_meth  = (PyCFunction)&CarControls::ClampFix,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyType_Slot CarControls::Slots[] = {
    {Py_tp_new, (void *)&CarControls::New},
    {Py_tp_init, (void *)&CarControls::Init},
    {Py_tp_dealloc, (void *)&CarControls::Dealloc},
    {Py_tp_members, &CarControls::Members},
    {Py_tp_methods, &CarControls::Methods},
    {0, nullptr},
};

PyType_Spec CarControls::Spec = {
    .name      = "RocketSim.CarControls",
    .basicsize = sizeof (CarControls),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = CarControls::Slots,
};

PyRef<CarControls> CarControls::NewFromCarControls (::CarControls const &controls_) noexcept
{
	auto const self = PyRef<CarControls>::stealObject (CarControls::New (CarControls::Type, nullptr, nullptr));
	if (!self || !InitFromCarControls (self.borrow (), controls_))
		return nullptr;

	return self;
}

bool CarControls::InitFromCarControls (CarControls *const self_, ::CarControls const &controls_) noexcept
{
	self_->controls = controls_;
	return true;
}

PyObject *CarControls::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<CarControls>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->controls)::CarControls{};

	return self.giftObject ();
}

int CarControls::Init (CarControls *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (!InitFromCarControls (self_, ::CarControls{}))
		return -1;

	return 0;
}

void CarControls::Dealloc (CarControls *self_) noexcept
{
	self_->controls.~CarControls ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *CarControls::ClampFix (CarControls *self_, PyObject *args_) noexcept
{
	self_->controls.ClampFix ();
	Py_RETURN_NONE;
}
}
