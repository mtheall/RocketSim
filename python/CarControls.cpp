#include "Module.h"

namespace RocketSim::Python
{
PyTypeObject *CarControls::Type = nullptr;

PyMemberDef CarControls::Members[] = {
    {.name      = "throttle",
        .type   = TypeHelper<decltype (::CarControls::throttle)>::type,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, throttle),
        .flags  = 0,
        .doc    = "Throttle"},
    {.name      = "steer",
        .type   = TypeHelper<decltype (::CarControls::steer)>::type,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, steer),
        .flags  = 0,
        .doc    = "Steer"},
    {.name      = "pitch",
        .type   = TypeHelper<decltype (::CarControls::pitch)>::type,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, pitch),
        .flags  = 0,
        .doc    = "Pitch"},
    {.name      = "yaw",
        .type   = TypeHelper<decltype (::CarControls::yaw)>::type,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, yaw),
        .flags  = 0,
        .doc    = "Yaw"},
    {.name      = "roll",
        .type   = TypeHelper<decltype (::CarControls::roll)>::type,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, roll),
        .flags  = 0,
        .doc    = "Roll"},
    {.name      = "boost",
        .type   = TypeHelper<decltype (::CarControls::boost)>::type,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, boost),
        .flags  = 0,
        .doc    = "Boost"},
    {.name      = "jump",
        .type   = TypeHelper<decltype (::CarControls::jump)>::type,
        .offset = offsetof (CarControls, controls) + offsetof (::CarControls, jump),
        .flags  = 0,
        .doc    = "Jump"},
    {.name      = "handbrake",
        .type   = TypeHelper<decltype (::CarControls::handbrake)>::type,
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
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&CarControls::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&CarControls::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.CarControls
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&CarControls::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.CarControls
Deep copy)"},
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
	static char useItemKwd[]   = "use_item"; // for future use

	static char *dict[] = {
	    throttleKwd, steerKwd, pitchKwd, yawKwd, rollKwd, boostKwd, jumpKwd, handbrakeKwd, useItemKwd, nullptr};

	::CarControls controls{};
	int boost     = controls.boost;
	int jump      = controls.jump;
	int handbrake = controls.handbrake;
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
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	::CarControls const model{};
	auto const controls = ToCarControls (self_);

	if (controls.throttle != model.throttle &&
	    !DictSetValue (dict.borrow (), "throttle", PyFloat_FromDouble (controls.throttle)))
		return nullptr;

	if (controls.steer != model.steer && !DictSetValue (dict.borrow (), "steer", PyFloat_FromDouble (controls.steer)))
		return nullptr;

	if (controls.pitch != model.pitch && !DictSetValue (dict.borrow (), "pitch", PyFloat_FromDouble (controls.pitch)))
		return nullptr;

	if (controls.yaw != model.yaw && !DictSetValue (dict.borrow (), "yaw", PyFloat_FromDouble (controls.yaw)))
		return nullptr;

	if (controls.roll != model.roll && !DictSetValue (dict.borrow (), "roll", PyFloat_FromDouble (controls.roll)))
		return nullptr;

	if (controls.boost != model.boost && !DictSetValue (dict.borrow (), "boost", PyBool_FromLong (controls.boost)))
		return nullptr;

	if (controls.jump != model.jump && !DictSetValue (dict.borrow (), "jump", PyBool_FromLong (controls.jump)))
		return nullptr;

	if (controls.handbrake != model.handbrake &&
	    !DictSetValue (dict.borrow (), "handbrake", PyBool_FromLong (controls.handbrake)))
		return nullptr;

	return dict.gift ();
}

PyObject *CarControls::Unpickle (CarControls *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *CarControls::Copy (CarControls *self_) noexcept
{
	return NewFromCarControls (self_->controls).giftObject ();
}

PyObject *CarControls::DeepCopy (CarControls *self_, PyObject *memo_) noexcept
{
	return NewFromCarControls (self_->controls).giftObject ();
}

PyObject *CarControls::ClampFix (CarControls *self_) noexcept
{
	self_->controls.ClampFix ();
	Py_RETURN_NONE;
}
}
