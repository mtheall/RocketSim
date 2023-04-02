#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *BallHitInfo::Type = nullptr;

PyMemberDef BallHitInfo::Members[] = {
    {.name      = "is_valid",
        .type   = TypeHelper<decltype (::BallHitInfo::isValid)>::type,
        .offset = offsetof (BallHitInfo, info) + offsetof (::BallHitInfo, isValid),
        .flags  = 0,
        .doc    = "Is valid"},
    {.name      = "tick_count_when_hit",
        .type   = TypeHelper<decltype (::BallHitInfo::tickCountWhenHit)>::type,
        .offset = offsetof (BallHitInfo, info) + offsetof (::BallHitInfo, tickCountWhenHit),
        .flags  = 0,
        .doc    = "Tick count when hit"},
    {.name      = "tick_count_when_extra_impulse_applied",
        .type   = TypeHelper<decltype (::BallHitInfo::tickCountWhenExtraImpulseApplied)>::type,
        .offset = offsetof (BallHitInfo, info) + offsetof (::BallHitInfo, tickCountWhenExtraImpulseApplied),
        .flags  = 0,
        .doc    = "Tick count when extra impulse applied"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef BallHitInfo::Methods[] = {
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef BallHitInfo::GetSet[] = {
    GETSET_ENTRY (BallHitInfo, relative_pos_on_ball),
    GETSET_ENTRY (BallHitInfo, ball_pos),
    GETSET_ENTRY (BallHitInfo, extra_hit_vel),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot BallHitInfo::Slots[] = {
    {Py_tp_new, (void *)(&BallHitInfo::New)},
    {Py_tp_init, (void *)(&BallHitInfo::Init)},
    {Py_tp_dealloc, (void *)(&BallHitInfo::Dealloc)},
    {Py_tp_members, &BallHitInfo::Members},
    {Py_tp_methods, &BallHitInfo::Methods},
    {Py_tp_getset, &BallHitInfo::GetSet},
    {0, nullptr},
};

PyType_Spec BallHitInfo::Spec = {
    .name      = "RocketSim.BallHitInfo",
    .basicsize = sizeof (BallHitInfo),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BallHitInfo::Slots,
};

PyRef<BallHitInfo> BallHitInfo::NewFromBallHitInfo (::BallHitInfo const &info_) noexcept
{
	auto const self = PyRef<BallHitInfo>::stealObject (BallHitInfo::New (BallHitInfo::Type, nullptr, nullptr));
	if (!self || !InitFromBallHitInfo (self.borrow (), info_))
		return nullptr;

	return self;
}

bool BallHitInfo::InitFromBallHitInfo (BallHitInfo *const self_, ::BallHitInfo const &info_) noexcept
{
	auto relativePosOnBall = Vec::NewFromVec (info_.relativePosOnBall);
	auto ballPos           = Vec::NewFromVec (info_.ballPos);
	auto extraHitVel       = Vec::NewFromVec (info_.extraHitVel);

	if (!relativePosOnBall || !ballPos || !extraHitVel)
		return false;

	PyRef<Vec>::assign (self_->relativePosOnBall, relativePosOnBall.borrowObject ());
	PyRef<Vec>::assign (self_->ballPos, ballPos.borrowObject ());
	PyRef<Vec>::assign (self_->extraHitVel, extraHitVel.borrowObject ());

	self_->info = info_;

	return true;
}

PyObject *BallHitInfo::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<BallHitInfo>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->info)::BallHitInfo ();

	self->relativePosOnBall = nullptr;
	self->ballPos           = nullptr;
	self->extraHitVel       = nullptr;

	return self.giftObject ();
}

int BallHitInfo::Init (BallHitInfo *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (!InitFromBallHitInfo (self_, ::BallHitInfo{}))
		return -1;

	return 0;
}

void BallHitInfo::Dealloc (BallHitInfo *self_) noexcept
{
	Py_XDECREF (self_->relativePosOnBall);
	Py_XDECREF (self_->ballPos);
	Py_XDECREF (self_->extraHitVel);

	self_->info.~BallHitInfo ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *BallHitInfo::Getrelative_pos_on_ball (BallHitInfo *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->relativePosOnBall).giftObject ();
}

int BallHitInfo::Setrelative_pos_on_ball (BallHitInfo *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_TypeError, "can't delete 'relative_pos_on_ball' attribute of 'RocketSim.BallHitInfo' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	if (value_ == (PyObject *)self_->relativePosOnBall)
		return 0;

	PyRef<Vec>::assign (self_->relativePosOnBall, value_);

	return 0;
}

PyObject *BallHitInfo::Getball_pos (BallHitInfo *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->ballPos).giftObject ();
}

int BallHitInfo::Setball_pos (BallHitInfo *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'ball_pos' attribute of 'RocketSim.BallHitInfo' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->ballPos, value_);

	return 0;
}

PyObject *BallHitInfo::Getextra_hit_vel (BallHitInfo *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->extraHitVel).giftObject ();
}

int BallHitInfo::Setextra_hit_vel (BallHitInfo *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'extra_hit_vel' attribute of 'RocketSim.BallHitInfo' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->extraHitVel, value_);

	return 0;
}
}
