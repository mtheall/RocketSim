#include <Python.h>

namespace RocketSim::Python
{
/// \brief PyObject reference wrapper
template <typename T>
class PyRef
{
public:
	/// \brief Steal reference
	/// \param obj_ Object to steal
	static PyRef stealObject (PyObject *obj_) noexcept
	{
		return PyRef (reinterpret_cast<T *> (obj_));
	}

	/// \brief Steal reference
	/// \param obj_ Object to steal
	static PyRef steal (T *obj_) noexcept
	{
		return PyRef (obj_);
	}

	/// \brief Increment reference
	/// \param obj_ Object to reference
	static PyRef incObjectRef (PyObject *obj_) noexcept
	{
		return incRef (reinterpret_cast<T *> (obj_));
	}

	/// \brief Increment reference
	/// \param obj_ Object to reference
	static PyRef incRef (T *obj_) noexcept
	{
		Py_XINCREF (obj_);
		return PyRef (obj_);
	}

	/// \brief Assign new reference
	/// \note decrements old_ and increments new_
	static void assign (T *&old_, PyObject *new_) noexcept
	{
		if (old_ != reinterpret_cast<T *> (new_))
		{
			Py_XINCREF (new_);
			Py_XDECREF (old_);
		}

		old_ = reinterpret_cast<T *> (new_);
	}

	~PyRef () noexcept
	{
		Py_XDECREF (m_object);
	}

	PyRef () noexcept = default;

	PyRef (nullptr_t) noexcept : PyRef ()
	{
	}

	PyRef (PyRef const &that_) noexcept : m_object (that_.m_object)
	{
		Py_XINCREF (m_object);
	}

	PyRef (PyRef &&that_) noexcept : m_object (that_.m_object)
	{
		// stolen reference
		that_.m_object = nullptr;
	}

	PyRef &operator= (PyRef const &that_) noexcept
	{
		if (this != &that_)
		{
			// increment first in case they point to the same object
			Py_XINCREF (that_.m_object);
			Py_XDECREF (m_object);
			m_object = that_.m_object;
		}

		return *this;
	}

	PyRef &operator= (PyRef &&that_) noexcept
	{
		if (this != &that_)
		{
			// stolen reference
			Py_XDECREF (m_object);
			m_object       = that_.m_object;
			that_.m_object = nullptr;
		}

		return *this;
	}

	explicit operator bool () const noexcept
	{
		return m_object;
	}

	T *operator->() noexcept
	{
		return m_object;
	}

	T const *operator->() const noexcept
	{
		return m_object;
	}

	/// \brief Relinquish reference
	T *gift () noexcept
	{
		auto const tmp = m_object;

		m_object = nullptr;

		return tmp;
	}

	/// \brief Relinquish reference as PyObject
	PyObject *giftObject () noexcept
	{
		return reinterpret_cast<PyObject *> (gift ());
	}

	/// \brief Borrow reference
	T *borrow () const noexcept
	{
		return m_object;
	}

	/// \brief Borrow reference as PyObject
	PyObject *borrowObject () const noexcept
	{
		return reinterpret_cast<PyObject *> (m_object);
	}

	/// \brief Return new reference
	T *newRef () const noexcept
	{
		Py_XINCREF (m_object);
		return m_object;
	}

	/// \brief Return new reference as PyObject
	PyObject *newObjectRef () const noexcept
	{
		return reinterpret_cast<PyObject *> (newRef ());
	}

private:
	explicit PyRef (T *object_) noexcept : m_object (object_)
	{
		// stolen reference
	}

	T *m_object = nullptr;
};

using PyObjectRef = PyRef<PyObject>;
using PyTypeRef   = PyRef<PyTypeObject>;
}
