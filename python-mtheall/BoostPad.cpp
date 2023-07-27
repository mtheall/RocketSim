#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *BoostPad::Type = nullptr;

PyMemberDef BoostPad::Members[] = {
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef BoostPad::Methods[] = {
    {.ml_name     = "get_pos",
        .ml_meth  = (PyCFunction)&BoostPad::GetPos,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_pos(self) -> RocketSim.Vec
Get position)"},
    {.ml_name     = "get_state",
        .ml_meth  = (PyCFunction)&BoostPad::GetState,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_state(self) -> RocketSim.BoostPadState
Get state)"},
    {.ml_name     = "set_state",
        .ml_meth  = (PyCFunction)&BoostPad::SetState,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_state(self, state: RocketSim.BoostPadState)
Set state)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef BoostPad::GetSet[] = {
    GETONLY_ENTRY (BoostPad, is_big, "Is big"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot BoostPad::Slots[] = {
    {Py_tp_new, (void *)&BoostPad::NewStub},
    {Py_tp_init, nullptr},
    {Py_tp_dealloc, (void *)&BoostPad::Dealloc},
    {Py_tp_members, &BoostPad::Members},
    {Py_tp_methods, &BoostPad::Methods},
    {Py_tp_getset, &BoostPad::GetSet},
    {Py_tp_doc, (void *)"Boost pad"},
    {0, nullptr},
};

PyType_Spec BoostPad::Spec = {
    .name      = "RocketSim.BoostPad",
    .basicsize = sizeof (BoostPad),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BoostPad::Slots,
};

BoostPad *BoostPad::New () noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (Type, Py_tp_alloc);

	auto self = PyRef<BoostPad>::stealObject (tp_alloc (Type, 0));
	if (!self)
		return nullptr;

	new (&self->arena) std::shared_ptr<::Arena>{};
	self->pad = nullptr;

	return self.gift ();
}

PyObject *BoostPad::NewStub (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	PyErr_SetString (PyExc_TypeError, "cannot create 'RocketSim.BoostPad' instances");
	return nullptr;
}

void BoostPad::Dealloc (BoostPad *self_) noexcept
{
	self_->arena.~shared_ptr ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *BoostPad::Getis_big (BoostPad *self_, void *) noexcept
{
	if (self_->pad->isBig)
		Py_RETURN_TRUE;

	Py_RETURN_FALSE;
}

PyObject *BoostPad::GetPos (BoostPad *self_) noexcept
{
	auto pos = Vec::NewFromVec (self_->pad->pos);
	if (!pos)
		return nullptr;

	return pos.giftObject ();
}

PyObject *BoostPad::GetState (BoostPad *self_) noexcept
{
	auto state = BoostPadState::NewFromBoostPadState (self_->pad->GetState ());
	if (!state)
		return nullptr;

	return state.giftObject ();
}

PyObject *BoostPad::SetState (BoostPad *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char stateKwd[] = "state";

	static char *dict[] = {stateKwd, nullptr};

	BoostPadState *state;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O!", dict, BoostPadState::Type, &state))
		return nullptr;

	self_->pad->SetState (BoostPadState::ToBoostPadState (state));

	Py_RETURN_NONE;
}
}
