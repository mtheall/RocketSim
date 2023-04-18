#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *BallState::Type = nullptr;

PyMemberDef BallState::Members[] = {
    {.name      = "last_hit_car_id",
        .type   = TypeHelper<decltype (::BallState::lastHitCarID)>::type,
        .offset = offsetof (BallState, state) + offsetof (::BallState, lastHitCarID),
        .flags  = 0,
        .doc    = "Last hit car id"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef BallState::Methods[] = {
    {.ml_name = "__getstate__", .ml_meth = (PyCFunction)&BallState::Pickle, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&BallState::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef BallState::GetSet[] = {
    GETSET_ENTRY (BallState, pos, "Position"),
    GETSET_ENTRY (BallState, vel, "Velocity"),
    GETSET_ENTRY (BallState, ang_vel, "Angular velocity"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot BallState::Slots[] = {
    {Py_tp_new, (void *)(&BallState::New)},
    {Py_tp_init, (void *)(&BallState::Init)},
    {Py_tp_dealloc, (void *)(&BallState::Dealloc)},
    {Py_tp_members, &BallState::Members},
    {Py_tp_methods, &BallState::Methods},
    {Py_tp_getset, &BallState::GetSet},
    {Py_tp_doc, (void *)R"(Ball state
__init__(self,
	pos: RocketSim.Vec = RocketSim.Vec(z = 93.15),
	vel: RocketSim.Vec = RocketSim.Vec(),
	ang_vel: RocketSim.Vec = RocketSim.Vec(),
	last_hit_car_id: int = 0))"},
    {0, nullptr},
};

PyType_Spec BallState::Spec = {
    .name      = "RocketSim.BallState",
    .basicsize = sizeof (BallState),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BallState::Slots,
};

PyRef<BallState> BallState::NewFromBallState (::BallState const &state_) noexcept
{
	auto const self = PyRef<BallState>::stealObject (BallState::New (BallState::Type, nullptr, nullptr));
	if (!self || !InitFromBallState (self.borrow (), state_))
		return nullptr;

	return self;
}

bool BallState::InitFromBallState (BallState *const self_, ::BallState const &state_) noexcept
{
	auto pos    = Vec::NewFromVec (state_.pos);
	auto vel    = Vec::NewFromVec (state_.vel);
	auto angVel = Vec::NewFromVec (state_.angVel);

	if (!pos || !vel || !angVel)
		return false;

	PyRef<Vec>::assign (self_->pos, pos.borrowObject ());
	PyRef<Vec>::assign (self_->vel, vel.borrowObject ());
	PyRef<Vec>::assign (self_->angVel, angVel.borrowObject ());

	self_->state = state_;

	return true;
}

::BallState BallState::ToBallState (BallState *self_) noexcept
{
	auto state = self_->state;

	state.pos    = Vec::ToVec (self_->pos);
	state.vel    = Vec::ToVec (self_->vel);
	state.angVel = Vec::ToVec (self_->angVel);

	return state;
}

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
	static char posKwd[]          = "pos";
	static char velKwd[]          = "vel";
	static char angVelKwd[]       = "ang_vel";
	static char lastHitCarIDKwd[] = "last_hit_car_id";

	static char *dict[] = {posKwd, velKwd, angVelKwd, lastHitCarIDKwd, nullptr};

	PyObject *pos       = nullptr; // borrowed references
	PyObject *vel       = nullptr;
	PyObject *angVel    = nullptr;
	unsigned long carId = 0;
	if (!PyArg_ParseTupleAndKeywords (
	        args_, kwds_, "|O!O!O!k", dict, Vec::Type, &pos, Vec::Type, &vel, Vec::Type, &angVel, &carId))
		return -1;

	::BallState state{};
	if (pos)
		state.pos = Vec::ToVec (reinterpret_cast<Vec *> (pos));
	if (vel)
		state.vel = Vec::ToVec (reinterpret_cast<Vec *> (vel));
	if (angVel)
		state.angVel = Vec::ToVec (reinterpret_cast<Vec *> (angVel));

	state.lastHitCarID = carId;

	if (!InitFromBallState (self_, state))
		return -1;

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

PyObject *BallState::Pickle (BallState *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	::BallState const model{};
	auto const state = ToBallState (self_);

	if (Vec::ToVec (self_->pos) != model.pos && !DictSetValue (dict.borrow (), "pos", PyNewRef (self_->pos)))
		return nullptr;

	if (Vec::ToVec (self_->vel) != model.vel && !DictSetValue (dict.borrow (), "vel", PyNewRef (self_->vel)))
		return nullptr;

	if (Vec::ToVec (self_->angVel) != model.angVel &&
	    !DictSetValue (dict.borrow (), "ang_vel", PyNewRef (self_->angVel)))
		return nullptr;

	if (state.lastHitCarID != model.lastHitCarID &&
	    !DictSetValue (dict.borrow (), "last_hit_car_id", PyLong_FromUnsignedLong (state.lastHitCarID)))
		return nullptr;

	return dict.gift ();
}

PyObject *BallState::Unpickle (BallState *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
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
