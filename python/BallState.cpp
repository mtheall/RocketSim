#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *BallState::Type = nullptr;

PyMemberDef BallState::Members[] = {
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef BallState::Methods[] = {
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef BallState::GetSet[] = {
    GETSET_ENTRY (BallState, pos),
    GETSET_ENTRY (BallState, vel),
    GETSET_ENTRY (BallState, ang_vel),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot BallState::Slots[] = {
    {Py_tp_new, (void *)(&BallState::New)},
    {Py_tp_init, (void *)(&BallState::Init)},
    {Py_tp_dealloc, (void *)(&BallState::Dealloc)},
    {Py_tp_members, &BallState::Members},
    {Py_tp_methods, &BallState::Methods},
    {Py_tp_getset, &BallState::GetSet},
    {0, nullptr},
};

PyType_Spec BallState::Spec = {
    .name      = "RocketSim.BallState",
    .basicsize = sizeof (BallState),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BallState::Slots,
};

PyObject *BallState::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<BallState>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->state)::BallState ();

	self->pos    = nullptr;
	self->vel    = nullptr;
	self->angVel = nullptr;

	return self.giftObject ();
}

int BallState::Init (BallState *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	self_->state = ::BallState{};

	auto pos    = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto vel    = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	auto angVel = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));

	if (!pos || !vel || !angVel)
		return -1;

	PyRef<Vec>::assign (self_->pos, pos.borrowObject ());
	PyRef<Vec>::assign (self_->vel, vel.borrowObject ());
	PyRef<Vec>::assign (self_->angVel, angVel.borrowObject ());

	self_->pos->vec    = self_->state.pos;
	self_->vel->vec    = self_->state.vel;
	self_->angVel->vec = self_->state.angVel;

	return 0;
}

void BallState::Dealloc (BallState *self_) noexcept
{
	Py_XDECREF (self_->pos);
	Py_XDECREF (self_->vel);
	Py_XDECREF (self_->angVel);

	self_->state.~BallState ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *BallState::Getpos (BallState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->pos).giftObject ();
}

int BallState::Setpos (BallState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'pos' attribute of 'RocketSim.BallState' objects");
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

PyObject *BallState::Getvel (BallState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->vel).giftObject ();
}

int BallState::Setvel (BallState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'vel' attribute of 'RocketSim.BallState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->vel, value_);

	return 0;
}

PyObject *BallState::Getang_vel (BallState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->angVel).giftObject ();
}

int BallState::Setang_vel (BallState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'ang_vel' attribute of 'RocketSim.BallState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->angVel, value_);

	return 0;
}
}
