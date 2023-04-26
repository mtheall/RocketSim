#include "Module.h"

#include <cstddef>
#include <cstring>
#include <limits>

namespace
{
unsigned templateDiff (::CarConfig const &config_, ::CarConfig const &model_) noexcept
{
	unsigned diff = 0;

	if (config_.hitboxSize != model_.hitboxSize)
		diff += 3;

	if (config_.hitboxPosOffset != model_.hitboxPosOffset)
		diff += 3;

	if (config_.frontWheels.wheelRadius != model_.frontWheels.wheelRadius)
		diff += 1;

	if (config_.frontWheels.suspensionRestLength != model_.frontWheels.suspensionRestLength)
		diff += 1;

	if (config_.frontWheels.connectionPointOffset != model_.frontWheels.connectionPointOffset)
		diff += 3;

	if (config_.backWheels.wheelRadius != model_.backWheels.wheelRadius)
		diff += 1;

	if (config_.backWheels.suspensionRestLength != model_.backWheels.suspensionRestLength)
		diff += 1;

	if (config_.backWheels.connectionPointOffset != model_.backWheels.connectionPointOffset)
		diff += 3;

	if (config_.dodgeDeadzone != model_.dodgeDeadzone)
		diff += 1;

	return diff;
}

RocketSim::Python::CarConfig::Index bestTemplateConfig (::CarConfig const &config_) noexcept
{
	using RocketSim::Python::CarConfig;

	CarConfig::Index best = CarConfig::Index::OCTANE;
	unsigned bestDiff     = std::numeric_limits<unsigned>::max ();

	for (auto const &[index, model] : {
	         // clang-format off
	         std::make_pair (CarConfig::Index::OCTANE, &CAR_CONFIG_OCTANE),
	         std::make_pair (CarConfig::Index::DOMINUS, &CAR_CONFIG_DOMINUS),
	         std::make_pair (CarConfig::Index::PLANK, &CAR_CONFIG_PLANK),
	         std::make_pair (CarConfig::Index::BREAKOUT, &CAR_CONFIG_BREAKOUT),
	         std::make_pair (CarConfig::Index::HYBRID, &CAR_CONFIG_HYBRID),
	         std::make_pair (CarConfig::Index::MERC, &CAR_CONFIG_MERC)
	         // clang-format on
	     })
	{
		auto const diff = templateDiff (config_, *model);
		if (diff == 0)
			return index;

		if (diff < bestDiff)
		{
			best     = index;
			bestDiff = diff;
		}
	}

	return best;
}
}

