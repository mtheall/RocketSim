#include "Module.h"

#include "RocketSim.h"

#include <concepts>
#include <exception>
#include <span>

namespace
{
// No data races due to GIL
bool inited = false;

PyObject *CopyModuleObj = nullptr;
PyObject *DeepCopyObj   = nullptr;

bool InitDeepCopy () noexcept
{
	if (DeepCopyObj)
		return true;

	CopyModuleObj = PyImport_ImportModule ("copy");
	if (!CopyModuleObj)
		return false;

	DeepCopyObj = PyObject_GetAttrString (CopyModuleObj, "deepcopy");
	if (!DeepCopyObj)
	{
		Py_DECREF (CopyModuleObj);
		CopyModuleObj = nullptr;

		return false;
	}

	if (!PyCallable_Check (DeepCopyObj))
	{
		Py_DECREF (DeepCopyObj);
		DeepCopyObj = nullptr;

		Py_DECREF (CopyModuleObj);
		CopyModuleObj = nullptr;

		PyErr_SetString (PyExc_ImportError, "Failed to import copy.deepcopy");
		return false;
	}

	return true;
}

template <typename T>
RocketSim::Python::PyObjectRef BuildObject (T const &input_) noexcept;

template <std::unsigned_integral T>
RocketSim::Python::PyObjectRef BuildObject (T const &input_) noexcept
{
	return RocketSim::Python::PyObjectRef::steal (
	    PyLong_FromUnsignedLongLong (static_cast<unsigned long long> (input_)));
}

template <std::signed_integral T>
RocketSim::Python::PyObjectRef BuildObject (T const &input_) noexcept
{
	return RocketSim::Python::PyObjectRef::steal (PyLong_FromLongLong (static_cast<long long> (input_)));
}

template <std::floating_point T>
RocketSim::Python::PyObjectRef BuildObject (T const &input_) noexcept
{
	return RocketSim::Python::PyObjectRef::steal (PyFloat_FromDouble (static_cast<double> (input_)));
}

template <>
RocketSim::Python::PyObjectRef BuildObject (RocketSim::Vec const &input_) noexcept
{
	return RocketSim::Python::PyObjectRef::steal (RocketSim::Python::Vec::NewFromVec (input_).giftObject ());
}

template <>
RocketSim::Python::PyObjectRef BuildObject (RocketSim::RLConst::CarSpawnPos const &input_) noexcept
{
	return RocketSim::Python::PyObjectRef::steal (Py_BuildValue ("fff", input_.x, input_.y, input_.yawAng));
}

template <>
RocketSim::Python::PyObjectRef BuildObject (RocketSim::LinearPieceCurve const &input_) noexcept
{
	auto dict = RocketSim::Python::PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	for (auto const &[key, value] : input_.valueMappings)
	{
		auto const k = RocketSim::Python::PyObjectRef::steal (PyFloat_FromDouble (key));
		auto const v = RocketSim::Python::PyObjectRef::steal (PyFloat_FromDouble (value));

		if (!k || !v)
			return nullptr;

		if (PyDict_SetItem (dict.borrow (), k.borrow (), v.borrow ()) < 0)
			return nullptr;
	}

	return dict;
}

template <typename T, std::size_t N>
RocketSim::Python::PyObjectRef BuildList (std::span<T, N> input_) noexcept
{
	auto list = RocketSim::Python::PyObjectRef::steal (PyList_New (input_.size ()));
	if (!list)
		return nullptr;

	for (unsigned i = 0; i < input_.size (); ++i)
	{
		auto object = BuildObject (input_[i]);
		if (!object)
			return nullptr;

		if (PyList_SetItem (list.borrow (), i, object.gift ()) < 0)
			return nullptr;
	}

	return list;
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

	RocketSim::Init (path_ ? path_ : COLLISION_MESH_BASE_PATH, true);

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

	return PyObject_Call (DeepCopyObj, args.borrow (), nullptr);
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
	Py_XDECREF (DeepCopyObj);
	DeepCopyObj = nullptr;

	Py_XDECREF (CopyModuleObj);
	CopyModuleObj = nullptr;
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
	MAKE_TYPE (ArenaConfig);
	MAKE_TYPE (Ball);
	MAKE_TYPE (BallHitInfo);
	MAKE_TYPE (BallPredictor);
	MAKE_TYPE (BallState);
	MAKE_TYPE (BoostPad);
	MAKE_TYPE (BoostPadConfig);
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
		if (PyObject_SetAttrString ((PyObject *)type_, name_, value_.borrowObject ()) < 0)                             \
			return nullptr;                                                                                            \
	} while (0)

