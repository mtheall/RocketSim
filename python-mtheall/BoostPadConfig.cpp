#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *BoostPadConfig::Type = nullptr;

PyMemberDef BoostPadConfig::Members[] = {
    {.name      = "is_big",
        .type   = TypeHelper<decltype (RocketSim::BoostPadConfig::isBig)>::type,
        .offset = offsetof (BoostPadConfig, config) + offsetof (RocketSim::BoostPadConfig, isBig),
        .flags  = 0,
        .doc    = "Whether boost pad is big"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef BoostPadConfig::Methods[] = {
    {.ml_name     = "__getstate__",
        .ml_meth  = (PyCFunction)&BoostPadConfig::Pickle,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "__setstate__",
        .ml_meth  = (PyCFunction)&BoostPadConfig::Unpickle,
        .ml_flags = METH_O,
        .ml_doc   = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&BoostPadConfig::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.BoostPadConfig
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&BoostPadConfig::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.BoostPadConfig
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef BoostPadConfig::GetSet[] = {
    GETSET_ENTRY (BoostPadConfig, pos, "Position"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot BoostPadConfig::Slots[] = {
    {Py_tp_new, (void *)(&BoostPadConfig::New)},
    {Py_tp_init, (void *)(&BoostPadConfig::Init)},
    {Py_tp_dealloc, (void *)(&BoostPadConfig::Dealloc)},
    {Py_tp_members, &BoostPadConfig::Members},
    {Py_tp_methods, &BoostPadConfig::Methods},
    {Py_tp_getset, &BoostPadConfig::GetSet},
    {Py_tp_doc, (void *)R"(Boost pad config
__init__(self,
	pos: RocketSim.Vec = RocketSim.Vec(),
	is_big: bool = False))"},
    {0, nullptr},
};

PyType_Spec BoostPadConfig::Spec = {
    .name      = "RocketSim.BoostPadConfig",
    .basicsize = sizeof (BoostPadConfig),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BoostPadConfig::Slots,
};

PyRef<BoostPadConfig> BoostPadConfig::NewFromBoostPadConfig (RocketSim::BoostPadConfig const &pad_) noexcept
{
	auto const self = PyRef<BoostPadConfig>::stealObject (BoostPadConfig::New (BoostPadConfig::Type, nullptr, nullptr));
	if (!self || !InitFromBoostPadConfig (self.borrow (), pad_))
		return nullptr;

	return self;
}

bool BoostPadConfig::InitFromBoostPadConfig (BoostPadConfig *const self_,
    RocketSim::BoostPadConfig const &config_) noexcept
{
	auto pos = Vec::NewFromVec (config_.pos);

	if (!pos)
		return false;

	PyRef<Vec>::assign (self_->pos, pos.borrowObject ());

	self_->config = config_;

	return true;
}

RocketSim::BoostPadConfig BoostPadConfig::ToBoostPadConfig (BoostPadConfig *self_) noexcept
{
	auto config = self_->config;

	config.pos = Vec::ToVec (self_->pos);

	return config;
}

PyObject *BoostPadConfig::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<BoostPadConfig>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->config) RocketSim::BoostPadConfig{};

	self->pos = nullptr;

	return self.giftObject ();
}

int BoostPadConfig::Init (BoostPadConfig *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char posKwd[]   = "pos";
	static char isBigKwd[] = "is_big";

	static char *dict[] = {posKwd, isBigKwd, nullptr};

	RocketSim::BoostPadConfig config{};

	PyObject *pos   = nullptr; // borrowed references
	PyObject *isBig = nullptr;

	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|O!O", dict, Vec::Type, &pos, &isBig))
		return -1;

	if (pos)
		config.pos = Vec::ToVec (PyCast<Vec> (pos));
	if (isBig)
		config.isBig = PyObject_IsTrue (isBig);

	if (!InitFromBoostPadConfig (self_, config))
		return -1;

	return 0;
}

void BoostPadConfig::Dealloc (BoostPadConfig *self_) noexcept
{
	Py_XDECREF (self_->pos);

	self_->config.~BoostPadConfig ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *BoostPadConfig::Pickle (BoostPadConfig *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	RocketSim::BoostPadConfig const model{};
	auto const config = ToBoostPadConfig (self_);

	if (Vec::ToVec (self_->pos) != model.pos && !DictSetValue (dict.borrow (), "pos", PyNewRef (self_->pos)))
		return nullptr;

	if (config.isBig != model.isBig && !DictSetValue (dict.borrow (), "is_big", PyBool_FromLong (config.isBig)))
		return nullptr;

	return dict.gift ();
}

PyObject *BoostPadConfig::Unpickle (BoostPadConfig *self_, PyObject *dict_) noexcept
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

PyObject *BoostPadConfig::Copy (BoostPadConfig *self_) noexcept
{
	auto config = PyRef<BoostPadConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->pos, reinterpret_cast<PyObject *> (self_->pos));

	config->config = ToBoostPadConfig (self_);

	return config.giftObject ();
}

PyObject *BoostPadConfig::DeepCopy (BoostPadConfig *self_, PyObject *memo_) noexcept
{
	auto config = PyRef<BoostPadConfig>::stealObject (New (Type, nullptr, nullptr));
	if (!config)
		return nullptr;

	PyRef<Vec>::assign (config->pos, PyDeepCopy (self_->pos, memo_));
	if (!config->pos)
		return nullptr;

	config->config = ToBoostPadConfig (self_);

	return config.giftObject ();
}

PyObject *BoostPadConfig::Getpos (BoostPadConfig *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->pos).giftObject ();
}

int BoostPadConfig::Setpos (BoostPadConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_AttributeError, "can't delete 'pos' attribute of 'RocketSim.BoostPadConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	if (value_ == (PyObject *)self_->pos)
		return 0;

	PyRef<Vec>::assign (self_->pos, value_);

	return 0;
}
}
