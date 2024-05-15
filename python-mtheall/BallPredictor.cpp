#include "Module.h"

namespace RocketSim::Python
{
PyTypeObject *BallPredictor::Type = nullptr;

PyMethodDef BallPredictor::Methods[] = {
    {.ml_name     = "get_ball_prediction",
        .ml_meth  = (PyCFunction)&BallPredictor::GetBallPrediction,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc =
            R"(get_ball_prediction(self, ball_state: RocketSim.BallState, ticks_since_last_update: int, num_states: int = 120, tick_interval: int = 1) -> List[RocketSim.BallState])"},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&BallPredictor::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.BallPredictor
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&BallPredictor::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.BallPredictor
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyType_Slot BallPredictor::Slots[] = {
    {Py_tp_new, (void *)&BallPredictor::New},
    {Py_tp_init, (void *)&BallPredictor::Init},
    {Py_tp_dealloc, (void *)&BallPredictor::Dealloc},
    {Py_tp_methods, &BallPredictor::Methods},
    {Py_tp_doc, (void *)R"(BallPredictor
__init__(self, game_mode: int = RocketSim.GameMode.SOCCAR, memory_weight_mode: int = RocketSim.MemoryWeightMode.HEAVY, tick_rate: float = 120.0))"},
    {0, nullptr},
};

PyType_Spec BallPredictor::Spec = {
    .name      = "RocketSim.BallPredictor",
    .basicsize = sizeof (BallPredictor),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = BallPredictor::Slots,
};

bool BallPredictor::InitFromArena (BallPredictor *self_, RocketSim::Arena *arena_) noexcept
{
	if (arena_ != self_->tracker.ballPredArena)
		delete self_->tracker.ballPredArena;

	self_->tracker.ballPredArena = arena_;
	self_->tracker.predData.clear ();
	self_->tracker.lastUpdateTickCount = 0;

	arena_->tickCount = 0;

	return true;
}

bool BallPredictor::InitFromParams (BallPredictor *self_,
    RocketSim::GameMode gameMode_,
    RocketSim::ArenaMemWeightMode memoryWeightMode_,
    float tickRate_) noexcept
{
	switch (gameMode_)
	{
	case RocketSim::GameMode::SOCCAR:
	case RocketSim::GameMode::HOOPS:
	case RocketSim::GameMode::SNOWDAY:
	case RocketSim::GameMode::HEATSEEKER:
	case RocketSim::GameMode::THE_VOID:
		break;

	default:
		PyErr_Format (PyExc_ValueError, "Invalid arena memory weight mode '%d'", gameMode_);
		return false;
	}

	switch (memoryWeightMode_)
	{
	case RocketSim::ArenaMemWeightMode::LIGHT:
	case RocketSim::ArenaMemWeightMode::HEAVY:
		break;

	default:
		PyErr_Format (PyExc_ValueError, "Invalid arena memory weight mode '%d'", memoryWeightMode_);
		return false;
	}

	if (self_->tracker.ballPredArena)
	{
		// try to reuse existing arena
		auto const arena = self_->tracker.ballPredArena;
		if (arena->gameMode == gameMode_ && arena->GetArenaConfig ().memWeightMode == memoryWeightMode_ &&
		    InitFromArena (self_, arena))
		{
			self_->tracker.ballPredArena = arena;
			self_->tracker.predData.clear ();
			self_->tracker.lastUpdateTickCount = 0;

			arena->tickCount = 0;
			arena->tickTime  = 1.0f / tickRate_;

			return true;
		}
	}

	try
	{
		// default initialization if it hasn't been done yet
		InitInternal (nullptr);
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return false;
	}

	try
	{
		RocketSim::ArenaConfig arenaConfig;
		arenaConfig.memWeightMode = memoryWeightMode_;

		auto arena = RocketSim::Arena::Create (gameMode_, arenaConfig, tickRate_);
		if (!arena)
			throw -1;

		if (!InitFromArena (self_, arena))
			throw -1;

		return true;
	}
	catch (std::bad_alloc const &err)
	{
		PyErr_NoMemory ();
		return false;
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return false;
	}
	catch (...)
	{
		PyErr_SetString (PyExc_RuntimeError, "Unknown exception");
		return false;
	}
}

PyObject *BallPredictor::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<BallPredictor>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	// bypass constructor so we can create arena on Init
	self->tracker.ballPredArena = nullptr;
	new (&self->tracker.predData) std::vector<BallState>{};
	self->tracker.numPredTicks        = 0;
	self->tracker.lastUpdateTickCount = 0;

	return self.giftObject ();
}

int BallPredictor::Init (BallPredictor *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char gameModeKwd[]         = "game_mode";
	static char memoryWeightModeKwd[] = "memory_weight_mode";
	static char tickRateKwd[]         = "tick_rate";

	static char *dict[] = {gameModeKwd, memoryWeightModeKwd, tickRateKwd, nullptr};

	int gameMode         = static_cast<int> (RocketSim::GameMode::SOCCAR);
	int memoryWeightMode = static_cast<int> (RocketSim::ArenaMemWeightMode::HEAVY);
	float tickRate       = 120.0f;

	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|iif", dict, &gameMode, &memoryWeightMode, &tickRate))
		return -1;

	if (!InitFromParams (self_,
	        static_cast<RocketSim::GameMode> (gameMode),
	        static_cast<RocketSim::ArenaMemWeightMode> (memoryWeightMode),
	        tickRate))
		return -1;

	return 0;
}

void BallPredictor::Dealloc (BallPredictor *self_) noexcept
{
	self_->tracker.~BallPredTracker ();

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *BallPredictor::Pickle (BallPredictor *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	auto const arena = self_->tracker.ballPredArena;

	if (arena && arena->gameMode != RocketSim::GameMode::SOCCAR &&
	    !DictSetValue (dict.borrow (), "game_mode", PyLong_FromLong (static_cast<long> (arena->gameMode))))
		return nullptr;

	if (arena && arena->GetArenaConfig ().memWeightMode != RocketSim::ArenaMemWeightMode::HEAVY &&
	    !DictSetValue (dict.borrow (),
	        "memory_weight_mode",
	        PyLong_FromLong (static_cast<long> (arena->GetArenaConfig ().memWeightMode))))
		return nullptr;

	if (arena->tickTime != 1.0f / 120.0f &&
	    !DictSetValue (dict.borrow (), "tick_time", PyFloat_FromDouble (arena->tickTime)))
		return nullptr;

	return dict.gift ();
}

PyObject *BallPredictor::Unpickle (BallPredictor *self_, PyObject *dict_) noexcept
{
	if (!Py_IS_TYPE (dict_, &PyDict_Type))
	{
		PyErr_SetString (PyExc_ValueError, "Pickled object is not a dict");
		return nullptr;
	}

	auto const dummy = PyObjectRef::steal (PyTuple_New (0));
	if (!dummy)
		return nullptr;

	static char gameModeKwd[]         = "game_mode";
	static char memoryWeightModeKwd[] = "memory_weight_mode";
	static char tickRateKwd[]         = "tick_rate";

	static char *dict[] = {gameModeKwd, memoryWeightModeKwd, tickRateKwd, nullptr};

	int gameMode         = static_cast<int> (RocketSim::GameMode::SOCCAR);
	int memoryWeightMode = static_cast<int> (RocketSim::ArenaMemWeightMode::HEAVY);
	float tickTime       = 1.0f / 120.0f;

	if (!PyArg_ParseTupleAndKeywords (dummy.borrow (), dict_, "|iif", dict, &gameMode, &memoryWeightMode, &tickTime))
		return nullptr;

	if (!InitFromParams (self_,
	        static_cast<RocketSim::GameMode> (gameMode),
	        static_cast<RocketSim::ArenaMemWeightMode> (memoryWeightMode),
	        1.0f / tickTime))
		return nullptr;

	self_->tracker.ballPredArena->tickTime = tickTime;

	Py_RETURN_NONE;
}

PyObject *BallPredictor::Copy (BallPredictor *self_) noexcept
{
	return DeepCopy (self_, nullptr);
}

PyObject *BallPredictor::DeepCopy (BallPredictor *self_, PyObject *memo_) noexcept
{
	auto self = PyRef<BallPredictor>::stealObject (New (Type, nullptr, nullptr));
	if (!self)
		return nullptr;

	auto const arena = self_->tracker.ballPredArena;

	if (!InitFromParams (self.borrow (),
	        arena ? arena->gameMode : RocketSim::GameMode::SOCCAR,
	        arena ? arena->GetArenaConfig ().memWeightMode : RocketSim::ArenaMemWeightMode::HEAVY,
	        1.0f / (arena ? arena->tickTime : 120.0f)))
		return nullptr;

	return self.giftObject ();
}

PyObject *BallPredictor::GetBallPrediction (BallPredictor *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char ballStateKwd[]            = "ball_state";
	static char ticksSinceLastUpdateKwd[] = "ticks_since_last_update";
	static char numStatesKwd[]            = "num_states";
	static char tickIntervalKwd[]         = "tick_interval";

	static char *dict[] = {ballStateKwd, ticksSinceLastUpdateKwd, numStatesKwd, tickIntervalKwd, nullptr};

	PyObject *ballState; // borrow reference
	unsigned int ticksSinceLastUpdate;
	unsigned int numStates    = 120;
	unsigned int tickInterval = 1;
	if (!PyArg_ParseTupleAndKeywords (args_,
	        kwds_,
	        "O!I|II",
	        dict,
	        BallState::Type,
	        &ballState,
	        &ticksSinceLastUpdate,
	        &numStates,
	        &tickInterval))
		return nullptr;

	try
	{
		if (numStates * tickInterval > self_->tracker.predData.capacity ())
			self_->tracker.predData.reserve (numStates * tickInterval);
	}
	catch (std::bad_alloc const &err)
	{
		PyErr_NoMemory ();
		return nullptr;
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return nullptr;
	}
	catch (...)
	{
		PyErr_SetString (PyExc_RuntimeError, "Unknown exception");
		return nullptr;
	}

	auto states = PyObjectRef::steal (PyList_New (numStates));
	if (!states)
		return nullptr;

	self_->tracker.numPredTicks = numStates * tickInterval;
	self_->tracker.UpdatePredManual (PyCast<BallState> (ballState)->state, ticksSinceLastUpdate);

	for (unsigned i = 0; i < numStates; ++i)
	{
		auto state = BallState::NewFromBallState (self_->tracker.predData[i * tickInterval]);
		if (!state)
			return nullptr;

		// steals ref
		if (PyList_SetItem (states.borrow (), i, state.giftObject ()) < 0)
			return nullptr;
	}

	return states.giftObject ();
}
}