	{
		using RocketSim::Python::PyObjectRef;
		using RocketSim::Python::PyRef;
		using RocketSim::Python::PyTypeRef;

		// GameMode
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "SOCCAR",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::GameMode::SOCCAR))));
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "HOOPS",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::GameMode::HOOPS))));
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "HEATSEEKER",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::GameMode::HEATSEEKER))));
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "SNOWDAY",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::GameMode::SNOWDAY))));
		SET_TYPE_ATTR (RocketSim::Python::GameMode::Type,
		    "THE_VOID",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::GameMode::THE_VOID))));

		// Team
		SET_TYPE_ATTR (RocketSim::Python::Team::Type,
		    "BLUE",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Team::BLUE))));
		SET_TYPE_ATTR (RocketSim::Python::Team::Type,
		    "ORANGE",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Team::ORANGE))));

		// DemoMode
		SET_TYPE_ATTR (RocketSim::Python::DemoMode::Type,
		    "NORMAL",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::DemoMode::NORMAL))));
		SET_TYPE_ATTR (RocketSim::Python::DemoMode::Type,
		    "ON_CONTACT",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::DemoMode::ON_CONTACT))));
		SET_TYPE_ATTR (RocketSim::Python::DemoMode::Type,
		    "DISABLED",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::DemoMode::DISABLED))));

		// MemoryWeightMode
		SET_TYPE_ATTR (RocketSim::Python::MemoryWeightMode::Type,
		    "HEAVY",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::ArenaMemWeightMode::HEAVY))));
		SET_TYPE_ATTR (RocketSim::Python::MemoryWeightMode::Type,
		    "LIGHT",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::ArenaMemWeightMode::LIGHT))));

		// CarConfig
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "OCTANE",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::OCTANE))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "DOMINUS",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::DOMINUS))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "PLANK",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::PLANK))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "BREAKOUT",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::BREAKOUT))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "HYBRID",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::HYBRID))));
		SET_TYPE_ATTR (RocketSim::Python::CarConfig::Type,
		    "MERC",
		    PyObjectRef::steal (PyLong_FromLong (static_cast<long> (RocketSim::Python::CarConfig::Index::MERC))));

		auto emptyTuple = PyObjectRef::steal (PyTuple_New (0));
		if (!emptyTuple)
			return nullptr;

		auto typesModule = PyObjectRef::steal (PyImport_ImportModule ("types"));
		if (!typesModule)
			return nullptr;

		auto namespaceType = PyTypeRef::stealObject (PyObject_GetAttrString (typesModule.borrow (), "SimpleNamespace"));
		if (!namespaceType)
			return nullptr;

		// RLConst
		auto rlConst = PyObjectRef::steal (((newfunc)PyType_GetSlot (namespaceType.borrow (), Py_tp_alloc)) (
		    namespaceType.borrow (), emptyTuple.borrow (), nullptr));

