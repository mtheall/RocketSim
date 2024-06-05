#include "Module.h"

namespace
{
bool checkCustomBoostPads (PyObject *input_, bool setError_) noexcept
{
	if (!PySequence_Check (input_))
	{
		if (setError_)
			PyErr_SetString (PyExc_TypeError, "Invalid type for custom boost pads");

		return false;
	}

	auto const size = PySequence_Size (input_);

	for (int i = 0; i < size; ++i)
	{
		if (!Py_IS_TYPE (PySequence_GetItem (input_, i), RocketSim::Python::BoostPadConfig::Type))
		{
			if (setError_)
				PyErr_SetString (PyExc_TypeError, "Invalid type for custom boost pads");

			return false;
		}
	}

	return true;
}

RocketSim::Python::PyObjectRef convert (std::vector<RocketSim::BoostPadConfig> const &input_) noexcept
{
	using namespace RocketSim::Python;

	auto output = PyObjectRef::steal (PyList_New (input_.size ()));
	if (!output)
		return nullptr;

	for (unsigned i = 0; i < input_.size (); ++i)
	{
		auto config = BoostPadConfig::NewFromBoostPadConfig (input_[i]);
		if (!config)
			return nullptr;

		// steals ref
		if (PyList_SetItem (output.borrow (), i, config.giftObject ()) < 0)
			return nullptr;
	}

	return output;
}

std::vector<RocketSim::BoostPadConfig> convert (PyObject *input_) noexcept
{
	using namespace RocketSim::Python;

	std::vector<RocketSim::BoostPadConfig> output;

	output.resize (PySequence_Size (input_));

	for (unsigned i = 0; i < output.size (); ++i)
	{
		auto const config = PySequence_GetItem (input_, i);

		assert (Py_IS_TYPE (config, BoostPadConfig::Type));
		if (!Py_IS_TYPE (config, BoostPadConfig::Type))
			return {};

		output[i] = BoostPadConfig::ToBoostPadConfig (PyCast<BoostPadConfig> (config));
	}

	return output;
}
}

