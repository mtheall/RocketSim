#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *BoostPadState::Type = nullptr;

PyMemberDef BoostPadState::Members[] = {
    {.name      = "is_active",
        .type   = T_BOOL,
        .offset = offsetof (BoostPadState, state) + offsetof (::BoostPadState, isActive),
        .flags  = 0,
        .doc    = "Is active"},
    {.name      = "cooldown",
        .type   = T_FLOAT,
        .offset = offsetof (BoostPadState, state) + offsetof (::BoostPadState, cooldown),
        .flags  = 0,
        .doc    = "Cooldown"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyType_Slot BoostPadState::Slots[] = {
    {Py_tp_new, (void *)&BoostPadState::New},
    {Py_tp_init, (void *)&BoostPadState::Init},
    {Py_tp_dealloc, (void *)&BoostPadState::Dealloc},
    {Py_tp_members, &BoostPadState::Members},
    {0, nullptr},
};

PyType_Spec BoostPadState::Spec = {
    .name      = "RocketSim.BoostPadState",
    .basicsize = sizeof (BoostPadState),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BoostPadState::Slots,
};

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
	PyObject *isActiveObj = nullptr; // borrowed reference
	float cooldown        = 0.0f;
	if (!PyArg_ParseTuple (args_, "|Of", &isActiveObj, &cooldown))
		return -1;

	if (isActiveObj)
	{
		auto const isActive = PyObject_IsTrue (isActiveObj);
		if (isActive < 0)
			return -1;

		self_->state.isActive = isActive;
	}

	self_->state.cooldown = cooldown;

	return 0;
}

void BoostPadState::Dealloc (BoostPadState *self_) noexcept
{
	self_->state.~BoostPadState ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}
}
