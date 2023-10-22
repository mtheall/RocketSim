#include "Module.h"

namespace RocketSim::Python
{
PyTypeObject *Ball::Type = nullptr;

PyMethodDef Ball::Methods[] = {
    {.ml_name     = "get_radius",
        .ml_meth  = (PyCFunction)&Ball::GetRadius,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_radius(self) -> float
Get ball radius)"},
    {.ml_name     = "get_state",
        .ml_meth  = (PyCFunction)&Ball::GetState,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_state(self) -> RocketSim.BallState
Get ball state)"},
    {.ml_name     = "set_state",
        .ml_meth  = (PyCFunction)&Ball::SetState,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_state(self, state: RocketSim.BallState)
Set ball state)"},
    {.ml_name     = "get_rot",
        .ml_meth  = (PyCFunction)&Ball::GetRot,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(get_rot(self) -> (float, float, float, float)
Get ball rotation as a quaternion (xyzw))"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyType_Slot Ball::Slots[] = {
    {Py_tp_new, (void *)&Ball::NewStub},
    {Py_tp_init, nullptr},
    {Py_tp_dealloc, (void *)&Ball::Dealloc},
    {Py_tp_methods, &Ball::Methods},
    {Py_tp_doc, (void *)"Ball"},
    {0, nullptr},
};

PyType_Spec Ball::Spec = {
    .name      = "RocketSim.Ball",
    .basicsize = sizeof (Ball),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = Ball::Slots,
};

Ball *Ball::New () noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (Type, Py_tp_alloc);

	auto self = PyRef<Ball>::stealObject (tp_alloc (Type, 0));
	if (!self)
		return nullptr;

	new (&self->arena) std::shared_ptr<::Arena>{};
	self->ball = nullptr;

	return self.gift ();
}

PyObject *Ball::NewStub (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	PyErr_SetString (PyExc_TypeError, "cannot create 'RocketSim.Ball' instances");
	return nullptr;
}

void Ball::Dealloc (Ball *self_) noexcept
{
	self_->arena.~shared_ptr ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Ball::GetRot (Ball *self_) noexcept
{
	auto tuple = PyObjectRef::steal (PyTuple_New (4));
	if (!tuple)
		return nullptr;

	auto rot = self_->ball->_rigidBody.getOrientation ();

	PyTuple_SetItem (tuple.borrow (), 0, PyFloat_FromDouble (rot.getX ()));
	PyTuple_SetItem (tuple.borrow (), 1, PyFloat_FromDouble (rot.getY ()));
	PyTuple_SetItem (tuple.borrow (), 2, PyFloat_FromDouble (rot.getZ ()));
	PyTuple_SetItem (tuple.borrow (), 3, PyFloat_FromDouble (rot.getW ()));

	return tuple.giftObject ();
}

PyObject *Ball::GetRadius (Ball *self_) noexcept
{
	return PyFloat_FromDouble (self_->ball->GetRadius ());
}

PyObject *Ball::GetState (Ball *self_) noexcept
{
	auto state = BallState::NewFromBallState (self_->ball->GetState ());
	if (!state)
		return nullptr;

	return state.giftObject ();
}

PyObject *Ball::SetState (Ball *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char stateKwd[] = "state";

	static char *dict[] = {stateKwd, nullptr};

	BallState *state;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O!", dict, BallState::Type, &state))
		return nullptr;

	self_->ball->SetState (BallState::ToBallState (state));

	Py_RETURN_NONE;
}
}
