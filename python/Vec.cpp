#include "Module.h"

#include "Array.h"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <tuple>

namespace RocketSim::Python
{
PyTypeObject *Vec::Type = nullptr;

PyMemberDef Vec::Members[] = {
    {.name      = "x",
        .type   = TypeHelper<decltype (::Vec::x)>::type,
        .offset = offsetof (Vec, vec) + offsetof (::Vec, x),
        .flags  = 0,
        .doc    = "X component"},
    {.name      = "y",
        .type   = TypeHelper<decltype (::Vec::y)>::type,
        .offset = offsetof (Vec, vec) + offsetof (::Vec, y),
        .flags  = 0,
        .doc    = "Y component"},
    {.name      = "z",
        .type   = TypeHelper<decltype (::Vec::z)>::type,
        .offset = offsetof (Vec, vec) + offsetof (::Vec, z),
        .flags  = 0,
        .doc    = "Z component"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef Vec::Methods[] = {
    {.ml_name     = "round",
        .ml_meth  = (PyCFunction)&Vec::Round,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(round(self, precision: float) -> float
Round vector to desired precision)"},
    {.ml_name     = "as_tuple",
        .ml_meth  = (PyCFunction)&Vec::AsTuple,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(as_tuple(self) -> tuple
Returns (self.x, self.y, self.z))"},
    {.ml_name     = "as_numpy",
        .ml_meth  = (PyCFunction)&Vec::AsNumpy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(as_numpy(self) -> numpy.ndarray
Returns numpy.array([self.x, self.y, self.z]))"},
    {.ml_name     = "__format__",
        .ml_meth  = (PyCFunction)&Vec::Format,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = nullptr},
    {.ml_name = "__getstate__", .ml_meth = (PyCFunction)&Vec::Pickle, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&Vec::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&Vec::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.Vec
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&Vec::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.Vec
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyType_Slot Vec::Slots[] = {
    {Py_tp_new, (void *)&Vec::New},
    {Py_tp_init, (void *)&Vec::Init},
    {Py_tp_dealloc, (void *)&Vec::Dealloc},
    {Py_tp_richcompare, (void *)&Vec::RichCompare},
    {Py_tp_repr, (void *)&Vec::Repr},
    {Py_tp_members, &Vec::Members},
    {Py_tp_methods, &Vec::Methods},
    {Py_tp_doc, (void *)R"(3-dimensional vector
__init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0))"},
    {0, nullptr},
};

PyType_Spec Vec::Spec = {
    .name      = "RocketSim.Vec",
    .basicsize = sizeof (Vec),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = Vec::Slots,
};

PyRef<Vec> Vec::NewFromVec (::Vec const &vec_) noexcept
{
	auto const self = PyRef<Vec>::stealObject (Vec::New (Vec::Type, nullptr, nullptr));
	if (!self || !InitFromVec (self.borrow (), vec_))
		return nullptr;

	return self;
}

bool Vec::InitFromVec (Vec *const self_, ::Vec const &vec_) noexcept
{
	self_->vec = vec_;
	return true;
}

::Vec Vec::ToVec (Vec *self_) noexcept
{
	return self_->vec;
}

PyObject *Vec::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<Vec>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->vec)::Vec{};

	return self.giftObject ();
}

int Vec::Init (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char xKwd[]  = "x";
	static char yKwd[]  = "y";
	static char zKwd[]  = "z";
	static char *dict[] = {xKwd, yKwd, zKwd, nullptr};

	::Vec vec{};
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|fff", dict, &vec.x, &vec.y, &vec.z))
		return -1;

	if (!InitFromVec (self_, vec))
		return -1;

	return 0;
}

void Vec::Dealloc (Vec *self_) noexcept
{
	self_->vec.~Vec ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Vec::RichCompare (Vec *self_, PyObject *other_, int op_) noexcept
{
	if (!Py_IS_TYPE (other_, Vec::Type))
		return Py_NotImplemented;

	auto const vec = PyCast<Vec> (other_);

	if (std::isnan (self_->vec.x) || std::isnan (self_->vec.y) || std::isnan (self_->vec.z) ||
	    std::isnan (vec->vec.x) || std::isnan (vec->vec.y) || std::isnan (vec->vec.z))
		return PyBool_FromLong (op_ == Py_NE);

	auto const a = std::make_tuple (self_->vec.x, self_->vec.y, self_->vec.z);
	auto const b = std::make_tuple (vec->vec.x, vec->vec.y, vec->vec.z);

	Py_RETURN_RICHCOMPARE (a, b, op_);
}

PyObject *Vec::Repr (Vec *self_) noexcept
{
	auto const tuple = PyObjectRef::steal (AsTuple (self_));
	if (!tuple)
		return nullptr;

	return PyObject_Repr (tuple.borrow ());
}

PyObject *Vec::Format (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto format = PyObject_GetAttrString (reinterpret_cast<PyObject *> (&PyFloat_Type), "__format__");
	if (!format)
		return nullptr;

	if (!PyCallable_Check (format))
	{
		PyErr_SetString (PyExc_TypeError, "float.__format__ is not callable");
		return nullptr;
	}

	static char specKwd[] = "format_spec";

	static char *dict[] = {specKwd, nullptr};

	PyObject *spec; // borrowed reference
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O!", dict, &PyUnicode_Type, &spec))
		return nullptr;

	auto const applyFormat = [&] (float x_) -> PyObjectRef {
		auto const value = PyObjectRef::steal (PyFloat_FromDouble (x_));
		if (!value)
			return nullptr;

		return PyObjectRef::steal (PyObject_CallFunctionObjArgs (format, value.borrow (), spec, nullptr));
	};

	auto const x = applyFormat (self_->vec.x);
	auto const y = applyFormat (self_->vec.y);
	auto const z = applyFormat (self_->vec.z);
	if (!x || !y || !z)
		return nullptr;

	return PyUnicode_FromFormat ("(%S, %S, %S)", x.borrow (), y.borrow (), z.borrow ());
}

PyObject *Vec::Pickle (Vec *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	::Vec const model{};
	auto const vec = ToVec (self_);

	if (vec.x != model.x && !DictSetValue (dict.borrow (), "x", PyFloat_FromDouble (vec.x)))
		return nullptr;

	if (vec.y != model.y && !DictSetValue (dict.borrow (), "y", PyFloat_FromDouble (vec.y)))
		return nullptr;

	if (vec.z != model.z && !DictSetValue (dict.borrow (), "z", PyFloat_FromDouble (vec.z)))
		return nullptr;

	return dict.gift ();
}

PyObject *Vec::Unpickle (Vec *self_, PyObject *dict_) noexcept
{
	auto const args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
		return nullptr;

	if (Init (self_, args.borrow (), dict_) != 0)
		return nullptr;

	Py_RETURN_NONE;
}

PyObject *Vec::Copy (Vec *self_) noexcept
{
	return NewFromVec (self_->vec).giftObject ();
}

PyObject *Vec::DeepCopy (Vec *self_, PyObject *memo_) noexcept
{
	return NewFromVec (self_->vec).giftObject ();
}

PyObject *Vec::AsTuple (Vec *self_) noexcept
{
	return Py_BuildValue ("fff", self_->vec.x, self_->vec.y, self_->vec.z);
}

PyObject *Vec::AsNumpy (Vec *self_) noexcept
{
	auto array = PyArrayRef (3);
	if (!array)
		return nullptr;

	array (0) = self_->vec.x;
	array (1) = self_->vec.y;
	array (2) = self_->vec.z;

	return array.giftObject ();
}

PyObject *Vec::Round (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char precisionKwd[] = "precision";

	static char *dict[] = {precisionKwd, nullptr};

	float precision;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "f", dict, &precision))
		return nullptr;

	auto vec = Vec::NewFromVec (Math::RoundVec (self_->vec, precision));
	if (!vec)
		return nullptr;

	return vec.giftObject ();
}
}
