#include "Module.h"

#include "RocketSim.h"

namespace
{
// No data races due to GIL
bool inited = false;
}

namespace RocketSim::Python
{
void InitInternal (char const *path_) noexcept
{
	if (inited)
		return;

	if (!path_)
		path_ = std::getenv ("RS_COLLISION_MESHES");

	RocketSim::Init (path_ ? path_ : COLLISION_MESH_BASE_PATH);
}

PyObject *Init (PyObject *self_, PyObject *args_) noexcept
{
	if (inited)
		return PyErr_Format (PyExc_RuntimeError, "Already inited");

	char const *path = nullptr;
	if (!PyArg_ParseTuple (args_, "|s", &path))
		return nullptr;

	InitInternal (path);

	inited = true;

	Py_RETURN_NONE;
}

struct PyMethodDef Methods[] = {
    {.ml_name = "init", .ml_meth = (PyCFunction)&Init, .ml_flags = METH_VARARGS, .ml_doc = nullptr},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

struct PyModuleDef Module = {
    // clang-format off
    .m_base    = PyModuleDef_HEAD_INIT,
    .m_name    = "RocketSim",
    .m_size    = -1,
    .m_methods = Methods,
    // clang-format on
};
}

#ifndef Py_EXPORTED_SYMBOL
#if defined(_WIN32) || defined(__CYGWIN__)
#define Py_EXPORTED_SYMBOL __declspec (dllexport)
#elif defined(__GNUC__)
#define Py_EXPORTED_SYMBOL __attribute__ ((visibility ("default")))
#else
#define Py_EXPORTED_SYMBOL
#endif
#endif

extern "C" Py_EXPORTED_SYMBOL PyObject *PyInit_RocketSim () noexcept
{
	auto m = RocketSim::Python::PyObjectRef::steal (PyModule_Create (&RocketSim::Python::Module));
	if (!m)
		return nullptr;

#define MAKE_TYPE(x_)                                                                                                  \
	do                                                                                                                 \
	{                                                                                                                  \
		auto type = RocketSim::Python::PyTypeRef::stealObject (PyType_FromSpec (&RocketSim::Python::x_::Spec));        \
		if (!type)                                                                                                     \
			return nullptr;                                                                                            \
		if (PyModule_AddObject (m.borrow (), #x_, type.borrowObject ()) < 0) /* careful! steals ref on success */      \
			return nullptr;                                                                                            \
		RocketSim::Python::x_::Type = type.gift (); /* ref belongs to module now */                                    \
	} while (0)

	MAKE_TYPE (Angle);
	MAKE_TYPE (Arena);
	MAKE_TYPE (Ball);
	MAKE_TYPE (BallHitInfo);
	MAKE_TYPE (BallState);
	MAKE_TYPE (BoostPad);
	MAKE_TYPE (BoostPadState);
	MAKE_TYPE (Car);
	MAKE_TYPE (CarConfig);
	MAKE_TYPE (CarControls);
	MAKE_TYPE (CarState);
	MAKE_TYPE (DemoMode);
	MAKE_TYPE (GameMode);
	MAKE_TYPE (MutatorConfig);
	MAKE_TYPE (RotMat);
	MAKE_TYPE (Team);
	MAKE_TYPE (Vec);
	MAKE_TYPE (WheelPairConfig);

#define SET_TYPE_ATTR(type_, name_, value_)                                                                            \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!value_)                                                                                                   \
			return nullptr;                                                                                            \
		if (PyObject_SetAttrString ((PyObject *)type_, name_, value_.borrow ()) < 0)                                   \
			return nullptr;                                                                                            \
	} while (0)

	{
		using RocketSim::Python::PyObjectRef;

		// GameMode
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "SOCCAR",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::GameMode::SOCCAR))));
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "THE_VOID",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::GameMode::THE_VOID))));

		// Team
		SET_TYPE_ATTR (RocketSim::Python::Team::Type,
		    "BLUE",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::Team::BLUE))));
		SET_TYPE_ATTR (RocketSim::Python::Team::Type,
		    "ORANGE",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::Team::ORANGE))));

		// DemoMode
		SET_TYPE_ATTR (RocketSim::Python::DemoMode::Type,
		    "NORMAL",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::DemoMode::NORMAL))));
		SET_TYPE_ATTR (RocketSim::Python::DemoMode::Type,
		    "ON_CONTACT",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::DemoMode::ON_CONTACT))));
		SET_TYPE_ATTR (RocketSim::Python::DemoMode::Type,
		    "DISABLED",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::DemoMode::DISABLED))));

		// CarConfig
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "OCTANE",
		    PyObjectRef::stealObject (
		        PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::OCTANE))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "DOMINUS",
		    PyObjectRef::stealObject (
		        PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::DOMINUS))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "PLANK",
		    PyObjectRef::stealObject (
		        PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::PLANK))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "BREAKOUT",
		    PyObjectRef::stealObject (
		        PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::BREAKOUT))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "HYBRID",
		    PyObjectRef::stealObject (
		        PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::HYBRID))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "MERC",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::MERC))));
	}

	return m.gift ();
}
