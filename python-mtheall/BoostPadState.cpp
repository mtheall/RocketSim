#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *BoostPadState::Type = nullptr;

PyMemberDef BoostPadState::Members[] = {
    {.name      = "is_active",
        .type   = TypeHelper<decltype (::BoostPadState::isActive)>::type,
        .offset = offsetof (BoostPadState, state) + offsetof (::BoostPadState, isActive),
        .flags  = 0,
        .doc    = "Is active"},
    {.name      = "cooldown",
        .type   = TypeHelper<decltype (::BoostPadState::cooldown)>::type,
        .offset = offsetof (BoostPadState, state) + offsetof (::BoostPadState, cooldown),
        .flags  = 0,
        .doc    = "Cooldown"},
    {.name      = "prev_locked_car_id",
        .type   = TypeHelper<decltype (::BoostPadState::prevLockedCarID)>::type,
        .offset = offsetof (BoostPadState, state) + offsetof (::BoostPadState, prevLockedCarID),
        .flags  = 0,
        .doc    = "Prev locked car id"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef BoostPadState::Methods[] = {
    {.ml_name     = "__getstate__",
        .ml_meth  = (PyCFunction)&BoostPadState::Pickle,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name     = "__setstate__",
        .ml_meth  = (PyCFunction)&BoostPadState::Unpickle,
        .ml_flags = METH_O,
        .ml_doc   = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&BoostPadState::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.BoostPadState
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&BoostPadState::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.BoostPadState
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyType_Slot BoostPadState::Slots[] = {
    {Py_tp_new, (void *)&BoostPadState::New},
    {Py_tp_init, (void *)&BoostPadState::Init},
    {Py_tp_dealloc, (void *)&BoostPadState::Dealloc},
    {Py_tp_members, &BoostPadState::Members},
    {Py_tp_methods, &BoostPadState::Methods},
    {0, nullptr},
};

PyType_Spec BoostPadState::Spec = {
    .name      = "RocketSim.BoostPadState",
    .basicsize = sizeof (BoostPadState),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BoostPadState::Slots,
};

PyRef<BoostPadState> BoostPadState::NewFromBoostPadState (::BoostPadState const &state_) noexcept
{
	auto const self = PyRef<BoostPadState>::stealObject (BoostPadState::New (BoostPadState::Type, nullptr, nullptr));
	if (!self || !InitFromBoostPadState (self.borrow (), state_))
		return nullptr;

	return self;
}

bool BoostPadState::InitFromBoostPadState (BoostPadState *self_, ::BoostPadState const &state_) noexcept
{
	self_->state = state_;
	return true;
}

::BoostPadState BoostPadState::ToBoostPadState (BoostPadState *self_) noexcept
{
	return self_->state;
}

PyObject *BoostPadState::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<BoostPadState>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->state)::BoostPadState;

	return self.giftObject ();
}

int BoostPadState::Init (BoostPadState *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char isActiveKwd[]        = "is_active";
	static char cooldownKwd[]        = "cooldown";
	static char prevLockedCarIDKwd[] = "prev_locked_car_id";

	static char *dict[] = {isActiveKwd, cooldownKwd, prevLockedCarIDKwd, nullptr};

	::BoostPadState state{};

	int isActive                  = state.isActive;
	unsigned long prevLockedCarID = state.prevLockedCarID;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|pfk", dict, &isActive, &state.cooldown, &prevLockedCarID))
		return -1;

	state.isActive        = isActive;
	state.prevLockedCarID = prevLockedCarID;

	if (!InitFromBoostPadState (self_, state))
		return -1;

	return 0;
}

void BoostPadState::Dealloc (BoostPadState *self_) noexcept
{
	self_->state.~BoostPadState ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *BoostPadState::Pickle (BoostPadState *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	::BoostPadState model{};
	auto const state = ToBoostPadState (self_);

	if (state.isActive != model.isActive &&
	    !DictSetValue (dict.borrow (), "is_active", PyBool_FromLong (state.isActive)))
		return nullptr;

	if (state.cooldown != model.cooldown &&
	    !DictSetValue (dict.borrow (), "cooldown", PyFloat_FromDouble (state.cooldown)))
		return nullptr;

	if (state.prevLockedCarID != model.prevLockedCarID &&
	    !DictSetValue (dict.borrow (), "prev_locked_car_id", PyLong_FromUnsignedLong (state.prevLockedCarID)))
		return nullptr;

	return dict.gift ();
}

PyObject *BoostPadState::Unpickle (BoostPadState *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *BoostPadState::Copy (BoostPadState *self_) noexcept
{
	return NewFromBoostPadState (self_->state).giftObject ();
}

PyObject *BoostPadState::DeepCopy (BoostPadState *self_, PyObject *memo_) noexcept
{
	return NewFromBoostPadState (self_->state).giftObject ();
}
}