namespace RocketSim::Python
{
PyTypeObject *ArenaConfig::Type = nullptr;

PyMemberDef ArenaConfig::Members[] = {
    {.name      = "max_aabb_len",
        .type   = TypeHelper<decltype (RocketSim::ArenaConfig::maxAABBLen)>::type,
        .offset = offsetof (ArenaConfig, config) + offsetof (RocketSim::ArenaConfig, maxAABBLen),
        .flags  = 0,
        .doc    = "Maximum length of any object, calculated as the distance from AABB min to AABB max"},
    {.name      = "no_ball_rot",
        .type   = TypeHelper<decltype (RocketSim::ArenaConfig::noBallRot)>::type,
        .offset = offsetof (ArenaConfig, config) + offsetof (RocketSim::ArenaConfig, noBallRot),
        .flags  = 0,
        .doc    = "Ball rotation updates are skipped to improve performance"},
    {.name      = "use_custom_broadphase",
        .type   = TypeHelper<decltype (RocketSim::ArenaConfig::useCustomBroadphase)>::type,
        .offset = offsetof (ArenaConfig, config) + offsetof (RocketSim::ArenaConfig, useCustomBroadphase),
        .flags  = 0,
        .doc = "Use a custom broadphase designed for RocketSim. Improves performance, but becomes inefficient on giant "
               "maps. Turn this off if you want to use a giant map."},
    {.name      = "max_objects",
        .type   = TypeHelper<decltype (RocketSim::ArenaConfig::maxObjects)>::type,
        .offset = offsetof (ArenaConfig, config) + offsetof (RocketSim::ArenaConfig, maxObjects),
        .flags  = 0,
        .doc    = "Maximum number of objects"},
    {.name      = "custom_boost_pads",
        .type   = T_OBJECT,
        .offset = offsetof (ArenaConfig, customBoostPads),
        .flags  = 0,
        .doc    = "Custom boost pads"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef ArenaConfig::Methods[] = {
    {.ml_name     = "__getstate__",
        .ml_meth  = (PyCFunction)&ArenaConfig::Pickle,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&ArenaConfig::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&ArenaConfig::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.ArenaConfig
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&ArenaConfig::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.ArenaConfig
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef ArenaConfig::GetSet[] = {
    GETSET_ENTRY (ArenaConfig, memory_weight_mode, "Memory weight mode"),
    GETSET_ENTRY (ArenaConfig, min_pos, "Minimum object position"),
    GETSET_ENTRY (ArenaConfig, max_pos, "Maximum object position"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot ArenaConfig::Slots[] = {
    {Py_tp_new, (void *)&ArenaConfig::New},
    {Py_tp_init, (void *)&ArenaConfig::Init},
    {Py_tp_dealloc, (void *)&ArenaConfig::Dealloc},
    {Py_tp_members, &ArenaConfig::Members},
    {Py_tp_methods, &ArenaConfig::Methods},
    {Py_tp_getset, &ArenaConfig::GetSet},
    {Py_tp_doc, (void *)R"(Arena config
__init__(self,
	memory_weight_mode: int = RocketSim.MemoryWeightMode.HEAVY,
	min_pos: RocketSim.Vec = RocketSim.Vec(-4500.0, -6000.0, 0.0),
	max_pos: RocketSim.Vec = RocketSim.Vec(4500.0, 6000.0, 2500.0),
	max_aabb_len: float = 370.0,
	no_ball_rot: bool = True,
	use_custom_broadphase: bool = True,
	max_objects: int = 512),
	custom_boost_pads: Union[None, Sequence[RocketSim.BoostPadConfig]]))"},
    {0, nullptr},
};

PyType_Spec ArenaConfig::Spec = {
    .name      = "RocketSim.ArenaConfig",
    .basicsize = sizeof (ArenaConfig),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = ArenaConfig::Slots,
};

PyRef<ArenaConfig> ArenaConfig::NewFromArenaConfig (RocketSim::ArenaConfig const &config_) noexcept
{
	auto const self = PyRef<ArenaConfig>::stealObject (ArenaConfig::New (ArenaConfig::Type, nullptr, nullptr));
	if (!self || !InitFromArenaConfig (self.borrow (), config_))
		return nullptr;

	return self;
}

bool ArenaConfig::InitFromArenaConfig (ArenaConfig *self_, RocketSim::ArenaConfig const &config_) noexcept
{
	auto minPos          = Vec::NewFromVec (config_.minPos);
	auto maxPos          = Vec::NewFromVec (config_.maxPos);
	auto customBoostPads = PyObjectRef::incRef (Py_None);

	if (!minPos || !maxPos)
		return false;

	if (config_.useCustomBoostPads)
	{
		customBoostPads = convert (config_.customBoostPads);

		if (!customBoostPads)
			return false;
	}

	PyRef<Vec>::assign (self_->minPos, minPos.borrowObject ());
	PyRef<Vec>::assign (self_->maxPos, maxPos.borrowObject ());
	PyObjectRef::assign (self_->customBoostPads, customBoostPads.borrow ());

	self_->config = config_;

	return true;
}

std::optional<RocketSim::ArenaConfig> ArenaConfig::ToArenaConfig (ArenaConfig *self_) noexcept
{
	auto config = self_->config;

	config.minPos = Vec::ToVec (self_->minPos);
	config.maxPos = Vec::ToVec (self_->maxPos);

	config.useCustomBoostPads = false;

	if (self_->customBoostPads && !Py_IsNone (self_->customBoostPads))
	{
		try
		{
			if (!checkCustomBoostPads (self_->customBoostPads, true))
				return std::nullopt;

			config.useCustomBoostPads = true;
			config.customBoostPads    = convert (self_->customBoostPads);
		}
		catch (...)
		{
			PyErr_NoMemory ();
			return std::nullopt;
		}
	}

	return config;
}

PyObject *ArenaConfig::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<ArenaConfig>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	self->minPos          = nullptr;
	self->maxPos          = nullptr;
	self->customBoostPads = nullptr;

	return self.giftObject ();
}

int ArenaConfig::Init (ArenaConfig *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	RocketSim::ArenaConfig config{};

	PyObject *minPos              = nullptr; // borrowed references
	PyObject *maxPos              = nullptr;
	PyObject *noBallRot           = nullptr;
	PyObject *useCustomBroadphase = nullptr;
	PyObject *customBoostPads     = Py_None;
	int memoryWeightMode          = static_cast<int> (config.memWeightMode);

	static char memoryWeightModeKwd[]    = "memory_weight_mode";
	static char minPosKwd[]              = "min_pos";
	static char maxPosKwd[]              = "max_pos";
	static char maxAABBLenKwd[]          = "max_aabb_len";
	static char noBallRotKwd[]           = "no_ball_rot";
	static char useCustomBroadphaseKwd[] = "use_custom_broadphase";
	static char maxObjectsKwd[]          = "max_objects";
	static char customBoostPadsKwd[]     = "custom_boost_pads";

	static char *dict[] = {memoryWeightModeKwd,
	    minPosKwd,
	    maxPosKwd,
	    maxAABBLenKwd,
	    noBallRotKwd,
	    useCustomBroadphaseKwd,
	    maxObjectsKwd,
	    customBoostPadsKwd,
	    nullptr};

	if (!PyArg_ParseTupleAndKeywords (args_,
	        kwds_,
	        "|iO!O!fOOiOO",
	        dict,
	        &memoryWeightMode,
	        Vec::Type,
	        &minPos,
	        Vec::Type,
	        &maxPos,
	        &config.maxAABBLen,
	        &noBallRot,
	        &useCustomBroadphase,
	        &config.maxObjects,
	        &customBoostPads))
		return -1;

	config.memWeightMode = static_cast<RocketSim::ArenaMemWeightMode> (memoryWeightMode);

	switch (config.memWeightMode)
	{
	case RocketSim::ArenaMemWeightMode::LIGHT:
	case RocketSim::ArenaMemWeightMode::HEAVY:
		break;

	default:
		PyErr_Format (PyExc_ValueError, "Invalid arena memory weight mode '%d'", config.memWeightMode);
		return -1;
	}

	if (minPos)
		config.minPos = Vec::ToVec (PyCast<Vec> (minPos));

	if (maxPos)
		config.maxPos = Vec::ToVec (PyCast<Vec> (maxPos));

	if (noBallRot)
		config.noBallRot = PyObject_IsTrue (noBallRot);

	if (useCustomBroadphase)
		config.useCustomBroadphase = PyObject_IsTrue (useCustomBroadphase);

	if (!Py_IsNone (customBoostPads))
	{
		if (!checkCustomBoostPads (customBoostPads, true))
			return -1;

		config.customBoostPads = convert (customBoostPads);
	}

	if (!InitFromArenaConfig (self_, config))
		return -1;

	return 0;
}

void ArenaConfig::Dealloc (ArenaConfig *self_) noexcept
{
	Py_XDECREF (self_->minPos);
	Py_XDECREF (self_->maxPos);
	Py_XDECREF (self_->customBoostPads);

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *ArenaConfig::Pickle (ArenaConfig *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	auto model = RocketSim::ArenaConfig{};

	if (self_->config.memWeightMode != model.memWeightMode &&
	    !DictSetValue (
	        dict.borrow (), "memory_weight_mode", PyLong_FromLong (static_cast<long> (self_->config.memWeightMode))))
		return nullptr;

	if (Vec::ToVec (self_->minPos) != model.minPos &&
	    !DictSetValue (dict.borrow (), "min_pos", PyNewRef (self_->minPos)))
		return nullptr;

	if (Vec::ToVec (self_->maxPos) != model.maxPos &&
	    !DictSetValue (dict.borrow (), "max_pos", PyNewRef (self_->maxPos)))
		return nullptr;

	if (self_->customBoostPads && checkCustomBoostPads (self_->customBoostPads, false) &&
	    !DictSetValue (dict.borrow (), "custom_boost_pads", PyNewRef (self_->customBoostPads)))
		return nullptr;

	if (self_->config.maxAABBLen != model.maxAABBLen &&
	    !DictSetValue (dict.borrow (), "max_aabb_len", PyFloat_FromDouble (self_->config.maxAABBLen)))
		return nullptr;

	if (self_->config.noBallRot != model.noBallRot &&
	    !DictSetValue (dict.borrow (), "no_ball_rot", PyBool_FromLong (self_->config.noBallRot)))
		return nullptr;

	if (self_->config.useCustomBroadphase != model.useCustomBroadphase &&
	    !DictSetValue (dict.borrow (), "use_custom_broadphase", PyBool_FromLong (self_->config.useCustomBroadphase)))
		return nullptr;

	if (self_->config.maxObjects != model.maxObjects &&
	    !DictSetValue (dict.borrow (), "max_objects", PyLong_FromLong (self_->config.maxObjects)))
		return nullptr;

	return dict.gift ();
}

PyObject *ArenaConfig::Unpickle (ArenaConfig *self_, PyObject *dict_) noexcept
{
	if (!PyDict_Check (dict_))
	{
		PyErr_SetString (PyExc_ValueError, "Pickled object is not a dict");
		return nullptr;
	}

	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *ArenaConfig::Copy (ArenaConfig *self_) noexcept
{
	auto config = PyRef<ArenaConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->minPos, reinterpret_cast<PyObject *> (self_->minPos));
	PyRef<Vec>::assign (config->maxPos, reinterpret_cast<PyObject *> (self_->maxPos));
	PyObjectRef::assign (config->customBoostPads, self_->customBoostPads);

	auto result = ToArenaConfig (self_);
	if (!result.has_value ())
		return nullptr;

	config->config = result.value ();

	return config.giftObject ();
}

PyObject *ArenaConfig::DeepCopy (ArenaConfig *self_, PyObject *memo_) noexcept
{
	auto config = PyRef<ArenaConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->minPos, PyDeepCopy (self_->minPos, memo_));
	if (!config->minPos)
		return nullptr;

	PyRef<Vec>::assign (config->maxPos, PyDeepCopy (self_->maxPos, memo_));
	if (!config->maxPos)
		return nullptr;

	if (self_->customBoostPads && !Py_IsNone (self_->customBoostPads))
	{
		PyObjectRef::assign (config->customBoostPads, PyDeepCopy (self_->customBoostPads, memo_));
		if (!config->customBoostPads)
			return nullptr;
	}
	else
		PyObjectRef::assign (config->customBoostPads, Py_None);

	auto result = ToArenaConfig (self_);
	if (!result.has_value ())
		return nullptr;

	config->config = result.value ();

	return config.giftObject ();
}

PyObject *ArenaConfig::Getmemory_weight_mode (ArenaConfig *self_, void *) noexcept
{
	return PyLong_FromLong (static_cast<long> (self_->config.memWeightMode));
}

int ArenaConfig::Setmemory_weight_mode (ArenaConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_AttributeError, "can't delete 'memory_weight_mode' attribute of 'RocketSim.ArenaConfig' objects");
		return -1;
	}

	auto const value = PyObjectRef::steal (PyNumber_Long (reinterpret_cast<PyObject *> (self_)));
	if (!value)
		return -1;

	auto const mode = PyLong_AsUnsignedLong (value.borrow ());
	if (mode == static_cast<unsigned long> (-1) && PyErr_Occurred ())
		return -1;

	switch (static_cast<RocketSim::ArenaMemWeightMode> (mode))
	{
	case RocketSim::ArenaMemWeightMode::LIGHT:
	case RocketSim::ArenaMemWeightMode::HEAVY:
		self_->config.memWeightMode = static_cast<RocketSim::ArenaMemWeightMode> (mode);
		return 0;

	default:
		PyErr_Format (PyExc_ValueError, "Invalid arena memory weight mode '%lu'", mode);
		return -1;
	}
}

PyObject *ArenaConfig::Getmin_pos (ArenaConfig *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->minPos).giftObject ();
}

int ArenaConfig::Setmin_pos (ArenaConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_AttributeError, "can't delete 'min_pos' attribute of 'RocketSim.ArenaConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	if (value_ == (PyObject *)self_->minPos)
		return 0;

	PyRef<Vec>::assign (self_->minPos, value_);

	return 0;
}

PyObject *ArenaConfig::Getmax_pos (ArenaConfig *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->minPos).giftObject ();
}

int ArenaConfig::Setmax_pos (ArenaConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_AttributeError, "can't delete 'max_pos' attribute of 'RocketSim.ArenaConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	if (value_ == (PyObject *)self_->maxPos)
		return 0;

	PyRef<Vec>::assign (self_->maxPos, value_);

	return 0;
}
}
