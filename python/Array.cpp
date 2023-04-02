#undef NO_IMPORT_ARRAY

#include "Array.h"

namespace
{
bool importNumpy ()
{
	import_array1 (false);

	return true;
}

RocketSim::Python::PyRef<PyArrayObject> makeArray (unsigned dim0_, unsigned dim1_)
{
	static bool const imported = importNumpy ();
	if (!imported)
	{
		PyErr_Format (PyExc_ImportError, "Failed to import numpy");
		return nullptr;
	}

	npy_intp const dims[2] = {dim0_, dim1_};

	auto const array = RocketSim::Python::PyRef<PyArrayObject>::stealObject (
	    PyArray_New (&PyArray_Type, dim1_ ? 2 : 1, dims, NPY_FLOAT, nullptr, nullptr, 0, NPY_ARRAY_DEFAULT, nullptr));
	if (!array)
		return nullptr;

	PyArray_FILLWBYTE (array.borrow (), 0);

	return array;
}
}

namespace RocketSim::Python
{
PyArrayRef::PyArrayRef (nullptr_t) noexcept : PyRef<PyArrayObject> (nullptr), m_dim0 (0), m_dim1 (0)
{
}

PyArrayRef::PyArrayRef (unsigned dim0_, unsigned dim1_) noexcept
    : PyRef<PyArrayObject> (makeArray (dim0_, dim1_)), m_dim0 (dim0_), m_dim1 (dim1_)
{
}

float &PyArrayRef::operator() (unsigned dim0_, unsigned dim1_) noexcept
{
	if (m_dim1)
		return *static_cast<float *> (PyArray_GETPTR2 (borrow (), dim0_, dim1_));

	return *static_cast<float *> (PyArray_GETPTR1 (borrow (), dim0_));
}

float const &PyArrayRef::operator() (unsigned dim0_, unsigned dim1_) const noexcept
{
	if (m_dim1)
		return *static_cast<float const *> (PyArray_GETPTR2 (borrow (), dim0_, dim1_));

	return *static_cast<float const *> (PyArray_GETPTR1 (borrow (), dim0_));
}
}
