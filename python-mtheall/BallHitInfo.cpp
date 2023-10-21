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
    {.ml_name     = "__getstate__",
        .ml_meth  = (PyCFunction)&BallHitInfo::Pickle,
        .ml_flags = METH_NOARGS,
        .ml_doc   = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&BallHitInfo::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&BallHitInfo::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.BallHitInfo
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&BallHitInfo::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.BallHitInfo
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef BallHitInfo::GetSet[] = {
    GETSET_ENTRY (BallHitInfo, relative_pos_on_ball, "Relative position on ball"),
    GETSET_ENTRY (BallHitInfo, ball_pos, "Ball position"),
    GETSET_ENTRY (BallHitInfo, extra_hit_vel, "Extra hit velocity"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot BallHitInfo::Slots[] = {
    {Py_tp_new, (void *)(&BallHitInfo::New)},
    {Py_tp_init, (void *)(&BallHitInfo::Init)},
    {Py_tp_dealloc, (void *)(&BallHitInfo::Dealloc)},
    {Py_tp_members, &BallHitInfo::Members},
    {Py_tp_methods, &BallHitInfo::Methods},
    {Py_tp_getset, &BallHitInfo::GetSet},
    {Py_tp_doc, (void *)R"(Ball hit info
__init__(self,
	is_valid: bool = False,
	relative_pos_on_ball: RocketSim.Vec = RocketSim.Vec(),
	ball_pos: RocketSim.Vec = RocketSim.Vec(),
	extra_hit_vel: RocketSim.Vec = RocketSim.Vec(),
	tick_count_when_hit: int = 18446744073709551615,
	tick_count_when_extra_impulse_applied: int = 18446744073709551615))"},
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

::BallHitInfo BallHitInfo::ToBallHitInfo (BallHitInfo *self_) noexcept
{
	auto info = self_->info;

	info.relativePosOnBall = Vec::ToVec (self_->relativePosOnBall);
	info.ballPos           = Vec::ToVec (self_->ballPos);
	info.extraHitVel       = Vec::ToVec (self_->extraHitVel);

	return info;
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
	static char isValidKwd[]                          = "is_valid";
	static char relativePosOnBallKwd[]                = "relative_pos_on_ball";
	static char ballPosKwd[]                          = "ball_pos";
	static char extraHitVelKwd[]                      = "extra_hit_vel";
	static char tickCountWhenHitKwd[]                 = "tick_count_when_hit";
	static char tickCountWhenExtraImpulseAppliedKwd[] = "tick_count_when_extra_impulse_applied";

	static char *dict[] = {isValidKwd,
	    relativePosOnBallKwd,
	    ballPosKwd,
	    extraHitVelKwd,
	    tickCountWhenHitKwd,
	    tickCountWhenExtraImpulseAppliedKwd,
	    nullptr};

	::BallHitInfo info{};

	int isValid                                         = info.isValid;
	PyObject *relativePosOnBall                         = nullptr;
	PyObject *ballPos                                   = nullptr;
	PyObject *extraHitVel                               = nullptr;
	unsigned long long tickCountWhenHit                 = info.tickCountWhenHit;
	unsigned long long tickCountWhenExtraImpulseApplied = info.tickCountWhenExtraImpulseApplied;

	if (!PyArg_ParseTupleAndKeywords (args_,
	        kwds_,
	        "|pO!O!O!KK",
	        dict,
	        &isValid,
	        Vec::Type,
	        &relativePosOnBall,
	        Vec::Type,
	        &ballPos,
	        Vec::Type,
	        &extraHitVel,
	        &tickCountWhenHit,
	        &tickCountWhenExtraImpulseApplied))
		return -1;

	info.isValid = isValid;

	if (relativePosOnBall)
		info.relativePosOnBall = Vec::ToVec (PyCast<Vec> (relativePosOnBall));
	if (ballPos)
		info.ballPos = Vec::ToVec (PyCast<Vec> (ballPos));
	if (extraHitVel)
		info.extraHitVel = Vec::ToVec (PyCast<Vec> (extraHitVel));

	info.tickCountWhenHit                 = tickCountWhenHit;
	info.tickCountWhenExtraImpulseApplied = tickCountWhenExtraImpulseApplied;

	if (!InitFromBallHitInfo (self_, info))
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

PyObject *BallHitInfo::Pickle (BallHitInfo *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	::BallHitInfo const model{};
	auto const info = ToBallHitInfo (self_);

	if (info.isValid != model.isValid && !DictSetValue (dict.borrow (), "is_valid", PyBool_FromLong (info.isValid)))
		return nullptr;

	if (Vec::ToVec (self_->relativePosOnBall) != model.relativePosOnBall &&
	    !DictSetValue (dict.borrow (), "relative_pos_on_ball", PyNewRef (self_->relativePosOnBall)))
		return nullptr;

	if (Vec::ToVec (self_->ballPos) != model.ballPos &&
	    !DictSetValue (dict.borrow (), "ball_pos", PyNewRef (self_->ballPos)))
		return nullptr;

	if (Vec::ToVec (self_->extraHitVel) != model.extraHitVel &&
	    !DictSetValue (dict.borrow (), "extra_hit_vel", PyNewRef (self_->extraHitVel)))
		return nullptr;

	if (info.tickCountWhenHit != model.tickCountWhenHit &&
	    !DictSetValue (dict.borrow (), "tick_count_when_hit", PyLong_FromUnsignedLongLong (info.tickCountWhenHit)))
		return nullptr;

	if (info.tickCountWhenExtraImpulseApplied != model.tickCountWhenExtraImpulseApplied &&
	    !DictSetValue (dict.borrow (),
	        "tick_count_when_extra_impulse_applied",
	        PyLong_FromUnsignedLongLong (info.tickCountWhenExtraImpulseApplied)))
		return nullptr;

	return dict.gift ();
}

PyObject *BallHitInfo::Unpickle (BallHitInfo *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *BallHitInfo::Copy (BallHitInfo *self_) noexcept
{
	auto info = PyRef<BallHitInfo>::stealObject (New (Type, nullptr, nullptr));
	if (!info)
		return nullptr;

	PyRef<Vec>::assign (info->relativePosOnBall, reinterpret_cast<PyObject *> (self_->relativePosOnBall));
	PyRef<Vec>::assign (info->ballPos, reinterpret_cast<PyObject *> (self_->ballPos));
	PyRef<Vec>::assign (info->extraHitVel, reinterpret_cast<PyObject *> (self_->extraHitVel));

	info->info = ToBallHitInfo (self_);

	return info.giftObject ();
}

PyObject *BallHitInfo::DeepCopy (BallHitInfo *self_, PyObject *memo_) noexcept
{
	auto info = PyRef<BallHitInfo>::stealObject (New (Type, nullptr, nullptr));
	if (!info)
		return nullptr;

	PyRef<Vec>::assign (info->relativePosOnBall, PyDeepCopy (self_->relativePosOnBall, memo_));
	if (!info->relativePosOnBall)
		return nullptr;

	PyRef<Vec>::assign (info->ballPos, PyDeepCopy (self_->ballPos, memo_));
	if (!info->ballPos)
		return nullptr;

	PyRef<Vec>::assign (info->extraHitVel, PyDeepCopy (self_->extraHitVel, memo_));
	if (!info->extraHitVel)
		return nullptr;

	info->info = ToBallHitInfo (self_);

	return info.giftObject ();
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
