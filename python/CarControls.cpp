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
    {.ml_name     = "__getstate__",
        .ml_meth  = (PyCFunction)&CarControls::Pickle,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)CarControls::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
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

::CarControls CarControls::ToCarControls (CarControls *self_) noexcept
{
	return self_->controls;
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
	static char throttleKwd[]  = "throttle";
	static char steerKwd[]     = "steer";
	static char pitchKwd[]     = "pitch";
	static char yawKwd[]       = "yaw";
	static char rollKwd[]      = "roll";
	static char boostKwd[]     = "boost";
	static char jumpKwd[]      = "jump";
	static char handbrakeKwd[] = "handbrake";
	static char useItemKwd[]   = "use_item";

	static char *dict[] = {
	    throttleKwd, steerKwd, pitchKwd, yawKwd, rollKwd, boostKwd, jumpKwd, handbrakeKwd, useItemKwd, nullptr};

	::CarControls controls{};
	int boost     = false;
	int jump      = false;
	int handbrake = false;
	int useItem   = false;
	if (!PyArg_ParseTupleAndKeywords (args_,
	        kwds_,
	        "|fffffpppp",
	        dict,
	        &controls.throttle,
	        &controls.steer,
	        &controls.pitch,
	        &controls.yaw,
	        &controls.roll,
	        &boost,
	        &jump,
	        &handbrake,
	        &useItem))
		return -1;

	controls.boost     = boost;
	controls.jump      = jump;
	controls.handbrake = handbrake;

	if (!InitFromCarControls (self_, controls))
		return -1;

	return 0;
}

void CarControls::Dealloc (CarControls *self_) noexcept
{
	self_->controls.~CarControls ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *CarControls::Pickle (CarControls *self_) noexcept
{
	return Py_BuildValue ("{sfsfsfsfsfsOsOsOsO}",
	    "throttle",
	    self_->controls.throttle,
	    "steer",
	    self_->controls.steer,
	    "pitch",
	    self_->controls.pitch,
	    "yaw",
	    self_->controls.yaw,
	    "roll",
	    self_->controls.roll,
	    "boost",
	    PyBool_FromLong (self_->controls.boost),
	    "jump",
	    PyBool_FromLong (self_->controls.jump),
	    "handbrake",
	    PyBool_FromLong (self_->controls.handbrake),
	    "use_item",
	    Py_False);
}

PyObject *CarControls::Unpickle (CarControls *self_, PyObject *dict_) noexcept
{
	if (!Py_IS_TYPE (dict_, &PyDict_Type))
	{
		PyErr_SetString (PyExc_ValueError, "Pickled object is not a dict.");
		return nullptr;
	}

	auto const throttle  = GetItem (dict_, "throttle");
	auto const steer     = GetItem (dict_, "steer");
	auto const pitch     = GetItem (dict_, "pitch");
	auto const yaw       = GetItem (dict_, "yaw");
	auto const roll      = GetItem (dict_, "roll");
	auto const boost     = GetItem (dict_, "boost");
	auto const jump      = GetItem (dict_, "jump");
	auto const handbrake = GetItem (dict_, "handbrake");
	auto const useItem   = GetItem (dict_, "use_item");

	if ((throttle && !Py_IS_TYPE (throttle.borrow (), &PyFloat_Type)) ||
	    (steer && !Py_IS_TYPE (steer.borrow (), &PyFloat_Type)) ||
	    (pitch && !Py_IS_TYPE (pitch.borrow (), &PyFloat_Type)) ||
	    (yaw && !Py_IS_TYPE (yaw.borrow (), &PyFloat_Type)) || (roll && !Py_IS_TYPE (roll.borrow (), &PyFloat_Type)) ||
	    (boost && !Py_IS_TYPE (boost.borrow (), &PyBool_Type)) ||
	    (jump && !Py_IS_TYPE (jump.borrow (), &PyBool_Type)) ||
	    (handbrake && !Py_IS_TYPE (handbrake.borrow (), &PyBool_Type)) ||
	    (useItem && !Py_IS_TYPE (useItem.borrow (), &PyBool_Type)))
	{
		PyErr_SetString (PyExc_ValueError, "Pickled object is invalid.");
		return nullptr;
	}

	self_->controls.throttle  = static_cast<float> (PyFloat_AsDouble (throttle.borrow ()));
	self_->controls.steer     = static_cast<float> (PyFloat_AsDouble (steer.borrow ()));
	self_->controls.pitch     = static_cast<float> (PyFloat_AsDouble (pitch.borrow ()));
	self_->controls.yaw       = static_cast<float> (PyFloat_AsDouble (yaw.borrow ()));
	self_->controls.roll      = static_cast<float> (PyFloat_AsDouble (roll.borrow ()));
	self_->controls.boost     = (boost.borrow () == Py_True);
	self_->controls.jump      = (jump.borrow () == Py_True);
	self_->controls.handbrake = (handbrake.borrow () == Py_True);

	Py_RETURN_NONE;
}

PyObject *CarControls::ClampFix (CarControls *self_, PyObject *args_) noexcept
{
	self_->controls.ClampFix ();
	Py_RETURN_NONE;
}
}