namespace RocketSim::Python
{
PyTypeObject *CarConfig::Type = nullptr;

PyMemberDef CarConfig::Members[] = {
    {.name      = "dodge_deadzone",
        .type   = TypeHelper<decltype (::CarConfig::dodgeDeadzone)>::type,
        .offset = offsetof (CarConfig, config) + offsetof (::CarConfig, dodgeDeadzone),
        .flags  = 0,
        .doc    = "Dodge deadzone"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef CarConfig::Methods[] = {
    {.ml_name = "__getstate__", .ml_meth = (PyCFunction)&CarConfig::Pickle, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&CarConfig::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&CarConfig::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.CarConfig
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&CarConfig::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.CarConfig
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef CarConfig::GetSet[] = {
    GETSET_ENTRY (CarConfig, hitbox_size, "Hitbox size"),
    GETSET_ENTRY (CarConfig, hitbox_pos_offset, "Hitbox position offset"),
    GETSET_ENTRY (CarConfig, front_wheels, "Front wheels"),
    GETSET_ENTRY (CarConfig, back_wheels, "Back wheels"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot CarConfig::Slots[] = {
    {Py_tp_new, (void *)&CarConfig::New},
    {Py_tp_init, (void *)&CarConfig::Init},
    {Py_tp_dealloc, (void *)&CarConfig::Dealloc},
    {Py_tp_members, &CarConfig::Members},
    {Py_tp_methods, &CarConfig::Methods},
    {Py_tp_getset, &CarConfig::GetSet},
    {Py_tp_doc, (void *)R"(Car config
__init__(self,
	template: int = RocketSim.CarConfig.OCTANE,
	hitbox_size: RocketSim.Vec,
	hitbox_pos_offset: RocketSim.Vec,
	front_wheels: RocketSim.WheelPairConfig,
	back_wheels: RocketSim.WheelPairConfig,
	dodge_deadzone: float))"},
    {0, nullptr},
};

PyType_Spec CarConfig::Spec = {
    .name      = "RocketSim.CarConfig",
    .basicsize = sizeof (CarConfig),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = CarConfig::Slots,
};

bool CarConfig::FromIndex (Index index_, ::CarConfig &config_) noexcept
{
	switch (index_)
	{
	case Index::OCTANE:
		config_ = CAR_CONFIG_OCTANE;
		return true;

	case Index::DOMINUS:
		config_ = CAR_CONFIG_DOMINUS;
		return true;

	case Index::PLANK:
		config_ = CAR_CONFIG_PLANK;
		return true;

	case Index::BREAKOUT:
		config_ = CAR_CONFIG_BREAKOUT;
		return true;

	case Index::HYBRID:
		config_ = CAR_CONFIG_HYBRID;
		return true;

	case Index::MERC:
		config_ = CAR_CONFIG_MERC;
		return true;
	}

	PyErr_SetString (PyExc_RuntimeError, "Invalid car configuration");
	return false;
}

PyRef<CarConfig> CarConfig::NewFromCarConfig (::CarConfig const &config_) noexcept
{
	auto const self = PyRef<CarConfig>::stealObject (CarConfig::New (CarConfig::Type, nullptr, nullptr));
	if (!self || !InitFromCarConfig (self.borrow (), config_))
		return nullptr;

	return self;
}

bool CarConfig::InitFromCarConfig (CarConfig *const self_, ::CarConfig const &config_) noexcept
{
	auto hitboxSize      = Vec::NewFromVec (config_.hitboxSize);
	auto hitboxPosOffset = Vec::NewFromVec (config_.hitboxPosOffset);
	auto frontWheels     = WheelPairConfig::NewFromWheelPairConfig (config_.frontWheels);
	auto backWheels      = WheelPairConfig::NewFromWheelPairConfig (config_.backWheels);

	if (!hitboxSize || !hitboxPosOffset || !frontWheels || !backWheels)
		return false;

	PyRef<Vec>::assign (self_->hitboxSize, hitboxSize.borrowObject ());
	PyRef<Vec>::assign (self_->hitboxPosOffset, hitboxPosOffset.borrowObject ());
	PyRef<WheelPairConfig>::assign (self_->frontWheels, frontWheels.borrowObject ());
	PyRef<WheelPairConfig>::assign (self_->backWheels, backWheels.borrowObject ());

	self_->config = config_;

	return true;
}

::CarConfig CarConfig::ToCarConfig (CarConfig *self_) noexcept
{
	auto config = self_->config;

	config.hitboxSize      = Vec::ToVec (self_->hitboxSize);
	config.hitboxPosOffset = Vec::ToVec (self_->hitboxPosOffset);
	config.frontWheels     = WheelPairConfig::ToWheelPairConfig (self_->frontWheels);
	config.backWheels      = WheelPairConfig::ToWheelPairConfig (self_->backWheels);

	return config;
}

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
	static char templateKwd[]        = "template";
	static char hitboxSizeKwd[]      = "hitbox_size";
	static char hitboxPosOffsetKwd[] = "hitbox_pos_offset";
	static char frontWheelsKwd[]     = "front_wheels";
	static char backWheelsKwd[]      = "back_wheels";
	static char dodgeDeadzoneKwd[]   = "dodge_deadzone";

	static char *dict[] = {
	    templateKwd, hitboxSizeKwd, hitboxPosOffsetKwd, frontWheelsKwd, backWheelsKwd, dodgeDeadzoneKwd, nullptr};

	PyObject *hitboxSize      = nullptr; // borrowed references
	PyObject *hitboxPosOffset = nullptr;
	PyObject *frontWheels     = nullptr;
	PyObject *backWheels      = nullptr;
	float dodgeDeadzone       = 0.5f;
	int templateId            = 0;
	if (!PyArg_ParseTupleAndKeywords (args_,
	        kwds_,
	        "|iO!O!O!O!f",
	        dict,
	        &templateId,
	        Vec::Type,
	        &hitboxSize,
	        Vec::Type,
	        &hitboxPosOffset,
	        WheelPairConfig::Type,
	        &frontWheels,
	        WheelPairConfig::Type,
	        &backWheels,
	        &dodgeDeadzone))
		return -1;

	// initialize with template
	::CarConfig config{};
	if (!FromIndex (static_cast<Index> (templateId), config))
		return -1;

	if (hitboxSize)
		config.hitboxSize = Vec::ToVec (PyCast<Vec> (hitboxSize));

	if (hitboxPosOffset)
		config.hitboxPosOffset = Vec::ToVec (PyCast<Vec> (hitboxPosOffset));

	if (frontWheels)
		config.frontWheels = WheelPairConfig::ToWheelPairConfig (PyCast<WheelPairConfig> (frontWheels));

	if (backWheels)
		config.backWheels = WheelPairConfig::ToWheelPairConfig (PyCast<WheelPairConfig> (backWheels));

	config.dodgeDeadzone = dodgeDeadzone;

	if (!InitFromCarConfig (self_, config))
		return -1;

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

PyObject *CarConfig::Pickle (CarConfig *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	auto const index = bestTemplateConfig (self_->config);
	if (!DictSetValue (dict.borrow (), "template", PyLong_FromLong (static_cast<long> (index))))
		return nullptr;

	::CarConfig model{};
	if (!FromIndex (index, model))
		return nullptr;

	auto const config = ToCarConfig (self_);
	if (config.hitboxSize != model.hitboxSize &&
	    !DictSetValue (dict.borrow (), "hitbox_size", PyNewRef (self_->hitboxSize)))
		return nullptr;

	if (config.hitboxPosOffset != model.hitboxPosOffset &&
	    !DictSetValue (dict.borrow (), "hitbox_pos_offset", PyNewRef (self_->hitboxPosOffset)))
		return nullptr;

	if ((config.frontWheels.wheelRadius != model.frontWheels.wheelRadius ||
	        config.frontWheels.suspensionRestLength != model.frontWheels.suspensionRestLength ||
	        config.frontWheels.connectionPointOffset != model.frontWheels.connectionPointOffset) &&
	    !DictSetValue (dict.borrow (), "front_wheels", PyNewRef (self_->frontWheels)))
		return nullptr;

	if ((config.backWheels.wheelRadius != model.backWheels.wheelRadius ||
	        config.backWheels.suspensionRestLength != model.backWheels.suspensionRestLength ||
	        config.backWheels.connectionPointOffset != model.backWheels.connectionPointOffset) &&
	    !DictSetValue (dict.borrow (), "back_wheels", PyNewRef (self_->backWheels)))
		return nullptr;

	if (config.dodgeDeadzone != model.dodgeDeadzone &&
	    !DictSetValue (dict.borrow (), "dodge_deadzone", PyFloat_FromDouble (config.dodgeDeadzone)))
		return nullptr;

	return dict.gift ();
}

PyObject *CarConfig::Unpickle (CarConfig *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *CarConfig::Copy (CarConfig *self_) noexcept
{
	auto config = PyRef<CarConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->hitboxSize, reinterpret_cast<PyObject *> (self_->hitboxSize));
	PyRef<Vec>::assign (config->hitboxPosOffset, reinterpret_cast<PyObject *> (self_->hitboxPosOffset));
	PyRef<WheelPairConfig>::assign (config->frontWheels, reinterpret_cast<PyObject *> (self_->frontWheels));
	PyRef<WheelPairConfig>::assign (config->backWheels, reinterpret_cast<PyObject *> (self_->backWheels));

	config->config = ToCarConfig (self_);

	return config.giftObject ();
}

PyObject *CarConfig::DeepCopy (CarConfig *self_, PyObject *memo_) noexcept
{
	auto config = PyRef<CarConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->hitboxSize, PyDeepCopy (self_->hitboxSize, memo_));
	if (!config->hitboxSize)
		return nullptr;

	PyRef<Vec>::assign (config->hitboxPosOffset, PyDeepCopy (self_->hitboxPosOffset, memo_));
	if (!config->hitboxPosOffset)
		return nullptr;

	PyRef<WheelPairConfig>::assign (config->frontWheels, PyDeepCopy (self_->frontWheels, memo_));
	if (!config->frontWheels)
		return nullptr;

	PyRef<WheelPairConfig>::assign (config->backWheels, PyDeepCopy (self_->backWheels, memo_));
	if (!config->backWheels)
		return nullptr;

	config->config = ToCarConfig (self_);

	return config.giftObject ();
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
