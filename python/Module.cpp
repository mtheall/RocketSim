#include "Module.h"

#include <memory>
#include <vector>

namespace RocketSim::Python
{
struct PyModuleDef Module = {PyModuleDef_HEAD_INIT, "RocketSim", nullptr, -1, nullptr};
}

PyMODINIT_FUNC PyInit_RocketSim () noexcept
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
	MAKE_TYPE (RotMat);
	MAKE_TYPE (Vec);
	MAKE_TYPE (WheelPairConfig);

	if (PyModule_AddIntConstant (m.borrow (), "SOCCAR", static_cast<long> (GameMode::SOCCAR)) < 0 ||
	    PyModule_AddIntConstant (m.borrow (), "BLUE", static_cast<long> (Team::BLUE)) < 0 ||
	    PyModule_AddIntConstant (m.borrow (), "ORANGE", static_cast<long> (Team::ORANGE)) < 0 ||
	    PyModule_AddIntConstant (m.borrow (), "OCTANE", 0) < 0 ||
	    PyModule_AddIntConstant (m.borrow (), "DOMINUS", 1) < 0 ||
	    PyModule_AddIntConstant (m.borrow (), "PLANK", 2) < 0 ||
	    PyModule_AddIntConstant (m.borrow (), "BREAKOUT", 3) < 0 ||
	    PyModule_AddIntConstant (m.borrow (), "HYBRID", 4) < 0 || PyModule_AddIntConstant (m.borrow (), "MERC", 5) < 0)
		return nullptr;

	return m.gift ();
}
