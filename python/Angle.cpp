#include "Module.h"

#include "Array.h"

#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *Angle::Type = nullptr;

PyMemberDef Angle::Members[] = {
    {.name      = "yaw",
        .type   = T_FLOAT,
        .offset = offsetof (Angle, angle) + offsetof (::Angle, yaw),
        .flags  = 0,
        .doc    = "yaw"},
    {.name      = "pitch",
        .type   = T_FLOAT,
        .offset = offsetof (Angle, angle) + offsetof (::Angle, pitch),
        .flags  = 0,
        .doc    = "pitch"},
    {.name      = "roll",
        .type   = T_FLOAT,
        .offset = offsetof (Angle, angle) + offsetof (::Angle, roll),
        .flags  = 0,
        .doc    = "roll"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef Angle::Methods[] = {
    {.ml_name = "__format__", .ml_meth = (PyCFunction)&Angle::Format, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name = "as_tuple", .ml_meth = (PyCFunction)&Angle::AsTuple, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name = "as_numpy", .ml_meth = (PyCFunction)&Angle::AsNumpy, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyType_Slot Angle::Slots[] = {
    {Py_tp_new, (void *)&Angle::New},
    {Py_tp_init, (void *)&Angle::Init},
    {Py_tp_dealloc, (void *)&Angle::Dealloc},
    {Py_tp_repr, (void *)&Angle::Repr},
    {Py_tp_members, &Angle::Members},
    {Py_tp_methods, &Angle::Methods},
    {0, nullptr},
};

PyType_Spec Angle::Spec = {
    .name      = "RocketSim.Angle",
    .basicsize = sizeof (Angle),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = Angle::Slots,
};

PyRef<Angle> Angle::NewFromAngle (::Angle const &angle_) noexcept
{
	auto const self = PyRef<Angle>::stealObject (Angle::New (Angle::Type, nullptr, nullptr));
	if (!self || !InitFromAngle (self.borrow (), angle_))
		return nullptr;

	return self;
}

bool Angle::InitFromAngle (Angle *const self_, ::Angle const &angle_) noexcept
{
	self_->angle = angle_;
	return true;
}

PyObject *Angle::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<Angle>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->angle)::Angle{};

	return self.giftObject ();
}

int Angle::Init (Angle *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	::Angle angle{};
	if (!PyArg_ParseTuple (args_, "|fff", &angle.yaw, &angle.pitch, &angle.roll))
		return -1;

	if (!InitFromAngle (self_, angle))
		return -1;

	return 0;
}

void Angle::Dealloc (Angle *self_) noexcept
{
	self_->angle.~Angle ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Angle::Repr (Angle *self_) noexcept
{
	auto const tp_repr = (reprfunc)PyType_GetSlot (&PyTuple_Type, Py_tp_repr);
	if (!tp_repr)
		return nullptr;

	auto const tuple =
	    PyObjectRef::steal (Py_BuildValue ("fff", self_->angle.yaw, self_->angle.pitch, self_->angle.roll));
	if (!tuple)
		return nullptr;

	return tp_repr (tuple.borrow ());
}

PyObject *Angle::Format (Angle *self_, PyObject *args_) noexcept
{
	auto tp_methods = (PyMethodDef *)PyType_GetSlot (&PyFloat_Type, Py_tp_methods);
	if (!tp_methods)
		return nullptr;

	PyCFunction format = nullptr;
	while (tp_methods->ml_name)
	{
		if (std::strcmp (tp_methods->ml_name, "__format__") == 0)
		{
			format = tp_methods->ml_meth;
			break;
		}

		++tp_methods;
	}

	if (!format)
		return nullptr;

	PyObject *spec; // borrowed reference
	if (!PyArg_ParseTuple (args_, "O!", &PyUnicode_Type, &spec))
		return nullptr;

	auto const yaw = PyObjectRef::steal (PyFloat_FromDouble (self_->angle.yaw));
	if (!yaw)
		return nullptr;

	auto const yawString = PyObjectRef::steal (format (yaw.borrow (), spec));
	if (!yawString)
		return nullptr;

	auto const pitch = PyObjectRef::steal (PyFloat_FromDouble (self_->angle.pitch));
	if (!pitch)
		return nullptr;

	auto const pitchString = PyObjectRef::steal (format (pitch.borrow (), spec));
	if (!pitchString)
		return nullptr;

	auto const roll = PyObjectRef::steal (PyFloat_FromDouble (self_->angle.roll));
	if (!roll)
		return nullptr;

	auto const rollString = PyObjectRef::steal (format (roll.borrow (), spec));
	if (!rollString)
		return nullptr;

	return PyUnicode_FromFormat ("(%S, %S, %S)", yawString.borrow (), pitchString.borrow (), rollString.borrow ());
}

PyObject *Angle::AsTuple (Angle *self_) noexcept
{
	return Py_BuildValue ("fff", self_->angle.yaw, self_->angle.pitch, self_->angle.yaw);
}

PyObject *Angle::AsNumpy (Angle *self_) noexcept
{
	auto array = PyArrayRef (3);
	if (!array)
		return nullptr;

	array (0) = self_->angle.yaw;
	array (1) = self_->angle.pitch;
	array (2) = self_->angle.roll;

	return array.giftObject ();
}
}
