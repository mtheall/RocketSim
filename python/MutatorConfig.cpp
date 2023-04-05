#include "Module.h"

#include "Array.h"

#include <cstring>
#include <type_traits>

namespace RocketSim::Python
{
PyTypeObject *MutatorConfig::Type = nullptr;

PyMemberDef MutatorConfig::Members[] = {
    {.name      = "ball_drag",
        .type   = TypeHelper<decltype (::MutatorConfig::ballDrag)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, ballDrag),
        .flags  = 0,
        .doc    = "Ball drag"},
    {.name      = "ball_hit_extra_force_scale",
        .type   = TypeHelper<decltype (::MutatorConfig::ballHitExtraForceScale)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, ballHitExtraForceScale),
        .flags  = 0,
        .doc    = "Ball hit extra force scale"},
    {.name      = "ball_mass",
        .type   = TypeHelper<decltype (::MutatorConfig::ballMass)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, ballMass),
        .flags  = 0,
        .doc    = "Ball mass"},
    {.name      = "ball_max_speed",
        .type   = TypeHelper<decltype (::MutatorConfig::ballMaxSpeed)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, ballMaxSpeed),
        .flags  = 0,
        .doc    = "Ball max speed"},
    {.name      = "ball_radius",
        .type   = TypeHelper<decltype (::MutatorConfig::ballRadius)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, ballRadius),
        .flags  = 0,
        .doc    = "Ball radius"},
    {.name      = "ball_world_friction",
        .type   = TypeHelper<decltype (::MutatorConfig::ballWorldFriction)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, ballWorldFriction),
        .flags  = 0,
        .doc    = "Ball world friction"},
    {.name      = "ball_world_restitution",
        .type   = TypeHelper<decltype (::MutatorConfig::ballWorldRestitution)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, ballWorldRestitution),
        .flags  = 0,
        .doc    = "Ball world restitution"},
    {.name      = "boost_accel",
        .type   = TypeHelper<decltype (::MutatorConfig::boostAccel)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, boostAccel),
        .flags  = 0,
        .doc    = "Boost accel"},
    {.name      = "boost_pad_cooldown_big",
        .type   = TypeHelper<decltype (::MutatorConfig::boostPadCooldown_Big)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, boostPadCooldown_Big),
        .flags  = 0,
        .doc    = "Boost pad cooldown big"},
    {.name      = "boost_pad_cooldown_small",
        .type   = TypeHelper<decltype (::MutatorConfig::boostPadCooldown_Small)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, boostPadCooldown_Small),
        .flags  = 0,
        .doc    = "Boost pad cooldown small"},
    {.name      = "boost_used_per_second",
        .type   = TypeHelper<decltype (::MutatorConfig::boostUsedPerSecond)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, boostUsedPerSecond),
        .flags  = 0,
        .doc    = "Boost used per second"},
    {.name      = "bump_cooldown_time",
        .type   = TypeHelper<decltype (::MutatorConfig::bumpCooldownTime)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, bumpCooldownTime),
        .flags  = 0,
        .doc    = "Bump cooldown time"},
    {.name      = "bump_force_scale",
        .type   = TypeHelper<decltype (::MutatorConfig::bumpForceScale)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, bumpForceScale),
        .flags  = 0,
        .doc    = "Bump force scale"},
    {.name      = "car_mass",
        .type   = TypeHelper<decltype (::MutatorConfig::carMass)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, carMass),
        .flags  = 0,
        .doc    = "Car mass"},
    {.name      = "car_spawn_boost_amount",
        .type   = TypeHelper<decltype (::MutatorConfig::carSpawnBoostAmount)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, carSpawnBoostAmount),
        .flags  = 0,
        .doc    = "Car spawn boost amount"},
    {.name      = "car_world_friction",
        .type   = TypeHelper<decltype (::MutatorConfig::carWorldFriction)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, carWorldFriction),
        .flags  = 0,
        .doc    = "Car world friction"},
    {.name      = "car_world_restitution",
        .type   = TypeHelper<decltype (::MutatorConfig::carWorldRestitution)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, carWorldRestitution),
        .flags  = 0,
        .doc    = "Car world restitution"},
    {.name      = "demo_mode",
        .type   = TypeHelper<std::underlying_type_t<decltype (::MutatorConfig::demoMode)>>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, demoMode),
        .flags  = 0,
        .doc    = "Demo mode"},
    {.name      = "enable_team_demos",
        .type   = TypeHelper<decltype (::MutatorConfig::enableTeamDemos)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, enableTeamDemos),
        .flags  = 0,
        .doc    = "Enable team demos"},
    {.name      = "jump_accel",
        .type   = TypeHelper<decltype (::MutatorConfig::jumpAccel)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, jumpAccel),
        .flags  = 0,
        .doc    = "Jump acceleration"},
    {.name      = "jump_immediate_force",
        .type   = TypeHelper<decltype (::MutatorConfig::jumpImmediateForce)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, jumpImmediateForce),
        .flags  = 0,
        .doc    = "Jump immediate force"},
    {.name      = "respawn_delay",
        .type   = TypeHelper<decltype (::MutatorConfig::respawnDelay)>::type,
        .offset = offsetof (MutatorConfig, config) + offsetof (::MutatorConfig, respawnDelay),
        .flags  = 0,
        .doc    = "Respawn delay"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyGetSetDef MutatorConfig::GetSet[] = {
    GETSET_ENTRY (MutatorConfig, gravity),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot MutatorConfig::Slots[] = {
    {Py_tp_new, (void *)&MutatorConfig::New},
    {Py_tp_init, (void *)&MutatorConfig::Init},
    {Py_tp_dealloc, (void *)&MutatorConfig::Dealloc},
    {Py_tp_members, &MutatorConfig::Members},
    {Py_tp_getset, &MutatorConfig::GetSet},
    {0, nullptr},
};

PyType_Spec MutatorConfig::Spec = {
    .name      = "RocketSim.MutatorConfig",
    .basicsize = sizeof (MutatorConfig),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = MutatorConfig::Slots,
};

PyRef<MutatorConfig> MutatorConfig::NewFromMutatorConfig (::MutatorConfig const &config_) noexcept
{
	auto const self = PyRef<MutatorConfig>::stealObject (MutatorConfig::New (MutatorConfig::Type, nullptr, nullptr));
	if (!self || !InitFromMutatorConfig (self.borrow (), config_))
		return nullptr;

	return self;
}

bool MutatorConfig::InitFromMutatorConfig (MutatorConfig *const self_, ::MutatorConfig const &config_) noexcept
{
	auto const gravity = Vec::NewFromVec (config_.gravity);

	if (!gravity)
		return false;

	PyRef<Vec>::assign (self_->gravity, gravity.borrowObject ());

	self_->config = config_;
	return true;
}

::MutatorConfig MutatorConfig::ToMutatorConfig (MutatorConfig *self_) noexcept
{
	auto config = self_->config;

	config.gravity = Vec::ToVec (self_->gravity);

	return config;
}

PyObject *MutatorConfig::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<MutatorConfig>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->config)::MutatorConfig{};
	self->gravity = nullptr;

	return self.giftObject ();
}

int MutatorConfig::Init (MutatorConfig *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	::MutatorConfig config{};
	if (!InitFromMutatorConfig (self_, config))
		return -1;

	return 0;
}

void MutatorConfig::Dealloc (MutatorConfig *self_) noexcept
{
	self_->config.~MutatorConfig ();

	Py_XDECREF (self_->gravity);

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *MutatorConfig::Getgravity (MutatorConfig *self_, void *) noexcept
{
	return PyRef<Vec>::incRef (self_->gravity).giftObject ();
}

int MutatorConfig::Setgravity (MutatorConfig *self_, PyObject *value_, void *) noexcept
{
	if (!value_)
	{
		PyErr_SetString (PyExc_TypeError, "can't delete 'gravity' attribute of 'RocketSim.MutatorConfig' objects");
		return -1;
	}

	if (!Py_IS_TYPE (value_, Vec::Type))
	{
		PyErr_SetString (PyExc_TypeError, "attribute value type must be RocketSim.Vec");
		return -1;
	}

	PyRef<Vec>::assign (self_->gravity, value_);

	return 0;
}
}