#define ATTR_OBJECT(parent_, ns_, x_) SET_TYPE_ATTR (parent_.borrow (), #x_, BuildObject (RocketSim::ns_::x_))
#define ATTR_LIST(parent_, ns_, x_) SET_TYPE_ATTR (parent_.borrow (), #x_, BuildList (std::span (RocketSim::ns_::x_)))

		ATTR_OBJECT (rlConst, RLConst, GRAVITY_Z);
		ATTR_OBJECT (rlConst, RLConst, ARENA_EXTENT_X);
		ATTR_OBJECT (rlConst, RLConst, ARENA_EXTENT_Y);
		ATTR_OBJECT (rlConst, RLConst, ARENA_HEIGHT);
		ATTR_OBJECT (rlConst, RLConst, ARENA_EXTENT_X_HOOPS);
		ATTR_OBJECT (rlConst, RLConst, ARENA_EXTENT_Y_HOOPS);
		ATTR_OBJECT (rlConst, RLConst, ARENA_HEIGHT_HOOPS);
		ATTR_OBJECT (rlConst, RLConst, CAR_MASS_BT);
		ATTR_OBJECT (rlConst, RLConst, BALL_MASS_BT);
		ATTR_OBJECT (rlConst, RLConst, CAR_COLLISION_FRICTION);
		ATTR_OBJECT (rlConst, RLConst, CAR_COLLISION_RESTITUTION);
		ATTR_OBJECT (rlConst, RLConst, CARBALL_COLLISION_FRICTION);
		ATTR_OBJECT (rlConst, RLConst, CARBALL_COLLISION_RESTITUTION);
		ATTR_OBJECT (rlConst, RLConst, CARWORLD_COLLISION_FRICTION);
		ATTR_OBJECT (rlConst, RLConst, CARWORLD_COLLISION_RESTITUTION);
		ATTR_OBJECT (rlConst, RLConst, CARCAR_COLLISION_FRICTION);
		ATTR_OBJECT (rlConst, RLConst, CARCAR_COLLISION_RESTITUTION);
		ATTR_OBJECT (rlConst, RLConst, BALL_REST_Z);
		ATTR_OBJECT (rlConst, RLConst, BALL_MAX_ANG_SPEED);
		ATTR_OBJECT (rlConst, RLConst, BALL_DRAG);
		ATTR_OBJECT (rlConst, RLConst, BALL_FRICTION);
		ATTR_OBJECT (rlConst, RLConst, BALL_RESTITUTION);
		ATTR_OBJECT (rlConst, RLConst, BALL_HOOPS_Z_VEL);
		ATTR_OBJECT (rlConst, RLConst, CAR_MAX_SPEED);
		ATTR_OBJECT (rlConst, RLConst, BALL_MAX_SPEED);
		ATTR_OBJECT (rlConst, RLConst, BOOST_MAX);
		ATTR_OBJECT (rlConst, RLConst, BOOST_USED_PER_SECOND);
		ATTR_OBJECT (rlConst, RLConst, BOOST_MIN_TIME);
		ATTR_OBJECT (rlConst, RLConst, BOOST_ACCEL_GROUND);
		ATTR_OBJECT (rlConst, RLConst, BOOST_ACCEL_AIR);
		ATTR_OBJECT (rlConst, RLConst, BOOST_SPAWN_AMOUNT);
		ATTR_OBJECT (rlConst, RLConst, CAR_MAX_ANG_SPEED);
		ATTR_OBJECT (rlConst, RLConst, SUPERSONIC_START_SPEED);
		ATTR_OBJECT (rlConst, RLConst, SUPERSONIC_MAINTAIN_MIN_SPEED);
		ATTR_OBJECT (rlConst, RLConst, SUPERSONIC_MAINTAIN_MAX_TIME);
		ATTR_OBJECT (rlConst, RLConst, POWERSLIDE_RISE_RATE);
		ATTR_OBJECT (rlConst, RLConst, POWERSLIDE_FALL_RATE);
		ATTR_OBJECT (rlConst, RLConst, THROTTLE_TORQUE_AMOUNT);
		ATTR_OBJECT (rlConst, RLConst, BRAKE_TORQUE_AMOUNT);
		ATTR_OBJECT (rlConst, RLConst, STOPPING_FORWARD_VEL);
		ATTR_OBJECT (rlConst, RLConst, COASTING_BRAKE_FACTOR);
		ATTR_OBJECT (rlConst, RLConst, BRAKING_NO_THROTTLE_SPEED_THRESH);
		ATTR_OBJECT (rlConst, RLConst, THROTTLE_DEADZONE);
		ATTR_OBJECT (rlConst, RLConst, THROTTLE_AIR_ACCEL);
		ATTR_OBJECT (rlConst, RLConst, JUMP_ACCEL);
		ATTR_OBJECT (rlConst, RLConst, JUMP_IMMEDIATE_FORCE);
		ATTR_OBJECT (rlConst, RLConst, JUMP_MIN_TIME);
		ATTR_OBJECT (rlConst, RLConst, JUMP_RESET_TIME_PAD);
		ATTR_OBJECT (rlConst, RLConst, JUMP_MAX_TIME);
		ATTR_OBJECT (rlConst, RLConst, DOUBLEJUMP_MAX_DELAY);
		ATTR_OBJECT (rlConst, RLConst, FLIP_Z_DAMP_120);
		ATTR_OBJECT (rlConst, RLConst, FLIP_Z_DAMP_START);
		ATTR_OBJECT (rlConst, RLConst, FLIP_Z_DAMP_END);
		ATTR_OBJECT (rlConst, RLConst, FLIP_TORQUE_TIME);
		ATTR_OBJECT (rlConst, RLConst, FLIP_TORQUE_MIN_TIME);
		ATTR_OBJECT (rlConst, RLConst, FLIP_PITCHLOCK_TIME);
		ATTR_OBJECT (rlConst, RLConst, FLIP_PITCHLOCK_EXTRA_TIME);
		ATTR_OBJECT (rlConst, RLConst, FLIP_INITIAL_VEL_SCALE);
		ATTR_OBJECT (rlConst, RLConst, FLIP_TORQUE_X);
		ATTR_OBJECT (rlConst, RLConst, FLIP_TORQUE_Y);
		ATTR_OBJECT (rlConst, RLConst, FLIP_FORWARD_IMPULSE_MAX_SPEED_SCALE);
		ATTR_OBJECT (rlConst, RLConst, FLIP_SIDE_IMPULSE_MAX_SPEED_SCALE);
		ATTR_OBJECT (rlConst, RLConst, FLIP_BACKWARD_IMPULSE_MAX_SPEED_SCALE);
		ATTR_OBJECT (rlConst, RLConst, FLIP_BACKWARD_IMPULSE_SCALE_X);
		ATTR_OBJECT (rlConst, RLConst, BALL_COLLISION_RADIUS_SOCCAR);
		ATTR_OBJECT (rlConst, RLConst, BALL_COLLISION_RADIUS_HOOPS);
		ATTR_OBJECT (rlConst, RLConst, BALL_COLLISION_RADIUS_DROPSHOT);
		ATTR_OBJECT (rlConst, RLConst, SOCCAR_GOAL_SCORE_BASE_THRESHOLD_Y);
		ATTR_OBJECT (rlConst, RLConst, HOOPS_GOAL_SCORE_THRESHOLD_Z);
		ATTR_OBJECT (rlConst, RLConst, CAR_TORQUE_SCALE);
		ATTR_OBJECT (rlConst, RLConst, CAR_AUTOFLIP_IMPULSE);
		ATTR_OBJECT (rlConst, RLConst, CAR_AUTOFLIP_TORQUE);
		ATTR_OBJECT (rlConst, RLConst, CAR_AUTOFLIP_TIME);
		ATTR_OBJECT (rlConst, RLConst, CAR_AUTOFLIP_NORMZ_THRESH);
		ATTR_OBJECT (rlConst, RLConst, CAR_AUTOFLIP_ROLL_THRESH);
		ATTR_OBJECT (rlConst, RLConst, CAR_AUTOROLL_FORCE);
		ATTR_OBJECT (rlConst, RLConst, CAR_AUTOROLL_TORQUE);
		ATTR_OBJECT (rlConst, RLConst, BALL_CAR_EXTRA_IMPULSE_Z_SCALE);
		ATTR_OBJECT (rlConst, RLConst, BALL_CAR_EXTRA_IMPULSE_Z_SCALE_HOOPS_GROUND);
		ATTR_OBJECT (rlConst, RLConst, BALL_CAR_EXTRA_IMPULSE_FORWARD_SCALE);
		ATTR_OBJECT (rlConst, RLConst, BALL_CAR_EXTRA_IMPULSE_MAXDELTAVEL_UU);
		ATTR_OBJECT (rlConst, RLConst, BALL_CAR_EXTRA_IMPULSE_Z_SCALE_HOOPS_NORMAL_Z_THRESH);
		ATTR_OBJECT (rlConst, RLConst, CAR_SPAWN_REST_Z);
		ATTR_OBJECT (rlConst, RLConst, CAR_RESPAWN_Z);
		ATTR_OBJECT (rlConst, RLConst, BUMP_COOLDOWN_TIME);
		ATTR_OBJECT (rlConst, RLConst, BUMP_MIN_FORWARD_DIST);
		ATTR_OBJECT (rlConst, RLConst, DEMO_RESPAWN_TIME);

		{
			// BTVehicle
			auto btVehicle = PyObjectRef::steal (((newfunc)PyType_GetSlot (namespaceType.borrow (), Py_tp_alloc)) (
			    namespaceType.borrow (), emptyTuple.borrow (), nullptr));

			ATTR_OBJECT (btVehicle, RLConst::BTVehicle, SUSPENSION_FORCE_SCALE_FRONT);
			ATTR_OBJECT (btVehicle, RLConst::BTVehicle, SUSPENSION_FORCE_SCALE_BACK);
			ATTR_OBJECT (btVehicle, RLConst::BTVehicle, SUSPENSION_STIFFNESS);
			ATTR_OBJECT (btVehicle, RLConst::BTVehicle, WHEELS_DAMPING_COMPRESSION);
			ATTR_OBJECT (btVehicle, RLConst::BTVehicle, WHEELS_DAMPING_RELAXATION);
			ATTR_OBJECT (btVehicle, RLConst::BTVehicle, MAX_SUSPENSION_TRAVEL);
			ATTR_OBJECT (btVehicle, RLConst::BTVehicle, SUSPENSION_SUBTRACTION);

			SET_TYPE_ATTR (rlConst.borrow (), "BTVehicle", btVehicle);
		}

		{
			// Heatseeker
			auto heatseeker = PyObjectRef::steal (((newfunc)PyType_GetSlot (namespaceType.borrow (), Py_tp_alloc)) (
			    namespaceType.borrow (), emptyTuple.borrow (), nullptr));

			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, INITIAL_TARGET_SPEED);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, TARGET_SPEED_INCREMENT);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, MIN_SPEEDUP_INTERVAL);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, TARGET_Y);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, TARGET_Z);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, HORIZONTAL_BLEND);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, VERTICAL_BLEND);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, SPEED_BLEND);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, MAX_TURN_PITCH);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, MAX_SPEED);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, WALL_BOUNCE_CHANGE_Y_THRESH);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, WALL_BOUNCE_CHANGE_Y_NORMAL);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, WALL_BOUNCE_FORCE_SCALE);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, WALL_BOUNCE_UP_FRAC);
			ATTR_OBJECT (heatseeker, RLConst::Heatseeker, BALL_START_POS);

			SET_TYPE_ATTR (rlConst.borrow (), "Heatseeker", heatseeker);
		}

		{
			// Snowday
			auto snowday = PyObjectRef::steal (((newfunc)PyType_GetSlot (namespaceType.borrow (), Py_tp_alloc)) (
			    namespaceType.borrow (), emptyTuple.borrow (), nullptr));

			ATTR_OBJECT (snowday, RLConst::Snowday, PUCK_RADIUS);
			ATTR_OBJECT (snowday, RLConst::Snowday, PUCK_HEIGHT);
			ATTR_OBJECT (snowday, RLConst::Snowday, PUCK_CIRCLE_POINT_AMOUNT);
			ATTR_OBJECT (snowday, RLConst::Snowday, PUCK_MASS_BT);
			ATTR_OBJECT (snowday, RLConst::Snowday, PUCK_GROUND_STICK_FORCE);
			ATTR_OBJECT (snowday, RLConst::Snowday, PUCK_FRICTION);
			ATTR_OBJECT (snowday, RLConst::Snowday, PUCK_RESTITUTION);

			SET_TYPE_ATTR (rlConst.borrow (), "Snowday", snowday);
		}

		ATTR_OBJECT (rlConst, RLConst, CAR_AIR_CONTROL_TORQUE);
		ATTR_OBJECT (rlConst, RLConst, CAR_AIR_CONTROL_DAMPING);

		{
			// BoostPads
			auto boostPads = PyObjectRef::steal (((newfunc)PyType_GetSlot (namespaceType.borrow (), Py_tp_alloc)) (
			    namespaceType.borrow (), emptyTuple.borrow (), nullptr));

			ATTR_OBJECT (boostPads, RLConst::BoostPads, CYL_HEIGHT);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, CYL_RAD_BIG);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, CYL_RAD_SMALL);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, BOX_HEIGHT);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, BOX_RAD_BIG);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, BOX_RAD_SMALL);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, COOLDOWN_BIG);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, COOLDOWN_SMALL);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, BOOST_AMOUNT_BIG);
			ATTR_OBJECT (boostPads, RLConst::BoostPads, BOOST_AMOUNT_SMALL);
			ATTR_LIST (boostPads, RLConst::BoostPads, LOCS_SMALL_SOCCAR);
			ATTR_LIST (boostPads, RLConst::BoostPads, LOCS_BIG_SOCCAR);
			ATTR_LIST (boostPads, RLConst::BoostPads, LOCS_SMALL_HOOPS);
			ATTR_LIST (boostPads, RLConst::BoostPads, LOCS_BIG_HOOPS);

			SET_TYPE_ATTR (rlConst.borrow (), "BoostPads", boostPads);
		}

		ATTR_LIST (rlConst, RLConst, CAR_SPAWN_LOCATIONS_SOCCAR);
		ATTR_LIST (rlConst, RLConst, CAR_SPAWN_LOCATIONS_HEATSEEKER);
		ATTR_LIST (rlConst, RLConst, CAR_SPAWN_LOCATIONS_HOOPS);
		ATTR_LIST (rlConst, RLConst, CAR_RESPAWN_LOCATIONS_SOCCAR);
		ATTR_LIST (rlConst, RLConst, CAR_RESPAWN_LOCATIONS_HOOPS);
		ATTR_OBJECT (rlConst, RLConst, STEER_ANGLE_FROM_SPEED_CURVE);
		ATTR_OBJECT (rlConst, RLConst, POWERSLIDE_STEER_ANGLE_FROM_SPEED_CURVE);
		ATTR_OBJECT (rlConst, RLConst, DRIVE_SPEED_TORQUE_FACTOR_CURVE);
		ATTR_OBJECT (rlConst, RLConst, NON_STICKY_FRICTION_FACTOR_CURVE);
		ATTR_OBJECT (rlConst, RLConst, LAT_FRICTION_CURVE);
		ATTR_OBJECT (rlConst, RLConst, LONG_FRICTION_CURVE);
		ATTR_OBJECT (rlConst, RLConst, HANDBRAKE_LAT_FRICTION_FACTOR_CURVE);
		ATTR_OBJECT (rlConst, RLConst, HANDBRAKE_LONG_FRICTION_FACTOR_CURVE);
		ATTR_OBJECT (rlConst, RLConst, BALL_CAR_EXTRA_IMPULSE_FACTOR_CURVE);
		ATTR_OBJECT (rlConst, RLConst, BUMP_VEL_AMOUNT_GROUND_CURVE);
		ATTR_OBJECT (rlConst, RLConst, BUMP_VEL_AMOUNT_AIR_CURVE);
		ATTR_OBJECT (rlConst, RLConst, BUMP_UPWARD_VEL_AMOUNT_CURVE);

		SET_TYPE_ATTR (m.borrow (), "RLConst", rlConst);
	}

	return m.gift ();
}
