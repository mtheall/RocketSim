#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *CarState::Type = nullptr;

PyMemberDef CarState::Members[] = {
    {.name      = "is_on_ground",
        .type   = TypeHelper<decltype (::CarState::isOnGround)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isOnGround),
        .flags  = 0,
        .doc    = "Is on ground"},
    {.name      = "has_jumped",
        .type   = TypeHelper<decltype (::CarState::hasJumped)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasJumped),
        .flags  = 0,
        .doc    = "Has jumped"},
    {.name      = "has_double_jumped",
        .type   = TypeHelper<decltype (::CarState::hasDoubleJumped)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasDoubleJumped),
        .flags  = 0,
        .doc    = "Has double jumped"},
    {.name      = "has_flipped",
        .type   = TypeHelper<decltype (::CarState::hasFlipped)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, hasFlipped),
        .flags  = 0,
        .doc    = "Has flipped"},
    {.name      = "jump_time",
        .type   = TypeHelper<decltype (::CarState::jumpTime)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, jumpTime),
        .flags  = 0,
        .doc    = "Jump time"},
    {.name      = "flip_time",
        .type   = TypeHelper<decltype (::CarState::flipTime)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, flipTime),
        .flags  = 0,
        .doc    = "Flip time"},
    {.name      = "is_jumping",
        .type   = TypeHelper<decltype (::CarState::isJumping)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isJumping),
        .flags  = 0,
        .doc    = "Is jumping"},
    {.name      = "air_time_since_jump",
        .type   = TypeHelper<decltype (::CarState::airTimeSinceJump)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, airTimeSinceJump),
        .flags  = 0,
        .doc    = "Air time since jump"},
    {.name      = "boost",
        .type   = TypeHelper<decltype (::CarState::boost)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, boost),
        .flags  = 0,
        .doc    = "Boost"},
    {.name      = "time_spent_boosting",
        .type   = TypeHelper<decltype (::CarState::timeSpentBoosting)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, timeSpentBoosting),
        .flags  = 0,
        .doc    = "Time spent boosting"},
    {.name      = "is_supersonic",
        .type   = TypeHelper<decltype (::CarState::isSupersonic)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isSupersonic),
        .flags  = 0,
        .doc    = "Is supersonic"},
    {.name      = "supersonic_time",
        .type   = TypeHelper<decltype (::CarState::supersonicTime)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, supersonicTime),
        .flags  = 0,
        .doc    = "Supersonic time"},
    {.name      = "handbrake_val",
        .type   = TypeHelper<decltype (::CarState::handbrakeVal)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, handbrakeVal),
        .flags  = 0,
        .doc    = "Handbrake val"},
    {.name      = "is_auto_flipping",
        .type   = TypeHelper<decltype (::CarState::isAutoFlipping)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isAutoFlipping),
        .flags  = 0,
        .doc    = "Is auto flipping"},
    {.name      = "auto_flip_timer",
        .type   = TypeHelper<decltype (::CarState::autoFlipTimer)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, autoFlipTimer),
        .flags  = 0,
        .doc    = "Auto flip timer"},
    {.name      = "auto_flip_torque_scale",
        .type   = TypeHelper<decltype (::CarState::autoFlipTorqueScale)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, autoFlipTorqueScale),
        .flags  = 0,
        .doc    = "Auto flip torque scale"},
    {.name      = "has_world_contact",
        .type   = TypeHelper<decltype (::CarState::worldContact.hasContact)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, worldContact.hasContact),
        .flags  = 0,
        .doc    = "Has world contact"},
    {.name      = "car_contact_id",
        .type   = TypeHelper<decltype (::CarState::carContact.otherCarID)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, carContact.otherCarID),
        .flags  = 0,
        .doc    = "Car contact other car id"},
    {.name      = "car_contact_cooldown_timer",
        .type   = TypeHelper<decltype (::CarState::carContact.cooldownTimer)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, carContact.cooldownTimer),
        .flags  = 0,
        .doc    = "Car contact cooldown timer"},
    {.name      = "is_demoed",
        .type   = TypeHelper<decltype (::CarState::isDemoed)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, isDemoed),
        .flags  = 0,
        .doc    = "Is demoed"},
    {.name      = "demo_respawn_timer",
        .type   = TypeHelper<decltype (::CarState::demoRespawnTimer)>::type,
        .offset = offsetof (CarState, state) + offsetof (::CarState, demoRespawnTimer),
        .flags  = 0,
        .doc    = "Demo respawn timer"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyGetSetDef CarState::GetSet[] = {
    GETSET_ENTRY (CarState, pos),
    GETSET_ENTRY (CarState, rot_mat),
    GETSET_ENTRY (CarState, vel),
    GETSET_ENTRY (CarState, ang_vel),
    GETSET_ENTRY (CarState, last_rel_dodge_torque),
    GETSET_ENTRY (CarState, last_controls),
    GETSET_ENTRY (CarState, world_contact_normal),
    GETSET_ENTRY (CarState, ball_hit_info),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot CarState::Slots[] = {
    {Py_tp_new, (void *)&CarState::New},
    {Py_tp_init, (void *)&CarState::Init},
    {Py_tp_dealloc, (void *)&CarState::Dealloc},
    {Py_tp_members, &CarState::Members},
    {Py_tp_getset, &CarState::GetSet},
    {0, nullptr},
};

PyType_Spec CarState::Spec = {
    .name      = "RocketSim.CarState",
    .basicsize = sizeof (CarState),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = CarState::Slots,
};

PyRef<CarState> CarState::NewFromCarState (::CarState const &state_) noexcept
{
	auto const self = PyRef<CarState>::stealObject (CarState::New (CarState::Type, nullptr, nullptr));
	if (!self || !InitFromCarState (self.borrow (), state_))
		return nullptr;

	return self;
}

bool CarState::InitFromCarState (CarState *const self_, ::CarState const &state_) noexcept
{
	auto pos                = Vec::NewFromVec (state_.pos);
	auto rotMat             = RotMat::NewFromRotMat (state_.rotMat);
	auto vel                = Vec::NewFromVec (state_.vel);
	auto angVel             = Vec::NewFromVec (state_.angVel);
	auto lastRelDodgeTorque = Vec::NewFromVec (state_.lastRelDodgeTorque);
	auto lastControls       = CarControls::NewFromCarControls (state_.lastControls);
	auto worldContactNormal = Vec::NewFromVec (state_.worldContact.contactNormal);
	auto ballHitInfo        = BallHitInfo::NewFromBallHitInfo (state_.ballHitInfo);

	if (!pos || !rotMat || !vel || !angVel || !lastRelDodgeTorque || !lastControls || !worldContactNormal ||
	    !ballHitInfo)
		return false;

	PyRef<Vec>::assign (self_->pos, pos.borrowObject ());
	PyRef<RotMat>::assign (self_->rotMat, rotMat.borrowObject ());
	PyRef<Vec>::assign (self_->vel, vel.borrowObject ());
	PyRef<Vec>::assign (self_->angVel, angVel.borrowObject ());
	PyRef<Vec>::assign (self_->lastRelDodgeTorque, lastRelDodgeTorque.borrowObject ());
	PyRef<CarControls>::assign (self_->lastControls, lastControls.borrowObject ());
	PyRef<Vec>::assign (self_->worldContactNormal, worldContactNormal.borrowObject ());
	PyRef<BallHitInfo>::assign (self_->ballHitInfo, ballHitInfo.borrowObject ());

	self_->state = state_;

	return true;
}

PyObject *CarState::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<CarState>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->state)::CarState ();

	self->pos                = nullptr;
	self->rotMat             = nullptr;
	self->vel                = nullptr;
	self->angVel             = nullptr;
	self->lastRelDodgeTorque = nullptr;
	self->lastControls       = nullptr;
	self->worldContactNormal = nullptr;

	return self.giftObject ();
}

