#include "Module.h"

#include "RocketSim.h"

#include <exception>

namespace
{
// No data races due to GIL
bool inited = false;

RocketSim::Python::PyObjectRef CopyModuleObj;
RocketSim::Python::PyObjectRef DeepCopyObj;

bool InitDeepCopy () noexcept
{
	if (DeepCopyObj)
		return true;

	CopyModuleObj = RocketSim::Python::PyObjectRef::steal (PyImport_ImportModule ("copy"));
	if (!CopyModuleObj)
		return false;

	DeepCopyObj = RocketSim::Python::PyObjectRef::steal (PyObject_GetAttrString (CopyModuleObj.borrow (), "deepcopy"));
	if (!DeepCopyObj)
		return false;

	if (!PyCallable_Check (DeepCopyObj.borrow ()))
	{
		DeepCopyObj = {};
		PyErr_SetString (PyExc_ImportError, "Failed to import copy.deepcopy");
		return false;
	}

	return true;
}
}

namespace RocketSim::Python
{
void InitInternal (char const *path_)
{
	if (inited)
		return;

	if (!path_)
		path_ = std::getenv ("RS_COLLISION_MESHES");

	RocketSim::Init (path_ ? path_ : COLLISION_MESH_BASE_PATH);

	inited = true;
}

bool DictSetValue (PyObject *dict_, char const *key_, PyObject *value_) noexcept
{
	if (!value_)
		return false;

	auto const success = (PyDict_SetItemString (dict_, key_, value_) == 0);
	Py_DECREF (value_);

	return success;
}

PyObject *PyDeepCopy (void *obj_, PyObject *memo_) noexcept
{
	if (!InitDeepCopy ())
		return nullptr;

	auto args = PyObjectRef::steal (PyTuple_Pack (2, obj_, memo_));
	if (!args)
		return nullptr;

	return PyObject_Call (DeepCopyObj.borrow (), args.borrow (), nullptr);
}
}

namespace
{
PyObject *Init (PyObject *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (inited)
	{
		PyErr_SetString (PyExc_RuntimeError, "Already inited");
		return nullptr;
	}

	static char pathKwd[] = "path";

	static char *dict[] = {pathKwd, nullptr};

	char const *path = nullptr;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|s", dict, &path))
		return nullptr;

	try
	{
		RocketSim::Python::InitInternal (path);
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return nullptr;
	}

	Py_RETURN_NONE;
}

void Free (void *)
{
	CopyModuleObj = nullptr;
	DeepCopyObj   = nullptr;
}

struct PyMethodDef Methods[] = {
    {.ml_name     = "init",
        .ml_meth  = (PyCFunction)&Init,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(init(path: str = os.getenv("RS_COLLISION_MESHES", "collision_meshes"))"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

struct PyModuleDef Module = {
    // clang-format off
    .m_base    = PyModuleDef_HEAD_INIT,
    .m_name    = "RocketSim",
    .m_doc     = R"(This is Rocket League!)",
    .m_size    = -1,
    .m_methods = Methods,
    .m_free    = &Free,
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
	PyEval_InitThreads ();

	auto m = RocketSim::Python::PyObjectRef::steal (PyModule_Create (&Module));
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
	MAKE_TYPE (MemoryWeightMode);
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
		    "HOOPS",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::GameMode::HOOPS))));
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "HEATSEEKER",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::GameMode::HEATSEEKER))));
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "SNOWDAY",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (::GameMode::SNOWDAY))));
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

		// MemoryWeightMode
		SET_TYPE_ATTR (RocketSim::Python::MemoryWeightMode::Type,
		    "HEAVY",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (ArenaMemWeightMode::HEAVY))));
		SET_TYPE_ATTR (RocketSim::Python::MemoryWeightMode::Type,
		    "LIGHT",
		    PyObjectRef::stealObject (PyLong_FromLong (static_cast<long> (ArenaMemWeightMode::LIGHT))));

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