int CarState::Init (CarState *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (!InitFromCarState (self_, ::CarState{}))
		return -1;

	return 0;
}

void CarState::Dealloc (CarState *self_) noexcept
{
	Py_XDECREF (self_->pos);
	Py_XDECREF (self_->rotMat);
	Py_XDECREF (self_->vel);
	Py_XDECREF (self_->angVel);
	Py_XDECREF (self_->lastRelDodgeTorque);
	Py_XDECREF (self_->lastControls);

	self_->state.~CarState ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *CarState::Getpos (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->pos).giftObject ();
}

int CarState::Setpos (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'pos' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->pos, value_);

	return 0;
}

PyObject *CarState::Getrot_mat (CarState *self_, void *) noexcept
{
	return PyRef<RotMat>::incRef (self_->rotMat).giftObject ();
}

int CarState::Setrot_mat (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'rot_mat' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, RotMat::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.RotMat");
		return -1;
	}

	PyRef<RotMat>::assign (self_->rotMat, value_);

	return 0;
}

PyObject *CarState::Getvel (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->vel).giftObject ();
}

int CarState::Setvel (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'vel' attribute of 'RocketSim.CarState' objects");
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

PyObject *CarState::Getang_vel (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->angVel).giftObject ();
}

int CarState::Setang_vel (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'ang_vel' attribute of 'RocketSim.CarState' objects");
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

PyObject *CarState::Getlast_rel_dodge_torque (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->lastRelDodgeTorque).giftObject ();
}

int CarState::Setlast_rel_dodge_torque (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_TypeError, "can't delete 'last_rel_dodge_torque' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->lastRelDodgeTorque, value_);

	return 0;
}

PyObject *CarState::Getlast_controls (CarState *self_, void *) noexcept
{
	return PyRef<CarControls>::incRef (self_->lastControls).giftObject ();
}

int CarState::Setlast_controls (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'last_controls' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, CarControls::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.CarControls");
		return -1;
	}

	PyRef<CarControls>::assign (self_->lastControls, value_);

	return 0;
}

PyObject *CarState::Getworld_contact_normal (CarState *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->worldContactNormal).giftObject ();
}

int CarState::Setworld_contact_normal (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (
		    PyExc_TypeError, "can't delete 'world_contact_normal' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->worldContactNormal, value_);

	return 0;
}

PyObject *CarState::Getball_hit_info (CarState *self_, void *) noexcept
{
	return PyRef<BallHitInfo>::incRef (self_->ballHitInfo).giftObject ();
}

int CarState::Setball_hit_info (CarState *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'ball_hit_info' attribute of 'RocketSim.CarState' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, BallHitInfo::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.BallHitInfo");
		return -1;
	}

	PyRef<BallHitInfo>::assign (self_->ballHitInfo, value_);

	return 0;
}
}
