#include "Module.h"

#include "Array.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <exception>
#include <future>
#include <mutex>
#include <new>
#include <set>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std::chrono_literals;

namespace
{
static_assert (sizeof (float) == sizeof (std::uint32_t));

std::uint32_t makeKey (float x_, float y_) noexcept
{
	auto const x = static_cast<std::int16_t> (x_) + 0x2000;
	auto const y = static_cast<std::int16_t> (y_) + 0x2000;

	assert (x >= 0);
	assert (y >= 0);

	return (static_cast<std::uint32_t> (x) << 16) | y;
}

std::tuple<float, float> extractKey (std::uint32_t key_) noexcept
{
	float const x = static_cast<std::int16_t> (key_ >> 16) - 0x2000;
	float const y = static_cast<std::int16_t> (key_) - 0x2000;

	return std::make_tuple (x, y);
}

std::array<std::uint32_t, 34> const soccarIndexMapping = {
    // clang-format off
    makeKey (    0, -4240),
    makeKey (-1792, -4184),
    makeKey ( 1792, -4184),
    makeKey (-3072, -4096),
    makeKey ( 3072, -4096),
    makeKey (- 940, -3308),
    makeKey (  940, -3308),
    makeKey (    0, -2816),
    makeKey (-3584, -2484),
    makeKey ( 3584, -2484),
    makeKey (-1788, -2300),
    makeKey ( 1788, -2300),
    makeKey (-2048, -1036),
    makeKey (    0, -1024),
    makeKey ( 2048, -1036),
    makeKey (-3584,     0),
    makeKey (-1024,     0),
    makeKey ( 1024,     0),
    makeKey ( 3584,     0),
    makeKey (-2048,  1036),
    makeKey (    0,  1024),
    makeKey ( 2048,  1036),
    makeKey (-1788,  2300),
    makeKey ( 1788,  2300),
    makeKey (-3584,  2484),
    makeKey ( 3584,  2484),
    makeKey (    0,  2816),
    makeKey (- 940,  3308),
    makeKey (  940,  3308),
    makeKey (-3072,  4096),
    makeKey ( 3072,  4096),
    makeKey (-1792,  4184),
    makeKey ( 1792,  4184),
    makeKey (    0,  4240),
    // clang-format on
};

std::array<std::uint32_t, 20> const hoopsIndexMapping = {
    // clang-format off
    makeKey (-2176, -2944),
    makeKey ( 2176, -2944),
    makeKey (    0, -2816),
    makeKey (-1280, -2304),
    makeKey ( 1280, -2304),
    makeKey (-1536, -1024),
    makeKey ( 1536, -1024),
    makeKey (- 512, - 512),
    makeKey (  512, - 512),
    makeKey (-2432,     0),
    makeKey ( 2432,     0),
    makeKey (- 512,   512),
    makeKey (  512,   512),
    makeKey (-1536,  1024),
    makeKey ( 1536,  1024),
    makeKey (-1280,  2304),
    makeKey ( 1280,  2304),
    makeKey (    0,  2816),
    makeKey (-2176,  2944),
    makeKey ( 2175,  2944), // truncated from (2175.99, 2944)
    // clang-format on
};

template <std::size_t Size>
std::unordered_map<std::uint64_t, unsigned> buildBoostMapping (
    std::array<std::uint32_t, Size> const &indexMapping_) noexcept
{
	std::unordered_map<std::uint64_t, unsigned> result;

	for (unsigned i = 0; i < indexMapping_.size (); ++i)
		result.emplace (indexMapping_[i], i);

	return result;
};

auto const soccarBoostMapping = buildBoostMapping (soccarIndexMapping);
auto const hoopsBoostMapping  = buildBoostMapping (hoopsIndexMapping);

int getBoostPadIndex (::BoostPad const *pad_, std::unordered_map<std::uint64_t, unsigned> const &map_) noexcept
{
	auto const it = map_.find (makeKey (pad_->pos.x, pad_->pos.y));
	if (it == std::end (map_))
		return -1;

	return it->second;
}

std::pair<std::uint32_t const *, std::size_t> getIndexMapping (::GameMode gameMode_) noexcept
{
	if (gameMode_ == ::GameMode::HOOPS)
		return {hoopsIndexMapping.data (), hoopsIndexMapping.size ()};

	return {soccarIndexMapping.data (), soccarIndexMapping.size ()};
}

std::unordered_map<std::uint64_t, unsigned> const &getBoostMapping (::GameMode gameMode_) noexcept
{
	if (gameMode_ == ::GameMode::HOOPS)
		return hoopsBoostMapping;

	return soccarBoostMapping;
}

bool ensureBoostPadByIndex (RocketSim::Python::Arena *arena_) noexcept
{
	if (arena_->boostPads->empty () || arena_->boostPadsByIndex)
		return true;

	arena_->boostPadsByIndex = new (std::nothrow)
	    std::vector<RocketSim::Python::PyRef<RocketSim::Python::BoostPad>> (arena_->boostPads->size ());
	if (!arena_->boostPadsByIndex)
	{
		PyErr_NoMemory ();
		return false;
	}

	auto const &boostMapping = getBoostMapping (arena_->arena->gameMode);
	for (auto const &[ptr, pad] : *arena_->boostPads)
	{
		auto const index = getBoostPadIndex (ptr, boostMapping);
		if (index < 0 || static_cast<unsigned> (index) > arena_->boostPads->size ())
		{
			delete arena_->boostPadsByIndex;
			arena_->boostPadsByIndex = nullptr;
			PyErr_SetString (PyExc_ValueError, "Failed to map boost pad index");
			return false;
		}

		(*arena_->boostPadsByIndex)[index] = pad;
	}

	for (auto const &pad : *arena_->boostPadsByIndex)
	{
		if (!pad)
		{
			delete arena_->boostPadsByIndex;
			arena_->boostPadsByIndex = nullptr;
			PyErr_SetString (PyExc_ValueError, "Failed to map boost pad index");
			return false;
		}
	}

	return true;
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned row_, unsigned col_, btVector3 const &value_) noexcept
{
	array_ (row_, col_ + 0) = value_.x ();
	array_ (row_, col_ + 1) = value_.y ();
	array_ (row_, col_ + 2) = value_.z ();
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned col_, btVector3 const &value_) noexcept
{
	assign (array_, 0, col_, value_);
	assign (array_, 1, col_, btVector3 (-value_.x (), -value_.y (), value_.z ()));
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned col_, btQuaternion const &value_) noexcept
{
	array_ (0, col_ + 0) = value_.w ();
	array_ (0, col_ + 1) = value_.x ();
	array_ (0, col_ + 2) = value_.y ();
	array_ (0, col_ + 3) = value_.z ();

	array_ (1, col_ + 0) = value_.z ();
	array_ (1, col_ + 1) = value_.y ();
	array_ (1, col_ + 2) = -value_.x ();
	array_ (1, col_ + 3) = -value_.w ();
}

btVector3 calcPYR (btMatrix3x3 const &mat_) noexcept
{
	btVector3 result;
	mat_.getEulerYPR (result.y (), result.x (), result.z ());
	result.z () *= -1.0f;
	return result;
}

void assign (RocketSim::Python::PyArrayRef &array_, unsigned col_, btMatrix3x3 const &mat_) noexcept
{
	auto const forward = mat_.getColumn (0);
	auto const right   = mat_.getColumn (1);
	auto const up      = mat_.getColumn (2);

	assign (array_, col_ + 0, forward);
	assign (array_, col_ + 3, right);
	assign (array_, col_ + 6, up);

	assign (array_, 0, col_ + 9, calcPYR (mat_));
	assign (array_, 1, col_ + 9, calcPYR (btMatrix3x3 (-1, 0, 0, 0, -1, 0, 0, 0, 1) * mat_));
}

void assign (RocketSim::Python::PyArrayRef &array_,
    unsigned col_,
    btRigidBody &rigidBody_,
    CarState const *state_ = nullptr) noexcept
{
	if (state_)
	{
		assign (array_, col_ + 0, btVector3 (state_->pos));
		assign (array_, col_ + 7, btVector3 (state_->vel));
		assign (array_, col_ + 10, btVector3 (state_->angVel));
	}
	else
	{
		assign (array_, col_ + 0, rigidBody_.getWorldTransform ().getOrigin () * BT_TO_UU);
		assign (array_, col_ + 7, rigidBody_.getLinearVelocity () * BT_TO_UU);
		assign (array_, col_ + 10, rigidBody_.getAngularVelocity ());
	}

	assign (array_, col_ + 3, rigidBody_.getOrientation ());
	assign (array_, col_ + 13, rigidBody_.getWorldTransform ().getBasis ());
}

void saveException (RocketSim::Python::Arena const *const arena_) noexcept
{
	PyErr_Fetch (&arena_->stepExceptionType, &arena_->stepExceptionValue, &arena_->stepExceptionTraceback);
	arena_->arena->Stop ();
}

void restoreException (RocketSim::Python::Arena const *const arena_) noexcept
{
	// normalize exception
	PyErr_NormalizeException (&arena_->stepExceptionType, &arena_->stepExceptionValue, &arena_->stepExceptionTraceback);
	if (arena_->stepExceptionTraceback)
		PyException_SetTraceback (arena_->stepExceptionValue, arena_->stepExceptionTraceback);
	else
		PyException_SetTraceback (arena_->stepExceptionValue, Py_None);

	// this steals our reference
	PyErr_Restore (arena_->stepExceptionType, arena_->stepExceptionValue, arena_->stepExceptionTraceback);

	arena_->stepExceptionType      = nullptr;
	arena_->stepExceptionValue     = nullptr;
	arena_->stepExceptionTraceback = nullptr;
}

void discardException (RocketSim::Python::Arena const *const arena_) noexcept
{
	Py_XDECREF (arena_->stepExceptionType);
	Py_XDECREF (arena_->stepExceptionValue);
	Py_XDECREF (arena_->stepExceptionTraceback);

	arena_->stepExceptionType      = nullptr;
	arena_->stepExceptionValue     = nullptr;
	arena_->stepExceptionTraceback = nullptr;
}
}

namespace RocketSim::Python
{
class Arena::ThreadPool
{
public:
	~ThreadPool () noexcept
	{
		{
			auto const lock = std::scoped_lock (m_mutex);
			m_quit          = true;
		}

		m_cv.notify_all ();

		for (auto &thread : m_threads)
			thread.join ();
	}

	ThreadPool () noexcept
	{
		auto const numThreads = std::max (1u, std::thread::hardware_concurrency ());

		m_threads.reserve (numThreads);

		for (unsigned i = 0; i < numThreads; ++i)
			m_threads.emplace_back (&Arena::ThreadPool::run, this);
	}

	bool SubmitJob (std::set<PyRef<Arena>> const &arenas_, unsigned const ticks_) noexcept
	{
		std::vector<std::shared_future<void>> futures;

		try
		{
			futures.reserve (arenas_.size ());
		}
		catch (std::exception const &err)
		{
			PyErr_SetString (PyExc_RuntimeError, err.what ());
			return false;
		}

		{
			auto const lock = std::scoped_lock (m_mutex);

			try
			{
				for (auto &arena : arenas_)
				{
					auto future =
					    std::async (std::launch::deferred, &::Arena::Step, arena->arena.get (), ticks_).share ();
					m_jobs.emplace_back (future);
					futures.emplace_back (future);
				}
			}
			catch (std::exception const &err)
			{
				// remove added jobs
				for (unsigned i = 0; i < futures.size (); ++i)
					m_jobs.pop_back ();

				PyErr_SetString (PyExc_RuntimeError, err.what ());
				return false;
			}
		}

		if (futures.size () >= m_threads.size ())
			m_cv.notify_all ();
		else
		{
			for (unsigned i = 0; i < futures.size (); ++i)
				m_cv.notify_one ();
		}

		// wait for submitted jobs to complete
		for (auto &future : futures)
		{
			while (future.valid () && future.wait_for (100ms) != std::future_status::ready)
			{
			}
		}

		// update game tracker events
		for (auto &arena : arenas_)
		{
			if (arena->gameEvent)
				arena->gameEvent->Update (arena->arena.get ());
		}

		return true;
	}

	static std::shared_ptr<ThreadPool> GetInstance () noexcept
	{
		static std::mutex mutex;
		auto const lock = std::scoped_lock (mutex);

		try
		{
			auto pool = s_instance.lock ();
			if (!pool)
				s_instance = pool = std::make_shared<ThreadPool> ();

			return pool;
		}
		catch (...)
		{
			return nullptr;
		}
	}

private:
	void run () noexcept
	{
		while (true)
		{
			auto lock = std::unique_lock (m_mutex);

			while (!m_quit && m_jobs.empty ())
				m_cv.wait (lock);

			if (m_quit && m_jobs.empty ())
				return;

			auto job = m_jobs.front ();
			m_jobs.pop_front ();

			lock.unlock ();

			job.get ();
		}
	}

	static std::weak_ptr<ThreadPool> s_instance;

	std::vector<std::thread> m_threads;
	std::deque<std::shared_future<void>> m_jobs;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	bool m_quit = false;
};

std::weak_ptr<Arena::ThreadPool> Arena::ThreadPool::s_instance;

PyTypeObject *Arena::Type = nullptr;

PyMemberDef Arena::Members[] = {
    {.name = "ball", .type = T_OBJECT_EX, .offset = offsetof (Arena, ball), .flags = READONLY, .doc = "Ball"},
    {.name      = "blue_score",
        .type   = TypeHelper<decltype (Arena::blueScore)>::type,
        .offset = offsetof (Arena, blueScore),
        .flags  = READONLY,
        .doc    = "Blue score"},
    {.name      = "orange_score",
        .type   = TypeHelper<decltype (Arena::orangeScore)>::type,
        .offset = offsetof (Arena, orangeScore),
        .flags  = READONLY,
        .doc    = "Orange score"},
    {.name = nullptr, .type = 0, .offset = 0, .flags = 0, .doc = nullptr},
};

PyMethodDef Arena::Methods[] = {
    {.ml_name     = "add_car",
        .ml_meth  = (PyCFunction)&Arena::AddCar,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(add_car(self, team: int, config: int = RocketSim.CarConfig.OCTANE) -> RocketSim.Car
add_car(self, team: int, config: RocketSim.CarConfig) -> RocketSim.Car
Use RocketSim.Team.BLUE or RocketSim.Team.ORANGE for team
Use RocketSim.CarConfig.OCTANE and friends for the int version of config)"},
    {.ml_name     = "clone",
        .ml_meth  = (PyCFunction)&Arena::Clone,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(clone(self, copy_callbacks: bool = False) -> RocketSim.Arena)"},
    {.ml_name     = "clone_into",
        .ml_meth  = (PyCFunction)&Arena::CloneInto,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(clone_into(self, target: RocketSim.Arena, copy_callbacks: bool = False))"},
    {.ml_name     = "get_car_from_id",
        .ml_meth  = (PyCFunction)&Arena::GetCarFromId,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(get_car_from_id(self, car_id: int) -> RocketSim.Car
Raises KeyError if car doesn't exist

get_car_from_id(self, car_id: int, default) -> RocketSim.Car
Returns `default` if car doesn't exist)"},
    {.ml_name     = "get_cars",
        .ml_meth  = (PyCFunction)&Arena::GetCars,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_cars(self) -> List[RocketSim.Car])"},
    {.ml_name     = "get_ball_prediction",
        .ml_meth  = (PyCFunction)&Arena::GetBallPrediction,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc =
            R"(get_ball_prediction(self, num_ticks: int = 120, tick_skip: int = 1) -> List[Tuple[float, RocketSim.BallState]])"},
    {.ml_name     = "get_boost_pads",
        .ml_meth  = (PyCFunction)&Arena::GetBoostPads,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_boost_pads(self) -> List[RocketSim.BoostPad])"},
    {.ml_name     = "get_gym_state",
        .ml_meth  = (PyCFunction)&Arena::GetGymState,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_gym_state(self) -> tuple
Gets game state for consumption by RLGym
Format: tuple(a, b, c, car1, car2, ...)
	a:    numpy.array([game_mode, ball_last_car_hit_id, blue_score, orange_score])
	b:    numpy.array([boost_pad_active, ...], [boost_pad_active_reverse, ...])
	c:    numpy.array(ball_state, ball_state_inverse)
	carX: numpy.array(car_state, car_state_inverse)

	ball_state: [pos_x, pos_y, pos_z,
	             quat_w, quat_x, quat_y, quat_z,
	             vel_x, vel_y, vel_z,
	             ang_vel_x, ang_vel_y, ang_vel_z,
	             rot_forward_x, rot_forward_y, rot_forward_z,
	             rot_right_x, rot_right_y, rot_right_z,
	             rot_up_x, rot_up_y, rot_up_z,
	             pitch, yaw, roll] # applied in yaw-pitch-roll order
	ball_state_inverse: ball_state rotated around Z-axis

	car_state: [car_id, team, goals, saves, shots, demos, boost_pickups,
	            is_demoed, is_on_ground, hit_last_step, boost, # boost is 0 to 100
	            pos_x, pos_y, pos_z,
	            quat_w, quat_x, quat_y, quat_z,
	            vel_x, vel_y, vel_z,
	            ang_vel_x, ang_vel_y, ang_vel_z,
	            rot_forward_x, rot_forward_y, rot_forward_z,
	            rot_right_x, rot_right_y, rot_right_z,
	            rot_up_x, rot_up_y, rot_up_z,
	            pitch, yaw, roll] # applied in yaw-pitch-roll order
	car_state_inverse: car_state rotated around Z-axis)"},

    {.ml_name     = "get_mutator_config",
        .ml_meth  = (PyCFunction)&Arena::GetMutatorConfig,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(get_mutator_config(self) -> RocketSim.MutatorConfig)"},
    {.ml_name     = "remove_car",
        .ml_meth  = (PyCFunction)&Arena::RemoveCar,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(remove_car(self, car: RocketSim.Car)
remove_car(self, car: int))"},
    {.ml_name     = "is_ball_probably_going_in",
        .ml_meth  = (PyCFunction)&Arena::IsBallProbablyGoingIn,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(is_ball_probably_going_in(self, max_time: float = 0.2) -> bool)"},
    {.ml_name     = "reset_kickoff",
        .ml_meth  = (PyCFunction)&Arena::ResetKickoff,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(reset_kickoff(self, seed = -1))"},
    {.ml_name     = "set_ball_touch_callback",
        .ml_meth  = (PyCFunction)&Arena::SetBallTouchCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_ball_touch_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, car: RocketSim.Car, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_boost_pickup_callback",
        .ml_meth  = (PyCFunction)&Arena::SetBoostPickupCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_boost_pickup_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, car: RocketSim.Car, boost_pad: RocketSim.BoostPad, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_car_ball_collision",
        .ml_meth  = (PyCFunction)&Arena::SetCarBallCollision,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_car_ball_collision(self, enable = True) -> None
Set whether cars can collide with the ball)"},
    {.ml_name     = "set_car_bump_callback",
        .ml_meth  = (PyCFunction)&Arena::SetCarBumpCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_car_bump_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, bumper: RocketSim.Car, victim: RocketSim.Car, is_demo: bool, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_car_car_collision",
        .ml_meth  = (PyCFunction)&Arena::SetCarCarCollision,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_car_car_collision(self, enable = True) -> None
Set whether cars can collide with each other)"},
    {.ml_name     = "set_car_demo_callback",
        .ml_meth  = (PyCFunction)&Arena::SetCarDemoCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_car_demo_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, bumper: RocketSim.Car, victim: RocketSim.Car, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_goal_score_callback",
        .ml_meth  = (PyCFunction)&Arena::SetGoalScoreCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_goal_score_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, scoring_team: int, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_shot_event_callback",
        .ml_meth  = (PyCFunction)&Arena::SetShotEventCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_shot_event_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, shooter: RocketSim.Car, passer: RocketSim.Car, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_goal_event_callback",
        .ml_meth  = (PyCFunction)&Arena::SetGoalEventCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_goal_event_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, scorer: RocketSim.Car, passer: RocketSim.Car, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_save_event_callback",
        .ml_meth  = (PyCFunction)&Arena::SetSaveEventCallback,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_save_event_callback(self, callback, data = None) -> tuple
`callback` must be callable e.g. `def callback(arena: RocketSim.Arena, saver: RocketSim.Car, data)`
`callback` always called with keyword arguments
Returns previous (callback, data))"},
    {.ml_name     = "set_mutator_config",
        .ml_meth  = (PyCFunction)&Arena::SetMutatorConfig,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(set_mutator_config(self, config: RocketSim.MutatorConfig))"},
    {.ml_name     = "step",
        .ml_meth  = (PyCFunction)&Arena::Step,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = R"(step(self, ticks: int = 1))"},
    {.ml_name = "stop", .ml_meth = (PyCFunction)&Arena::Stop, .ml_flags = METH_NOARGS, .ml_doc = R"(stop(self)
This can be called from within a callback to stop simulation early)"},
    {.ml_name     = "multi_step",
        .ml_meth  = (PyCFunction)&Arena::MultiStep,
        .ml_flags = METH_VARARGS | METH_KEYWORDS | METH_STATIC,
        .ml_doc   = R"(multi_step(arenas: list[RocketSim.Arena] = [], ticks: int = 1))"},
    {.ml_name = "__getstate__", .ml_meth = (PyCFunction)&Arena::Pickle, .ml_flags = METH_NOARGS, .ml_doc = nullptr},
    {.ml_name = "__setstate__", .ml_meth = (PyCFunction)&Arena::Unpickle, .ml_flags = METH_O, .ml_doc = nullptr},
    {.ml_name     = "__copy__",
        .ml_meth  = (PyCFunction)&Arena::Copy,
        .ml_flags = METH_NOARGS,
        .ml_doc   = R"(__copy__(self) -> RocketSim.Arena
Shallow copy)"},
    {.ml_name     = "__deepcopy__",
        .ml_meth  = (PyCFunction)&Arena::DeepCopy,
        .ml_flags = METH_O,
        .ml_doc   = R"(__deepcopy__(self, memo) -> RocketSim.Arena
Deep copy)"},
    {.ml_name = nullptr, .ml_meth = nullptr, .ml_flags = 0, .ml_doc = nullptr},
};

PyGetSetDef Arena::GetSet[] = {
    GETONLY_ENTRY (Arena, game_mode, "Game mode"),
    GETONLY_ENTRY (Arena, tick_count, "Tick count"),
    GETONLY_ENTRY (Arena, tick_rate, "Tick rate"),
    GETONLY_ENTRY (Arena, tick_time, "Tick time"),
    {.name = nullptr, .get = nullptr, .set = nullptr, .doc = nullptr, .closure = nullptr},
};

PyType_Slot Arena::Slots[] = {
    {Py_tp_new, (void *)&Arena::New},
    {Py_tp_init, (void *)&Arena::Init},
    {Py_tp_dealloc, (void *)&Arena::Dealloc},
    {Py_tp_methods, &Arena::Methods},
    {Py_tp_members, &Arena::Members},
    {Py_tp_getset, &Arena::GetSet},
    {Py_tp_doc, (void *)R"(Arena
__init__(self, game_mode: int = RocketSim.GameMode.SOCCAR, memory_weight_mode = RocketSim.MemoryWeightMode.HEAVY, tick_rate: float = 120.0))"},
    {0, nullptr},
};

PyType_Spec Arena::Spec = {
    .name      = "RocketSim.Arena",
    .basicsize = sizeof (Arena),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = Arena::Slots,
};

PyObject *Arena::New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept
{
	auto const tp_alloc = (allocfunc)PyType_GetSlot (subtype_, Py_tp_alloc);

	auto self = PyRef<Arena>::stealObject (tp_alloc (subtype_, 0));
	if (!self)
		return nullptr;

	new (&self->arena) std::shared_ptr<::Arena>{};
	new (&self->threadPool) std::shared_ptr<Arena::ThreadPool>{};
	self->cars                        = new (std::nothrow) std::map<std::uint32_t, PyRef<Car>>{};
	self->boostPads                   = new (std::nothrow) std::unordered_map<::BoostPad *, PyRef<BoostPad>>{};
	self->boostPadsByIndex            = nullptr;
	self->ballPrediction              = nullptr;
	self->gameEvent                   = nullptr;
	self->ball                        = nullptr;
	self->ballTouchCallback           = nullptr;
	self->ballTouchCallbackUserData   = nullptr;
	self->boostPickupCallback         = nullptr;
	self->boostPickupCallbackUserData = nullptr;
	self->carBumpCallback             = nullptr;
	self->carBumpCallbackUserData     = nullptr;
	self->carDemoCallback             = nullptr;
	self->carDemoCallbackUserData     = nullptr;
	self->goalScoreCallback           = nullptr;
	self->goalScoreCallbackUserData   = nullptr;
	self->shotEventCallback           = nullptr;
	self->shotEventCallbackUserData   = nullptr;
	self->goalEventCallback           = nullptr;
	self->goalEventCallbackUserData   = nullptr;
	self->saveEventCallback           = nullptr;
	self->saveEventCallbackUserData   = nullptr;
	self->blueScore                   = 0;
	self->orangeScore                 = 0;
	self->lastGoalTick                = 0;
	self->lastGymStateTick            = 0;
	self->stepExceptionType           = nullptr;
	self->stepExceptionValue          = nullptr;
	self->stepExceptionTraceback      = nullptr;

	if (!self->cars || !self->boostPads)
	{
		self->arena.~shared_ptr ();
		self->threadPool.~shared_ptr ();
		delete self->cars;
		delete self->boostPads;

		return PyErr_NoMemory ();
	}

	return self.giftObject ();
}

int Arena::Init (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	int gameMode         = static_cast<int> (::GameMode::SOCCAR);
	int memoryWeightMode = static_cast<int> (::ArenaMemWeightMode::HEAVY);
	float tickRate       = 120.0f;

	static char gameModeKwd[]         = "game_mode";
	static char memoryWeightModeKwd[] = "memory_weight_mode";
	static char tickRateKwd[]         = "tick_rate";

	static char *dict[] = {gameModeKwd, memoryWeightModeKwd, tickRateKwd, nullptr};

	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|iif", dict, &gameMode, &memoryWeightMode, &tickRate))
		return -1;

	try
	{
		switch (static_cast<::GameMode> (gameMode))
		{
		case ::GameMode::SOCCAR:
		case ::GameMode::HOOPS:
		case ::GameMode::SNOWDAY:
			self_->gameEvent = new ::GameEventTracker{};
			self_->gameEvent->SetShotCallback (&Arena::HandleShotEventCallback, self_);
			self_->gameEvent->SetGoalCallback (&Arena::HandleGoalEventCallback, self_);
			self_->gameEvent->SetSaveCallback (&Arena::HandleSaveEventCallback, self_);
			break;

		case ::GameMode::HEATSEEKER:
		case ::GameMode::THE_VOID:
			break;

		default:
			PyErr_SetString (PyExc_RuntimeError, "Invalid game mode");
			return -1;
		}
	}
	catch (...)
	{
		PyErr_NoMemory ();
		return -1;
	}

	if (tickRate < 15.0f || tickRate > 120.0f)
	{
		PyErr_SetString (PyExc_RuntimeError, "Invalid tick rate");
		return -1;
	}

	try
	{
		// default initialization if it hasn't been done yet
		InitInternal (nullptr);
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return -1;
	}

	try
	{
		auto arena = std::shared_ptr<::Arena> (::Arena::Create (
		    static_cast<::GameMode> (gameMode), static_cast<::ArenaMemWeightMode> (memoryWeightMode), tickRate));
		if (!arena)
			throw -1;

		arena->SetCarBumpCallback (&Arena::HandleCarBumpCallback, self_);

		if (arena->gameMode != ::GameMode::THE_VOID)
		{
			arena->SetBoostPickupCallback (&Arena::HandleBoostPickupCallback, self_);
			arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, self_);
		}

		auto ball = PyRef<Ball>::steal (Ball::New ());
		if (!ball)
			throw -1;

		auto boostPads = std::unordered_map<::BoostPad *, PyRef<BoostPad>>{};
		for (auto const &pad : arena->GetBoostPads ())
		{
			auto ref = PyRef<BoostPad>::steal (BoostPad::New ());
			if (!ref)
				throw -1;

			ref->pad = pad;
			boostPads.emplace (pad, std::move (ref));
		}

		// no exceptions thrown after this point
		self_->arena = arena;
		self_->cars->clear ();
		*self_->boostPads = std::move (boostPads);

		PyRef<Ball>::assign (self_->ball, ball.borrowObject ());
		self_->ball->arena = self_->arena;
		self_->ball->ball  = self_->arena->ball;

		PyObjectRef::assign (self_->ballTouchCallback, Py_None);
		PyObjectRef::assign (self_->ballTouchCallbackUserData, Py_None);
		PyObjectRef::assign (self_->boostPickupCallback, Py_None);
		PyObjectRef::assign (self_->boostPickupCallbackUserData, Py_None);
		PyObjectRef::assign (self_->carBumpCallback, Py_None);
		PyObjectRef::assign (self_->carBumpCallbackUserData, Py_None);
		PyObjectRef::assign (self_->carDemoCallback, Py_None);
		PyObjectRef::assign (self_->carDemoCallbackUserData, Py_None);
		PyObjectRef::assign (self_->goalScoreCallback, Py_None);
		PyObjectRef::assign (self_->goalScoreCallbackUserData, Py_None);
		PyObjectRef::assign (self_->shotEventCallback, Py_None);
		PyObjectRef::assign (self_->shotEventCallbackUserData, Py_None);
		PyObjectRef::assign (self_->goalEventCallback, Py_None);
		PyObjectRef::assign (self_->goalEventCallbackUserData, Py_None);
		PyObjectRef::assign (self_->saveEventCallback, Py_None);
		PyObjectRef::assign (self_->saveEventCallbackUserData, Py_None);

		return 0;
	}
	catch (...)
	{
		PyErr_NoMemory ();
		return -1;
	}
}

void Arena::Dealloc (Arena *self_) noexcept
{
	self_->arena.~shared_ptr ();
	self_->threadPool.~shared_ptr ();
	delete self_->cars;
	delete self_->boostPads;
	delete self_->boostPadsByIndex;
	delete self_->ballPrediction;
	delete self_->gameEvent;
	Py_XDECREF (self_->ball);
	Py_XDECREF (self_->ballTouchCallback);
	Py_XDECREF (self_->ballTouchCallbackUserData);
	Py_XDECREF (self_->boostPickupCallback);
	Py_XDECREF (self_->boostPickupCallbackUserData);
	Py_XDECREF (self_->carBumpCallback);
	Py_XDECREF (self_->carBumpCallbackUserData);
	Py_XDECREF (self_->carDemoCallback);
	Py_XDECREF (self_->carDemoCallbackUserData);
	Py_XDECREF (self_->goalScoreCallback);
	Py_XDECREF (self_->goalScoreCallbackUserData);
	Py_XDECREF (self_->shotEventCallback);
	Py_XDECREF (self_->shotEventCallbackUserData);
	Py_XDECREF (self_->goalEventCallback);
	Py_XDECREF (self_->goalEventCallbackUserData);
	Py_XDECREF (self_->saveEventCallback);
	Py_XDECREF (self_->saveEventCallbackUserData);

	auto const tp_free = (freefunc)PyType_GetSlot (Type, Py_tp_free);
	tp_free (self_);
}

PyObject *Arena::Pickle (Arena *self_) noexcept
{
	auto dict = PyObjectRef::steal (PyDict_New ());
	if (!dict)
		return nullptr;

	if (!DictSetValue (dict.borrow (),
	        "mutator_config",
	        MutatorConfig::NewFromMutatorConfig (self_->arena->GetMutatorConfig ()).giftObject ()))
		return nullptr;

	if (!DictSetValue (
	        dict.borrow (), "ball_state", BallState::NewFromBallState (self_->ball->ball->GetState ()).giftObject ()))
		return nullptr;

	if (!self_->cars->empty ())
	{
		auto cars = PyObjectRef::steal (PyList_New (self_->cars->size ()));
		if (!cars)
			return nullptr;

		std::size_t index = 0;
		for (auto const &[id, car] : *self_->cars)
		{
			auto entry = Car::InternalPickle (car.borrow ());
			if (!entry)
				return nullptr;

			// steals ref
			if (PyList_SetItem (cars.borrow (), index++, entry) < 0)
				return nullptr;
		}

		if (!DictSetValue (dict.borrow (), "cars", cars.gift ()))
			return nullptr;
	}

	if (!self_->boostPads->empty ())
	{
		auto pads = PyObjectRef::steal (PyList_New (self_->boostPads->size ()));
		if (!pads)
			return nullptr;

		auto const &boostMapping = getBoostMapping (self_->arena->gameMode);
		for (auto const &[ptr, pad] : *self_->boostPads)
		{
			auto entry = BoostPadState::NewFromBoostPadState (pad->pad->GetState ());
			if (!entry)
				return nullptr;

			auto const index = getBoostPadIndex (ptr, boostMapping);
			if (index < 0 || index >= PyList_Size (pads.borrow ()))
				continue;

			// steals ref
			if (PyList_SetItem (pads.borrow (), index, entry.giftObject ()) < 0)
				return nullptr;
		}

		for (unsigned i = 0; i < PyList_Size (pads.borrow ()); ++i)
		{
			if (!PyList_GetItem (pads.borrow (), i))
			{
				PyErr_SetString (PyExc_RuntimeError, "Failed to enumerate all boost pads");
				return nullptr;
			}
		}

		if (!DictSetValue (dict.borrow (), "boost_pads", pads.gift ()))
			return nullptr;
	}

	if (self_->arena->gameMode != ::GameMode::SOCCAR &&
	    !DictSetValue (dict.borrow (), "game_mode", PyLong_FromLong (static_cast<long> (self_->arena->gameMode))))
		return nullptr;

	if (self_->arena->_lastCarID &&
	    !DictSetValue (dict.borrow (), "last_car_id", PyLong_FromUnsignedLong (self_->arena->_lastCarID)))
		return nullptr;

	if (self_->arena->tickTime != 1.0f / 120.0f &&
	    !DictSetValue (dict.borrow (), "tick_time", PyFloat_FromDouble (self_->arena->tickTime)))
		return nullptr;

	if (self_->arena->tickCount &&
	    !DictSetValue (dict.borrow (), "tick_count", PyLong_FromUnsignedLongLong (self_->arena->tickCount)))
		return nullptr;

	if (self_->blueScore && !DictSetValue (dict.borrow (), "blue_score", PyLong_FromUnsignedLong (self_->blueScore)))
		return nullptr;

	if (self_->orangeScore &&
	    !DictSetValue (dict.borrow (), "orange_score", PyLong_FromUnsignedLong (self_->orangeScore)))
		return nullptr;

	if (self_->lastGoalTick &&
	    !DictSetValue (dict.borrow (), "last_goal_tick", PyLong_FromUnsignedLongLong (self_->lastGoalTick)))
		return nullptr;

	if (self_->lastGymStateTick &&
	    !DictSetValue (dict.borrow (), "last_gym_state_tick", PyLong_FromUnsignedLongLong (self_->lastGymStateTick)))
		return nullptr;

	if (self_->ballTouchCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "ball_touch_callback", PyNewRef (self_->ballTouchCallback)))
		return nullptr;

	if (self_->ballTouchCallbackUserData != Py_None &&
	    !DictSetValue (dict.borrow (), "ball_touch_callback_user_data", PyNewRef (self_->ballTouchCallbackUserData)))
		return nullptr;

	if (self_->boostPickupCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "boost_pickup_callback", PyNewRef (self_->boostPickupCallback)))
		return nullptr;

	if (self_->boostPickupCallbackUserData != Py_None &&
	    !DictSetValue (
	        dict.borrow (), "boost_pickup_callback_user_data", PyNewRef (self_->boostPickupCallbackUserData)))
		return nullptr;

	if (self_->carBumpCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "car_bump_callback", PyNewRef (self_->carBumpCallback)))
		return nullptr;

	if (self_->carBumpCallbackUserData != Py_None &&
	    !DictSetValue (dict.borrow (), "car_bump_callback_user_data", PyNewRef (self_->carBumpCallbackUserData)))
		return nullptr;

	if (self_->carDemoCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "car_bump_callback", PyNewRef (self_->carDemoCallback)))
		return nullptr;

	if (self_->carDemoCallbackUserData != Py_None &&
	    !DictSetValue (dict.borrow (), "car_bump_callback_user_data", PyNewRef (self_->carDemoCallbackUserData)))
		return nullptr;

	if (self_->goalScoreCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "goal_score_callback", PyNewRef (self_->goalScoreCallback)))
		return nullptr;

	if (self_->goalScoreCallbackUserData != Py_None &&
	    !DictSetValue (dict.borrow (), "goal_score_callback_user_data", PyNewRef (self_->goalScoreCallbackUserData)))
		return nullptr;

	if (self_->shotEventCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "shot_event_callback", PyNewRef (self_->shotEventCallback)))
		return nullptr;

	if (self_->shotEventCallbackUserData != Py_None &&
	    !DictSetValue (dict.borrow (), "shot_event_callback_user_data", PyNewRef (self_->shotEventCallbackUserData)))
		return nullptr;

	if (self_->goalEventCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "goal_event_callback", PyNewRef (self_->goalEventCallback)))
		return nullptr;

	if (self_->goalEventCallbackUserData != Py_None &&
	    !DictSetValue (dict.borrow (), "goal_event_callback_user_data", PyNewRef (self_->goalEventCallbackUserData)))
		return nullptr;

	if (self_->saveEventCallback != Py_None &&
	    !DictSetValue (dict.borrow (), "save_event_callback", PyNewRef (self_->saveEventCallback)))
		return nullptr;

	if (self_->saveEventCallbackUserData != Py_None &&
	    !DictSetValue (dict.borrow (), "save_event_callback_user_data", PyNewRef (self_->saveEventCallbackUserData)))
		return nullptr;

	return dict.gift ();
}

PyObject *Arena::Unpickle (Arena *self_, PyObject *dict_) noexcept
{
	if (!Py_IS_TYPE (dict_, &PyDict_Type))
	{
		PyErr_SetString (PyExc_ValueError, "Pickled object is not a dict");
		return nullptr;
	}

	auto const dummy = PyObjectRef::steal (PyTuple_New (0));
	if (!dummy)
		return nullptr;

	static char gameModeKwd[]                    = "game_mode";
	static char memoryWeightModeKwd[]            = "memory_weight_mode";
	static char lastCarIDKwd[]                   = "last_car_id";
	static char mutatorConfigKwd[]               = "mutator_config";
	static char tickTimeKwd[]                    = "tick_time";
	static char tickCountKwd[]                   = "tick_count";
	static char ballStateKwd[]                   = "ball_state";
	static char carsKwd[]                        = "cars";
	static char boostPadsKwd[]                   = "boost_pads";
	static char blueScoreKwd[]                   = "blue_score";
	static char orangeScoreKwd[]                 = "orange_score";
	static char lastGoalTickKwd[]                = "last_goal_tick";
	static char lastGymStateTickKwd[]            = "last_gym_state_tick";
	static char ballTouchCallbackKwd[]           = "ball_touch_callback";
	static char ballTouchCallbackUserDataKwd[]   = "ball_touch_callback_user_data";
	static char boostPickupCallbackKwd[]         = "boost_pickup_callback";
	static char boostPickupCallbackUserDataKwd[] = "boost_pickup_callback_user_data";
	static char carBumpCallbackKwd[]             = "car_bump_callback";
	static char carBumpCallbackUserDataKwd[]     = "car_bump_callback_user_data";
	static char carDemoCallbackKwd[]             = "car_demo_callback";
	static char carDemoCallbackUserDataKwd[]     = "car_demo_callback_user_data";
	static char goalScoreCallbackKwd[]           = "goal_score_callback";
	static char goalScoreCallbackUserDataKwd[]   = "goal_score_callback_user_data";
	static char shotEventCallbackKwd[]           = "shot_score_callback";
	static char shotEventCallbackUserDataKwd[]   = "shot_score_callback_user_data";
	static char goalEventCallbackKwd[]           = "goal_score_callback";
	static char goalEventCallbackUserDataKwd[]   = "goal_score_callback_user_data";
	static char saveEventCallbackKwd[]           = "save_score_callback";
	static char saveEventCallbackUserDataKwd[]   = "save_score_callback_user_data";

	static char *dict[] = {gameModeKwd,
	    memoryWeightModeKwd,
	    lastCarIDKwd,
	    mutatorConfigKwd,
	    tickTimeKwd,
	    tickCountKwd,
	    ballStateKwd,
	    carsKwd,
	    boostPadsKwd,
	    blueScoreKwd,
	    orangeScoreKwd,
	    lastGoalTickKwd,
	    lastGymStateTickKwd,
	    ballTouchCallbackKwd,
	    ballTouchCallbackUserDataKwd,
	    boostPickupCallbackKwd,
	    boostPickupCallbackUserDataKwd,
	    carBumpCallbackKwd,
	    carBumpCallbackUserDataKwd,
	    carDemoCallbackKwd,
	    carDemoCallbackUserDataKwd,
	    goalScoreCallbackKwd,
	    goalScoreCallbackUserDataKwd,
	    shotEventCallbackKwd,
	    shotEventCallbackUserDataKwd,
	    goalEventCallbackKwd,
	    goalEventCallbackUserDataKwd,
	    saveEventCallbackKwd,
	    saveEventCallbackUserDataKwd,
	    nullptr};

	PyObject *mutatorConfig               = nullptr; // borrowed references
	PyObject *ballState                   = nullptr;
	PyObject *cars                        = nullptr;
	PyObject *pads                        = nullptr;
	PyObject *ballTouchCallback           = nullptr;
	PyObject *ballTouchCallbackUserData   = nullptr;
	PyObject *boostPickupCallback         = nullptr;
	PyObject *boostPickupCallbackUserData = nullptr;
	PyObject *carBumpCallback             = nullptr;
	PyObject *carBumpCallbackUserData     = nullptr;
	PyObject *carDemoCallback             = nullptr;
	PyObject *carDemoCallbackUserData     = nullptr;
	PyObject *goalScoreCallback           = nullptr;
	PyObject *goalScoreCallbackUserData   = nullptr;
	PyObject *shotEventCallback           = nullptr;
	PyObject *shotEventCallbackUserData   = nullptr;
	PyObject *goalEventCallback           = nullptr;
	PyObject *goalEventCallbackUserData   = nullptr;
	PyObject *saveEventCallback           = nullptr;
	PyObject *saveEventCallbackUserData   = nullptr;
	unsigned long long tickCount          = 0;
	unsigned long long lastGoalTick       = 0;
	unsigned long long lastGymStateTick   = 0;
	unsigned long lastCarID               = 0;
	float tickTime                        = 1.0f / 120.0f;
	unsigned blueScore                    = 0;
	unsigned orangeScore                  = 0;
	int gameMode                          = static_cast<int> (::GameMode::SOCCAR);
	int memoryWeightMode                  = static_cast<int> (::ArenaMemWeightMode::HEAVY);
	if (!PyArg_ParseTupleAndKeywords (dummy.borrow (),
	        dict_,
	        "|iikO!fKO!O!O!IIKKOOOOOOOOOOOOOO",
	        dict,
	        &gameMode,
	        &memoryWeightMode,
	        &lastCarID,
	        MutatorConfig::Type,
	        &mutatorConfig,
	        &tickTime,
	        &tickCount,
	        BallState::Type,
	        &ballState,
	        &PyList_Type,
	        &cars,
	        &PyList_Type,
	        &pads,
	        &blueScore,
	        &orangeScore,
	        &lastGoalTick,
	        &lastGymStateTick,
	        &ballTouchCallback,
	        &ballTouchCallbackUserData,
	        &boostPickupCallback,
	        &boostPickupCallbackUserData,
	        &carBumpCallback,
	        &carBumpCallbackUserData,
	        &carDemoCallback,
	        &carDemoCallbackUserData,
	        &goalScoreCallback,
	        &goalScoreCallbackUserData,
	        &shotEventCallback,
	        &shotEventCallbackUserData,
	        &goalEventCallback,
	        &goalEventCallbackUserData,
	        &saveEventCallback,
	        &saveEventCallbackUserData))
		return nullptr;

	switch (static_cast<::GameMode> (gameMode))
	{
	case ::GameMode::SOCCAR:
	case ::GameMode::HOOPS:
	case ::GameMode::HEATSEEKER:
	case ::GameMode::SNOWDAY:
	case ::GameMode::THE_VOID:
		break;

	default:
		return PyErr_Format (PyExc_ValueError, "Invalid game mode '%d'", gameMode);
	}

	if (static_cast<::ArenaMemWeightMode> (memoryWeightMode) != ::ArenaMemWeightMode::LIGHT &&
	    static_cast<::ArenaMemWeightMode> (memoryWeightMode) != ::ArenaMemWeightMode::HEAVY)
		return PyErr_Format (PyExc_ValueError, "Invalid arena memory weight mode '%d'", memoryWeightMode);

	// make sure callback are None or callable
	if (ballTouchCallback && ballTouchCallback != Py_None && !PyCallable_Check (ballTouchCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid ball touch callback");
		return nullptr;
	}

	if (boostPickupCallback && boostPickupCallback != Py_None && !PyCallable_Check (boostPickupCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid boost pickup callback");
		return nullptr;
	}

	if (carBumpCallback && carBumpCallback != Py_None && !PyCallable_Check (carBumpCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid car bump callback");
		return nullptr;
	}

	if (carDemoCallback && carDemoCallback != Py_None && !PyCallable_Check (carDemoCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid car demo callback");
		return nullptr;
	}

	if (goalScoreCallback && goalScoreCallback != Py_None && !PyCallable_Check (goalScoreCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid goal score callback");
		return nullptr;
	}

	if (shotEventCallback && shotEventCallback != Py_None && !PyCallable_Check (shotEventCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid shot event callback");
		return nullptr;
	}

	if (goalEventCallback && goalEventCallback != Py_None && !PyCallable_Check (goalEventCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid goal event callback");
		return nullptr;
	}

	if (saveEventCallback && saveEventCallback != Py_None && !PyCallable_Check (saveEventCallback))
	{
		PyErr_SetString (PyExc_ValueError, "Invalid save event callback");
		return nullptr;
	}

	try
	{
		// default initialization if it hasn't been done yet
		InitInternal (nullptr);
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return nullptr;
	}

	try
	{
		auto arena = std::shared_ptr<::Arena> (::Arena::Create (
		    static_cast<::GameMode> (gameMode), static_cast<::ArenaMemWeightMode> (memoryWeightMode), 1.0f / tickTime));
		if (!arena)
			return PyErr_NoMemory ();

		if (mutatorConfig)
			arena->SetMutatorConfig (MutatorConfig::ToMutatorConfig (PyCast<MutatorConfig> (mutatorConfig)));

		arena->tickTime  = tickTime;
		arena->tickCount = tickCount;

		if (ballState)
		{
			arena->ball->SetState (BallState::ToBallState (PyCast<BallState> (ballState)));
			arena->ball->_internalState.updateCounter = PyCast<BallState> (ballState)->state.updateCounter;
		}

		auto carMap        = std::map<std::uint32_t, PyRef<Car>>{};
		auto const numCars = PyList_Size (cars);
		for (unsigned i = 0; i < numCars; ++i)
		{
			auto car = PyRef<Car>::steal (Car::New ());
			if (!car)
				return nullptr;

			auto result = PyObjectRef::steal (Car::InternalUnpickle (arena, car.borrow (), PyList_GetItem (cars, i)));
			if (!result)
				return nullptr;

			carMap[car->car->id] = car;
		}

		auto padMap = std::unordered_map<::BoostPad *, PyRef<BoostPad>>{};
		if (pads)
		{
			auto const &[indexMapping, indexMappingSize] = getIndexMapping (arena->gameMode);

			auto const numPads = PyList_Size (pads);

			if (numPads != indexMappingSize)
				return PyErr_Format (PyExc_KeyError,
				    "Internal mapping error: expected %zu boost pads, got %zu",
				    indexMappingSize,
				    numPads);

			for (unsigned i = 0; i < numPads; ++i)
			{
				auto pad = PyRef<BoostPad>::steal (BoostPad::New ());
				if (!pad)
					return nullptr;

				auto const [x, y] = extractKey (indexMapping[i]);

				for (auto const p : arena->GetBoostPads ())
				{
					if (static_cast<int> (p->pos.x) == x && static_cast<int> (p->pos.y) == y)
					{
						pad->arena = arena;
						pad->pad   = p;
						break;
					}
				}

				if (!pad->pad)
				{
					PyErr_SetString (PyExc_RuntimeError, "Failed to enumerate all boost pads");
					return nullptr;
				}

				auto state = PyList_GetItem (pads, i);
				if (!state)
					return nullptr;

				if (!Py_IS_TYPE (state, BoostPadState::Type))
				{
					PyErr_SetString (PyExc_RuntimeError, "Unexpected type");
					return nullptr;
				}

				pad->pad->SetState (BoostPadState::ToBoostPadState (PyCast<BoostPadState> (state)));

				padMap[pad->pad] = pad;
			}
		}

		auto ball = PyRef<Ball>::steal (Ball::New ());
		if (!ball)
			return PyErr_NoMemory ();

		// no exceptions thrown after this point
		arena->_lastCarID = lastCarID;

		if (self_->arena)
		{
			self_->arena->SetBallTouchCallback (nullptr, nullptr);
			self_->arena->SetBoostPickupCallback (nullptr, nullptr);
			self_->arena->SetCarBumpCallback (nullptr, nullptr);
			self_->arena->SetGoalScoreCallback (nullptr, nullptr);
		}

		self_->arena = std::move (arena);

		PyRef<Ball>::assign (self_->ball, ball.borrowObject ());
		self_->ball->arena = self_->arena;
		self_->ball->ball  = self_->arena->ball;

		*self_->cars      = std::move (carMap);
		*self_->boostPads = std::move (padMap);

		self_->blueScore        = blueScore;
		self_->orangeScore      = orangeScore;
		self_->lastGoalTick     = lastGoalTick;
		self_->lastGymStateTick = lastGymStateTick;

		PyObjectRef::assign (self_->ballTouchCallback, ballTouchCallback);
		PyObjectRef::assign (self_->ballTouchCallbackUserData, ballTouchCallbackUserData);
		PyObjectRef::assign (self_->boostPickupCallback, boostPickupCallback);
		PyObjectRef::assign (self_->boostPickupCallbackUserData, boostPickupCallbackUserData);
		PyObjectRef::assign (self_->carBumpCallback, carBumpCallback);
		PyObjectRef::assign (self_->carBumpCallbackUserData, carBumpCallbackUserData);
		PyObjectRef::assign (self_->carDemoCallback, carDemoCallback);
		PyObjectRef::assign (self_->carDemoCallbackUserData, carDemoCallbackUserData);
		PyObjectRef::assign (self_->goalScoreCallback, goalScoreCallback);
		PyObjectRef::assign (self_->goalScoreCallbackUserData, goalScoreCallbackUserData);
		PyObjectRef::assign (self_->shotEventCallback, shotEventCallback);
		PyObjectRef::assign (self_->shotEventCallbackUserData, shotEventCallbackUserData);
		PyObjectRef::assign (self_->goalEventCallback, goalEventCallback);
		PyObjectRef::assign (self_->goalEventCallbackUserData, goalEventCallbackUserData);
		PyObjectRef::assign (self_->saveEventCallback, saveEventCallback);
		PyObjectRef::assign (self_->saveEventCallbackUserData, saveEventCallbackUserData);

		if (self_->ballTouchCallback != Py_None)
			self_->arena->SetBallTouchCallback (&Arena::HandleBallTouchCallback, self_);

		self_->arena->SetCarBumpCallback (&Arena::HandleCarBumpCallback, self_);

		if (self_->arena->gameMode != ::GameMode::THE_VOID)
		{
			self_->arena->SetBoostPickupCallback (&Arena::HandleBoostPickupCallback, self_);
			self_->arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, self_);
		}

		Py_RETURN_NONE;
	}
	catch (...)
	{
		return PyErr_NoMemory ();
	}
}

PyObject *Arena::Copy (Arena *self_) noexcept
{
	auto args = PyObjectRef::steal (PyTuple_New (1));
	if (!args)
		return nullptr;

	PyTuple_SetItem (args.borrow (), 0, PyBool_FromLong (true));

	return Clone (self_, args.borrow (), nullptr);
}

PyObject *Arena::DeepCopy (Arena *self_, PyObject *memo_) noexcept
{
	auto args = PyObjectRef::steal (PyTuple_New (1));
	if (!args)
		return nullptr;

	PyTuple_SetItem (args.borrow (), 0, PyBool_FromLong (true));

	return Clone (self_, args.borrow (), nullptr);
}

PyObject *Arena::AddCar (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char teamKwd[]   = "team";
	static char configKwd[] = "config";

	static char *dict[] = {teamKwd, configKwd, nullptr};

	int team;
	PyObject *config = nullptr; // borrowed reference
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "i|O", dict, &team, &config))
		return nullptr;

	if (team != static_cast<int> (::Team::BLUE) && team != static_cast<int> (::Team::ORANGE))
	{
		PyErr_SetString (PyExc_RuntimeError, "Invalid team");
		return nullptr;
	}

	::CarConfig carConfig = CAR_CONFIG_OCTANE;
	if (config && Py_IS_TYPE (config, CarConfig::Type))
	{
		carConfig = reinterpret_cast<CarConfig const *> (config)->config;
	}
	else if (config)
	{
		auto const configId = PyLong_AsLong (config);
		if (PyErr_Occurred ())
			return nullptr;

		if (!CarConfig::FromIndex (static_cast<CarConfig::Index> (configId), carConfig))
			return nullptr;
	}

	auto car = PyRef<Car>::steal (Car::New ());
	if (!car)
		return nullptr;

	::Car *rsCar = nullptr;

	try
	{
		rsCar = self_->arena->AddCar (static_cast<::Team> (team), carConfig);

		auto &carRef = (*self_->cars)[rsCar->id];
		if (carRef) // this probably shouldn't happen
			throw 0;

		// no exceptions thrown after this point
		car->arena = self_->arena;
		car->car   = rsCar;
		carRef     = car;

		return car.giftObject ();
	}
	catch (...)
	{
		if (rsCar)
			self_->arena->RemoveCar (rsCar);

		return PyErr_NoMemory ();
	}
}

PyObject *Arena::Clone (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char copyKwd[] = "copy_callbacks";

	static char *dict[] = {copyKwd, nullptr};

	int copyCallbacks = false;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|p", dict, &copyCallbacks))
		return nullptr;

	auto clone = PyRef<Arena>::stealObject (New (Type, nullptr, nullptr));
	if (!clone)
		return nullptr;

	try
	{
		auto arena = std::shared_ptr<::Arena> (self_->arena->Clone (false));
		if (!arena)
			return PyErr_NoMemory ();

		auto ball = PyRef<Ball>::steal (Ball::New ());
		if (!ball)
			return PyErr_NoMemory ();

		auto boostPads = std::unordered_map<::BoostPad *, PyRef<BoostPad>>{};
		for (auto const &pad : arena->GetBoostPads ())
		{
			auto ref = PyRef<BoostPad>::steal (BoostPad::New ());
			if (!ref)
				return PyErr_NoMemory ();

			ref->pad = pad;
			boostPads.emplace (pad, std::move (ref));
		}

		auto cars = std::map<std::uint32_t, PyRef<Car>>{};
		for (auto const &car : arena->GetCars ())
		{
			auto &carRef = cars[car->id];

			carRef = PyRef<Car>::steal (Car::New ());
			if (!carRef)
				return PyErr_NoMemory ();

			carRef->arena = arena;
			carRef->car   = car;

			auto it = self_->cars->find (car->id);
			if (it != std::end (*self_->cars))
			{
				carRef->goals        = it->second->goals;
				carRef->demos        = it->second->demos;
				carRef->boostPickups = it->second->boostPickups;
				carRef->shots        = it->second->shots;
				carRef->saves        = it->second->saves;
				carRef->assists      = it->second->assists;
			}
			else
			{
				carRef->goals        = 0;
				carRef->demos        = 0;
				carRef->boostPickups = 0;
				carRef->shots        = 0;
				carRef->saves        = 0;
				carRef->assists      = 0;
			}
		}

		// no exceptions thrown after this point
		clone->arena      = arena;
		clone->threadPool = self_->threadPool;
		*clone->boostPads = std::move (boostPads);
		*clone->cars      = std::move (cars);

		PyRef<Ball>::assign (clone->ball, ball.borrowObject ());
		clone->ball->arena = clone->arena;
		clone->ball->ball  = clone->arena->ball;

		if (copyCallbacks)
		{
			PyObjectRef::assign (clone->ballTouchCallback, self_->ballTouchCallback);
			PyObjectRef::assign (clone->ballTouchCallbackUserData, self_->ballTouchCallbackUserData);
			PyObjectRef::assign (clone->boostPickupCallback, self_->boostPickupCallback);
			PyObjectRef::assign (clone->boostPickupCallbackUserData, self_->boostPickupCallbackUserData);
			PyObjectRef::assign (clone->carBumpCallback, self_->carBumpCallback);
			PyObjectRef::assign (clone->carBumpCallbackUserData, self_->carBumpCallbackUserData);
			PyObjectRef::assign (clone->carDemoCallback, self_->carDemoCallback);
			PyObjectRef::assign (clone->carDemoCallbackUserData, self_->carDemoCallbackUserData);
			PyObjectRef::assign (clone->goalScoreCallback, self_->goalScoreCallback);
			PyObjectRef::assign (clone->goalScoreCallbackUserData, self_->goalScoreCallbackUserData);
			PyObjectRef::assign (clone->shotEventCallback, self_->shotEventCallback);
			PyObjectRef::assign (clone->shotEventCallbackUserData, self_->shotEventCallbackUserData);
			PyObjectRef::assign (clone->goalEventCallback, self_->goalEventCallback);
			PyObjectRef::assign (clone->goalEventCallbackUserData, self_->goalEventCallbackUserData);
			PyObjectRef::assign (clone->saveEventCallback, self_->saveEventCallback);
			PyObjectRef::assign (clone->saveEventCallbackUserData, self_->saveEventCallbackUserData);
		}
		else
		{
			PyObjectRef::assign (clone->ballTouchCallback, Py_None);
			PyObjectRef::assign (clone->ballTouchCallbackUserData, Py_None);
			PyObjectRef::assign (clone->boostPickupCallback, Py_None);
			PyObjectRef::assign (clone->boostPickupCallbackUserData, Py_None);
			PyObjectRef::assign (clone->carBumpCallback, Py_None);
			PyObjectRef::assign (clone->carBumpCallbackUserData, Py_None);
			PyObjectRef::assign (clone->carDemoCallback, Py_None);
			PyObjectRef::assign (clone->carDemoCallbackUserData, Py_None);
			PyObjectRef::assign (clone->goalScoreCallback, Py_None);
			PyObjectRef::assign (clone->goalScoreCallbackUserData, Py_None);
			PyObjectRef::assign (clone->shotEventCallback, Py_None);
			PyObjectRef::assign (clone->shotEventCallbackUserData, Py_None);
			PyObjectRef::assign (clone->goalEventCallback, Py_None);
			PyObjectRef::assign (clone->goalEventCallbackUserData, Py_None);
			PyObjectRef::assign (clone->saveEventCallback, Py_None);
			PyObjectRef::assign (clone->saveEventCallbackUserData, Py_None);
		}

		if (clone->ballTouchCallback != Py_None)
			clone->arena->SetBallTouchCallback (&Arena::HandleBallTouchCallback, clone.borrow ());

		clone->arena->SetCarBumpCallback (&Arena::HandleCarBumpCallback, clone.borrow ());

		if (clone->arena->gameMode != ::GameMode::THE_VOID)
		{
			clone->arena->SetBoostPickupCallback (&Arena::HandleBoostPickupCallback, clone.borrow ());
			clone->arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, clone.borrow ());
		}

		clone->blueScore        = self_->blueScore;
		clone->orangeScore      = self_->orangeScore;
		clone->lastGoalTick     = self_->lastGoalTick;
		clone->lastGymStateTick = self_->lastGymStateTick;

		return clone.giftObject ();
	}
	catch (...)
	{
		return PyErr_NoMemory ();
	}
}

PyObject *Arena::CloneInto (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char targetKwd[] = "target";
	static char copyKwd[]   = "copy_callbacks";

	static char *dict[] = {targetKwd, copyKwd, nullptr};

	Arena *target;
	int copyCallbacks = false;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O!|p", dict, Type, &target, &copyCallbacks))
		return nullptr;

	if (self_->arena->gameMode != target->arena->gameMode)
	{
		PyErr_SetString (PyExc_ValueError, "Game mode mismatch");
		return nullptr;
	}

	auto it  = std::begin (*self_->cars);
	auto it2 = std::begin (*target->cars);

	while (it != std::end (*self_->cars) && it2 != std::end (*target->cars))
	{
		auto const carA = (it++)->second;
		auto const carB = (it2++)->second;

		if (carA->car->id != carB->car->id || carA->car->team != carB->car->team)
		{
			PyErr_SetString (PyExc_ValueError, "Car id mismatch");
			return nullptr;
		}
	}

	if (it != std::end (*self_->cars) || it2 != std::end (*target->cars))
	{
		PyErr_SetString (PyExc_ValueError, "Car list mismatch");
		return nullptr;
	}

	if (!ensureBoostPadByIndex (self_) || !ensureBoostPadByIndex (target))
		return nullptr;

	if (static_cast<bool> (target->boostPadsByIndex) != static_cast<bool> (self_->boostPadsByIndex))
	{
		PyErr_SetString (PyExc_ValueError, "Boost pad list mismatch");
		return nullptr;
	}

	if (self_->boostPadsByIndex)
	{
		auto const padsCount = self_->boostPadsByIndex->size ();
		assert (target->boostPadsByIndex->size () == padsCount);

		auto const &padsA = *self_->boostPadsByIndex;
		auto const &padsB = *target->boostPadsByIndex;

		for (unsigned i = 0; i < padsCount; ++i)
		{
			if (padsB[i]->pad->isBig != padsA[i]->pad->isBig || padsB[i]->pad->pos != padsA[i]->pad->pos)
			{
				PyErr_SetString (PyExc_ValueError, "Boost pad mismatch");
				return nullptr;
			}
		}
	}

	// everything is 1-to-1, now we can start setting states

	target->ball->ball->SetState (self_->ball->ball->GetState ());
	target->ball->ball->_internalState.updateCounter = self_->ball->ball->_internalState.updateCounter;

	if (self_->boostPadsByIndex)
	{
		auto const padsCount = self_->boostPadsByIndex->size ();
		assert (target->boostPadsByIndex->size () == padsCount);

		auto const &padsA = *self_->boostPadsByIndex;
		auto const &padsB = *target->boostPadsByIndex;

		for (unsigned i = 0; i < padsCount; ++i)
			padsB[i]->pad->SetState (padsA[i]->pad->GetState ());
	}

	it  = std::begin (*self_->cars);
	it2 = std::begin (*target->cars);

	while (it != std::end (*self_->cars) && it2 != std::end (*target->cars))
	{
		auto carA = (it++)->second;
		auto carB = (it2++)->second;

		carB->car->SetState (carA->car->GetState ());
		carB->car->_internalState.updateCounter = carA->car->_internalState.updateCounter;

		carB->car->controls = carA->car->controls;

		carB->goals        = carA->goals;
		carB->demos        = carA->demos;
		carB->boostPickups = carA->boostPickups;
		carB->shots        = carA->shots;
		carB->saves        = carA->saves;
		carB->assists      = carA->assists;
	}

	if (copyCallbacks)
	{
		PyObjectRef::assign (target->ballTouchCallback, self_->ballTouchCallback);
		PyObjectRef::assign (target->ballTouchCallbackUserData, self_->ballTouchCallbackUserData);
		PyObjectRef::assign (target->boostPickupCallback, self_->boostPickupCallback);
		PyObjectRef::assign (target->boostPickupCallbackUserData, self_->boostPickupCallbackUserData);
		PyObjectRef::assign (target->carBumpCallback, self_->carBumpCallback);
		PyObjectRef::assign (target->carBumpCallbackUserData, self_->carBumpCallbackUserData);
		PyObjectRef::assign (target->carDemoCallback, self_->carDemoCallback);
		PyObjectRef::assign (target->carDemoCallbackUserData, self_->carDemoCallbackUserData);
		PyObjectRef::assign (target->goalScoreCallback, self_->goalScoreCallback);
		PyObjectRef::assign (target->goalScoreCallbackUserData, self_->goalScoreCallbackUserData);
		PyObjectRef::assign (target->shotEventCallback, self_->shotEventCallback);
		PyObjectRef::assign (target->shotEventCallbackUserData, self_->shotEventCallbackUserData);
		PyObjectRef::assign (target->goalEventCallback, self_->goalEventCallback);
		PyObjectRef::assign (target->goalEventCallbackUserData, self_->goalEventCallbackUserData);
		PyObjectRef::assign (target->saveEventCallback, self_->saveEventCallback);
		PyObjectRef::assign (target->saveEventCallbackUserData, self_->saveEventCallbackUserData);
	}
	else
	{
		PyObjectRef::assign (target->ballTouchCallback, Py_None);
		PyObjectRef::assign (target->ballTouchCallbackUserData, Py_None);
		PyObjectRef::assign (target->boostPickupCallback, Py_None);
		PyObjectRef::assign (target->boostPickupCallbackUserData, Py_None);
		PyObjectRef::assign (target->carBumpCallback, Py_None);
		PyObjectRef::assign (target->carBumpCallbackUserData, Py_None);
		PyObjectRef::assign (target->carDemoCallback, Py_None);
		PyObjectRef::assign (target->carDemoCallbackUserData, Py_None);
		PyObjectRef::assign (target->goalScoreCallback, Py_None);
		PyObjectRef::assign (target->goalScoreCallbackUserData, Py_None);
		PyObjectRef::assign (target->shotEventCallback, Py_None);
		PyObjectRef::assign (target->shotEventCallbackUserData, Py_None);
		PyObjectRef::assign (target->goalEventCallback, Py_None);
		PyObjectRef::assign (target->goalEventCallbackUserData, Py_None);
		PyObjectRef::assign (target->saveEventCallback, Py_None);
		PyObjectRef::assign (target->saveEventCallbackUserData, Py_None);
	}

	if (target->ballTouchCallback == Py_None)
		target->arena->SetBallTouchCallback (nullptr, nullptr);
	else
		target->arena->SetBallTouchCallback (&Arena::HandleBallTouchCallback, target);

	target->arena->SetCarBumpCallback (&Arena::HandleCarBumpCallback, target);

	if (target->arena->gameMode != ::GameMode::THE_VOID)
	{
		target->arena->SetBoostPickupCallback (&Arena::HandleBoostPickupCallback, target);
		target->arena->SetGoalScoreCallback (&Arena::HandleGoalScoreCallback, target);
	}

	target->arena->_lastCarID = self_->arena->_lastCarID;
	target->arena->tickTime   = self_->arena->tickTime;
	target->arena->tickCount  = self_->arena->tickCount;

	target->blueScore        = self_->blueScore;
	target->orangeScore      = self_->orangeScore;
	target->lastGoalTick     = self_->lastGoalTick;
	target->lastGymStateTick = self_->lastGymStateTick;

	// how expensive?
	target->arena->SetMutatorConfig (self_->arena->GetMutatorConfig ());

	Py_RETURN_NONE;
}

PyObject *Arena::GetCarFromId (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char carIdKwd[]   = "car_id";
	static char defaultKwd[] = "default";

	static char *dict[] = {carIdKwd, defaultKwd, nullptr};

	unsigned id;
	PyObject *defaultResult = nullptr; // borrowed reference
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "I|O", dict, &id, &defaultResult))
		return nullptr;

	auto it = self_->cars->find (id);
	if (it == std::end (*self_->cars) || !it->second)
	{
		if (!defaultResult)
			return PyErr_Format (PyExc_KeyError, "%u", id);

		return PyObjectRef::incRef (defaultResult).giftObject ();
	}

	return it->second.newObjectRef ();
}

PyObject *Arena::GetCars (Arena *self_) noexcept
{
	auto list = PyObjectRef::steal (PyList_New (self_->cars->size ()));
	if (!list)
		return nullptr;

	unsigned index = 0;
	for (auto &[id, car] : *self_->cars)
	{
		// steals ref
		if (PyList_SetItem (list.borrow (), index++, car.newObjectRef ()) < 0)
			return nullptr;
	}

	return list.gift ();
}

PyObject *Arena::GetBallPrediction (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char numTicksKwd[] = "num_ticks";
	static char tickSkipKwd[] = "tick_skip";

	static char *dict[] = {numTicksKwd, tickSkipKwd, nullptr};

	unsigned numTicks = 120;
	unsigned tickSkip = 1;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|II", dict, &numTicks, &tickSkip))
		return nullptr;

	if (!self_->ballPrediction)
	{
		try
		{
			self_->ballPrediction = new (std::nothrow)::BallPredTracker (self_->arena.get (), numTicks);
			if (!self_->ballPrediction)
			{
				PyErr_NoMemory ();
				return nullptr;
			}
		}
		catch (std::exception const &err)
		{
			PyErr_SetString (PyExc_RuntimeError, err.what ());
			return nullptr;
		}
	}

	auto const count = numTicks / (tickSkip + 1);

	auto list = PyObjectRef::steal (PyList_New (count));
	if (!list)
		return nullptr;

	if (count == 0)
		return list.gift ();

	try
	{
		if (self_->ballPrediction->predData.capacity () < numTicks)
			self_->ballPrediction->predData.reserve (numTicks);

		self_->ballPrediction->numPredTicks = numTicks;

		self_->ballPrediction->UpdatePred (self_->arena.get ());

		unsigned index = 0;
		for (unsigned i = 0; i < self_->ballPrediction->predData.size (); i += tickSkip + 1)
		{
			auto time =
			    PyObjectRef::stealObject (PyFloat_FromDouble ((self_->arena->tickCount + i) * self_->arena->tickTime));
			auto state = BallState::NewFromBallState (self_->ballPrediction->predData[i]);

			auto tuple = PyObjectRef::steal (PyTuple_New (2));
			if (!tuple)
				return nullptr;

			PyTuple_SetItem (tuple.borrow (), 0, time.giftObject ());
			PyTuple_SetItem (tuple.borrow (), 1, state.giftObject ());

			// steals ref
			if (PyList_SetItem (list.borrow (), index++, tuple.newObjectRef ()) < 0)
				return nullptr;
		}
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return nullptr;
	}

	return list.gift ();
}

PyObject *Arena::GetBoostPads (Arena *self_) noexcept
{
	if (!ensureBoostPadByIndex (self_))
		return nullptr;

	assert (!self_->boostPadsByIndex || self_->boostPads->size () == self_->boostPadsByIndex->size ());

	auto list = PyObjectRef::steal (PyList_New (self_->boostPads->size ()));
	if (!list)
		return nullptr;

	if (self_->boostPadsByIndex)
	{
		unsigned index = 0;
		for (auto &pad : *self_->boostPadsByIndex)
		{
			// steals ref
			if (PyList_SetItem (list.borrow (), index++, pad.newObjectRef ()) < 0)
				return nullptr;
		}
	}

	return list.gift ();
}

PyObject *Arena::GetGymState (Arena *self_) noexcept
{
	auto tuple = PyObjectRef::steal (PyTuple_New (3 + self_->cars->size ()));
	if (!tuple)
		return nullptr;

	std::uint64_t ballLastCarHitTick = 0;
	std::uint32_t ballLastCarHitId   = 0;
	for (auto const &[id, car] : *self_->cars)
	{
		if (!car->car->_internalState.ballHitInfo.isValid)
			continue;

		if (car->car->_internalState.ballHitInfo.tickCountWhenHit <= ballLastCarHitTick)
			continue;

		ballLastCarHitTick = car->car->_internalState.ballHitInfo.tickCountWhenHit;
		ballLastCarHitId   = car->car->id;
	}

	{
		auto gameData = PyArrayRef (4);
		if (!gameData)
			return nullptr;

		gameData (0) = static_cast<int> (self_->arena->gameMode);
		gameData (1) = ballLastCarHitId;
		gameData (2) = self_->blueScore;
		gameData (3) = self_->orangeScore;

		PyTuple_SetItem (tuple.borrow (), 0, gameData.giftObject ());
	}

	{
		auto const &boostPads   = self_->arena->GetBoostPads ();
		auto const numBoostPads = self_->arena->GetBoostPads ().size ();

		auto boostPadState = PyArrayRef (2, numBoostPads);
		if (!boostPadState)
			return nullptr;

		auto const &boostMapping = getBoostMapping (self_->arena->gameMode);
		for (unsigned i = 0; i < numBoostPads; ++i)
		{
			auto const &pad = boostPads[i];

			auto const idx = getBoostPadIndex (pad, boostMapping);
			if (idx < 0)
				continue; // shouldn't happen

			auto const inv = numBoostPads - idx - 1;

			boostPadState (0, idx) = pad->_internalState.isActive;
			boostPadState (1, inv) = pad->_internalState.isActive;
		}

		PyTuple_SetItem (tuple.borrow (), 1, boostPadState.giftObject ());
	}

	{
		auto ballState = PyArrayRef (2, 25);
		if (!ballState)
			return nullptr;

		auto const ball = self_->arena->ball;

		assign (ballState, 0, ball->_rigidBody);
		if (ballState.isnan ())
		{
			PyErr_SetString (PyExc_RuntimeError, R"(!!DETECTED NaN VALUE IN BALL DATA!!
DID YOU STATE SET MULTIPLE OBJECTS IN THE SAME LOCATION?)");
			return nullptr;
		}

		PyTuple_SetItem (tuple.borrow (), 2, ballState.giftObject ());
	}

	unsigned carIndex = 0;
	for (auto const &[id, car] : *self_->cars)
	{
		auto carState = PyArrayRef (2, 36);
		if (!carState)
			return nullptr;

		auto const &state = car->car->_internalState;

		auto const hitLastStep =
		    state.ballHitInfo.isValid && state.ballHitInfo.tickCountWhenHit >= self_->lastGymStateTick;

		for (unsigned i = 0; i < 2; ++i)
		{
			carState (i, 0)  = car->car->id;
			carState (i, 1)  = static_cast<int> (car->car->team);
			carState (i, 2)  = car->goals;
			carState (i, 3)  = car->saves;
			carState (i, 4)  = car->shots;
			carState (i, 5)  = car->demos;
			carState (i, 6)  = car->boostPickups;
			carState (i, 7)  = state.isDemoed;
			carState (i, 8)  = state.isOnGround;
			carState (i, 9)  = hitLastStep;
			carState (i, 10) = state.boost;
		}

		assign (carState, 11, car->car->_rigidBody, state.isDemoed ? &car->demoState : nullptr);
		if (carState.isnan ())
		{
			PyErr_SetString (PyExc_RuntimeError, R"(!!DETECTED NaN VALUE IN CAR DATA!!
DID YOU STATE SET MULTIPLE OBJECTS IN THE SAME LOCATION?)");
			return nullptr;
		}

		PyTuple_SetItem (tuple.borrow (), 3 + carIndex, carState.giftObject ());
		++carIndex;
	}

	self_->lastGymStateTick = self_->arena->tickCount;

	return tuple.giftObject ();
}

PyObject *Arena::GetMutatorConfig (Arena *self_) noexcept
{
	auto config = MutatorConfig::NewFromMutatorConfig (self_->arena->GetMutatorConfig ());
	if (!config)
		return nullptr;

	return config.giftObject ();
}

PyObject *Arena::RemoveCar (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char carKwd[] = "car";

	static char *dict[] = {carKwd, nullptr};

	PyObject *carOrId;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O", dict, &carOrId))
		return nullptr;

	Car *car;

	if (Py_IS_TYPE (carOrId, Car::Type))
		car = PyCast<Car> (carOrId);
	else
	{
		auto const id = PyLong_AsLong (carOrId);
		if (PyErr_Occurred ())
			return nullptr;

		auto it = self_->cars->find (id);
		if (it == std::end (*self_->cars))
		{
			PyErr_SetString (PyExc_KeyError, "Car not found in this arena");
			return nullptr;
		}

		car = it->second.borrow ();
	}

	if (car->arena != self_->arena)
	{
		PyErr_SetString (PyExc_RuntimeError, "This car is not in this arena");
		return nullptr;
	}

	self_->cars->erase (car->car->id);
	self_->arena->RemoveCar (car->car);
	car->car = nullptr;

	// detach car from arena
	car->arena.reset ();

	Py_RETURN_NONE;
}

PyObject *Arena::IsBallProbablyGoingIn (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char maxTimeKwd[] = "max_time";

	static char *dict[] = {maxTimeKwd, nullptr};

	float maxTime = 0.2f;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|f", dict, &maxTime))
		return nullptr;

	return PyBool_FromLong (self_->arena->IsBallProbablyGoingIn (maxTime));
}

PyObject *Arena::ResetKickoff (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char seedKwd[] = "seed";

	static char *dict[] = {seedKwd, nullptr};

	int seed = -1;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|i", dict, &seed))
		return nullptr;

	self_->arena->ResetToRandomKickoff (seed);

	Py_RETURN_NONE;
}

PyObject *Arena::SetBallTouchCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback == Py_None)
		self_->arena->SetBallTouchCallback (nullptr, nullptr);
	else if (PyCallable_Check (callback))
		self_->arena->SetBallTouchCallback (&Arena::HandleBallTouchCallback, self_);
	else
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->ballTouchCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->ballTouchCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetBoostPickupCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (self_->arena->gameMode == ::GameMode::THE_VOID)
	{
		PyErr_SetString (PyExc_RuntimeError, "Cannot set a boost pickup callback when on THE_VOID gamemode!");
		return nullptr;
	}

	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->boostPickupCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->boostPickupCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetCarBallCollision (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char enableKwd[] = "enable";

	static char *dict[] = {enableKwd, nullptr};

	int enable = true;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|p", dict, &enable))
		return nullptr;

	self_->arena->SetCarBallCollision (enable);

	Py_RETURN_NONE;
}

PyObject *Arena::SetCarBumpCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->carBumpCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->carBumpCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetCarCarCollision (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char enableKwd[] = "enable";

	static char *dict[] = {enableKwd, nullptr};

	int enable = true;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|p", dict, &enable))
		return nullptr;

	self_->arena->SetCarCarCollision (enable);

	Py_RETURN_NONE;
}

PyObject *Arena::SetCarDemoCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->carDemoCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->carDemoCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetGoalScoreCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (self_->arena->gameMode == ::GameMode::THE_VOID)
	{
		PyErr_SetString (PyExc_RuntimeError, "Cannot set a goal score callback when on THE_VOID gamemode!");
		return nullptr;
	}

	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->goalScoreCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->goalScoreCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetShotEventCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (!self_->gameEvent)
	{
		PyErr_Format (PyExc_RuntimeError,
		    "Cannot set a shot event callback when on %s gamemode!",
		    GAMEMODE_STRS[static_cast<int> (self_->arena->gameMode)]);
		return nullptr;
	}

	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->shotEventCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->shotEventCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetGoalEventCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (!self_->gameEvent)
	{
		PyErr_Format (PyExc_RuntimeError,
		    "Cannot set a goal event callback when on %s gamemode!",
		    GAMEMODE_STRS[static_cast<int> (self_->arena->gameMode)]);
		return nullptr;
	}

	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->goalEventCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->goalEventCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetSaveEventCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	if (!self_->gameEvent)
	{
		PyErr_Format (PyExc_RuntimeError,
		    "Cannot set a save event callback when on %s gamemode!",
		    GAMEMODE_STRS[static_cast<int> (self_->arena->gameMode)]);
		return nullptr;
	}

	static char callbackKwd[] = "callback";
	static char dataKwd[]     = "data";

	static char *dict[] = {callbackKwd, dataKwd, nullptr};

	PyObject *callback; // borrowed references
	PyObject *userData = Py_None;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O|O", dict, &callback, &userData))
		return nullptr;

	if (callback != Py_None && !PyCallable_Check (callback))
	{
		PyErr_SetString (PyExc_RuntimeError, "First parameter must be a callable object or None");
		return nullptr;
	}

	auto prev = PyObjectRef::steal (PyTuple_Pack (2, callback, userData));
	if (!prev)
		return nullptr;

	PyObjectRef::assign (self_->saveEventCallback, PyTuple_GetItem (prev.borrow (), 0));
	PyObjectRef::assign (self_->saveEventCallbackUserData, PyTuple_GetItem (prev.borrow (), 1));

	return prev.giftObject ();
}

PyObject *Arena::SetMutatorConfig (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char configKwd[] = "config";

	static char *dict[] = {configKwd, nullptr};

	MutatorConfig *config;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "O!", dict, MutatorConfig::Type, &config))
		return nullptr;

	self_->arena->SetMutatorConfig (MutatorConfig::ToMutatorConfig (config));

	Py_RETURN_NONE;
}

PyObject *Arena::Step (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char ticksKwd[] = "ticks";

	static char *dict[] = {ticksKwd, nullptr};

	int ticksToSimulate = 1;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|i", dict, &ticksToSimulate))
		return nullptr;

	Py_BEGIN_ALLOW_THREADS;
	self_->arena->Step (ticksToSimulate);
	Py_END_ALLOW_THREADS;

	if (self_->gameEvent)
		self_->gameEvent->Update (self_->arena.get ());

	if (self_->stepExceptionType)
	{
		restoreException (self_);
		return nullptr;
	}

	Py_RETURN_NONE;
}

PyObject *Arena::Stop (Arena *self_) noexcept
{
	self_->arena->Stop ();

	Py_RETURN_NONE;
}

PyObject *Arena::MultiStep (PyObject *dummy_, PyObject *args_, PyObject *kwds_) noexcept
{
	static char arenasKwd[] = "arenas";
	static char ticksKwd[]  = "ticks";

	static char *dict[] = {arenasKwd, ticksKwd, nullptr};

	PyObject *arenas    = nullptr;
	int ticksToSimulate = 1;
	if (!PyArg_ParseTupleAndKeywords (args_, kwds_, "|O!i", dict, &PyList_Type, &arenas, &ticksToSimulate))
		return nullptr;

	auto const count = PyList_Size (arenas);
	if (count == 0)
		Py_RETURN_NONE;

	std::set<PyRef<Arena>> jobs;

	auto pool = Arena::ThreadPool::GetInstance ();
	if (!pool)
	{
		PyErr_SetString (PyExc_RuntimeError, "Failed to create thread pool");
		return nullptr;
	}

	try
	{
		for (unsigned i = 0; i < count; ++i)
		{
			auto obj = PyList_GetItem (arenas, i);
			if (!obj)
				return nullptr;

			if (!Py_IS_TYPE (obj, Arena::Type))
			{
				PyErr_SetString (PyExc_RuntimeError, "Unexpected type");
				return nullptr;
			}

			auto arena = PyRef<Arena>::incObjectRef (obj);
			if (!jobs.insert (arena).second)
			{
				PyErr_SetString (PyExc_RuntimeError, "Duplicate arena detected");
				return nullptr;
			}

			arena->threadPool = pool;
		}
	}
	catch (std::exception const &err)
	{
		PyErr_SetString (PyExc_RuntimeError, err.what ());
		return nullptr;
	}

	bool ok;
	Py_BEGIN_ALLOW_THREADS;
	ok = pool->SubmitJob (jobs, ticksToSimulate);
	Py_END_ALLOW_THREADS;

	if (!ok)
		return nullptr;

	auto raised = false;
	for (auto &arena : jobs)
	{
		if (arena->stepExceptionType)
		{
			restoreException (arena.borrow ());
			raised = true;
			break;
		}
	}

	if (!raised)
		Py_RETURN_NONE;

	for (auto &arena : jobs)
		discardException (arena.borrow ());

	return nullptr;
}

void Arena::HandleBallTouchCallback (::Arena *arena_, ::Car *car_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepExceptionType)
		return;

	if (self->ballTouchCallback == Py_None)
		return;

	auto car = PyRef<Car>::incObjectRef (Py_None);
	if (car_)
	{
		auto it = self->cars->find (car_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", car_->id);
			saveException (self);
			return;
		}

		car = it->second;
	}

	auto const gil = GIL{};

	auto args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
	{
		saveException (self);
		return;
	}

	auto kwds = PyObjectRef::steal (
	    Py_BuildValue ("{sOsOsO}", "arena", self, "car", car.borrow (), "data", self->ballTouchCallbackUserData));
	if (!kwds)
	{
		saveException (self);
		return;
	}

	if (!PyObject_Call (self->ballTouchCallback, args.borrow (), kwds.borrow ()))
	{
		saveException (self);
		return;
	}
}

void Arena::HandleBoostPickupCallback (::Arena *arena_, ::Car *car_, ::BoostPad *boostPad_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepExceptionType)
		return;

	auto car = PyRef<Car>::incObjectRef (Py_None);
	if (car_)
	{
		auto const it = self->cars->find (car_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			// this should never happen
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", car_->id);
			saveException (self);
			return;
		}

		car = it->second;
		++car->boostPickups;
	}

	auto boostPad = PyRef<BoostPad>::incObjectRef (Py_None);
	if (boostPad_)
	{
		auto const it = self->boostPads->find (boostPad_);
		if (it == std::end (*self->boostPads) || !it->second)
		{
			// this should never happen
			auto const gil = GIL{};

			PyErr_SetString (PyExc_KeyError, "Boost pad not found");
			saveException (self);
			return;
		}

		boostPad = it->second;
	}

	if (self->boostPickupCallback == Py_None)
		return;

	auto const gil = GIL{};

	auto args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
	{
		saveException (self);
		return;
	}

	auto kwds = PyObjectRef::steal (Py_BuildValue ("{sOsOsOsO}",
	    "arena",
	    self,
	    "car",
	    car.borrow (),
	    "boost_pad",
	    boostPad.borrowObject (),
	    "data",
	    self->boostPickupCallbackUserData));
	if (!kwds)
	{
		saveException (self);
		return;
	}

	if (!PyObject_Call (self->boostPickupCallback, args.borrow (), kwds.borrow ()))
	{
		saveException (self);
		return;
	}
}

void Arena::HandleCarBumpCallback (::Arena *arena_,
    ::Car *bumper_,
    ::Car *victim_,
    bool isDemo_,
    void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepExceptionType)
		return;

	auto bumper = PyRef<Car>::incObjectRef (Py_None);
	if (bumper_)
	{
		auto const it = self->cars->find (bumper_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", bumper_->id);
			saveException (self);
			return;
		}

		bumper = it->second;
		if (isDemo_)
			++bumper->demos;
	}

	if (self->carBumpCallback == Py_None && !isDemo_)
		return;

	auto victim = PyRef<Car>::incObjectRef (Py_None);
	if (victim_)
	{
		auto const it = self->cars->find (victim_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", victim_->id);
			saveException (self);
			return;
		}

		victim = it->second;
		if (isDemo_)
			victim->demoState = victim->car->GetState ();
	}

	if (self->carBumpCallback == Py_None && (!isDemo_ || self->carDemoCallback == Py_None))
		return;

	auto const gil = GIL{};

	if (self->carBumpCallback != Py_None)
	{
		auto args = PyObjectRef::steal (PyTuple_New (0));
		if (!args)
		{
			saveException (self);
			return;
		}

		auto kwds = PyObjectRef::steal (Py_BuildValue ("{sOsOsOsOsO}",
		    "arena",
		    self,
		    "bumper",
		    bumper.borrow (),
		    "victim",
		    victim.borrowObject (),
		    "is_demo",
		    PyBool_FromLong (isDemo_),
		    "data",
		    self->carBumpCallbackUserData));
		if (!kwds)
		{
			saveException (self);
			return;
		}

		if (!PyObject_Call (self->carBumpCallback, args.borrow (), kwds.borrow ()))
		{
			saveException (self);
			return;
		}
	}

	if (isDemo_ && self->carDemoCallback != Py_None)
	{
		auto args = PyObjectRef::steal (PyTuple_New (0));
		if (!args)
		{
			saveException (self);
			return;
		}

		auto kwds = PyObjectRef::steal (Py_BuildValue ("{sOsOsOsO}",
		    "arena",
		    self,
		    "bumper",
		    bumper.borrow (),
		    "victim",
		    victim.borrowObject (),
		    "data",
		    self->carDemoCallbackUserData));
		if (!kwds)
		{
			saveException (self);
			return;
		}

		if (!PyObject_Call (self->carDemoCallback, args.borrow (), kwds.borrow ()))
		{
			saveException (self);
			return;
		}
	}
}

void Arena::HandleGoalScoreCallback (::Arena *arena_, ::Team scoringTeam_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepExceptionType)
		return;

	// avoid continuously counting goals until the ball exits goal zone
	if (self->lastGoalTick + 1 != self->arena->tickCount)
	{
		if (scoringTeam_ == ::Team::BLUE)
			++self->blueScore;
		else
			++self->orangeScore;
	}

	self->lastGoalTick = self->arena->tickCount;

	if (self->goalScoreCallback == Py_None)
		return;

	auto const gil = GIL{};

	auto const team = PyObjectRef::steal (PyLong_FromLong (static_cast<long> (scoringTeam_)));
	if (!team)
	{
		saveException (self);
		return;
	}

	auto args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
	{
		saveException (self);
		return;
	}

	auto kwds = PyObjectRef::steal (
	    Py_BuildValue ("{sOsOsO}", "arena", self, "team", team.borrow (), "data", self->goalScoreCallbackUserData));

	if (!PyObject_Call (self->goalScoreCallback, args.borrow (), kwds.borrow ()))
	{
		saveException (self);
		return;
	}
}

void Arena::HandleShotEventCallback (::Arena *arena_, ::Car *shooter_, ::Car *passer_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepExceptionType)
		return;

	auto shooter = PyRef<Car>::incObjectRef (Py_None);
	if (shooter_)
	{
		auto const it = self->cars->find (shooter_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", shooter_->id);
			saveException (self);
			return;
		}

		shooter = it->second;
		++shooter->shots;
	}

	if (self->shotEventCallback == Py_None)
		return;

	auto passer = PyRef<Car>::incObjectRef (Py_None);
	if (passer_)
	{
		auto const it = self->cars->find (passer_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", passer_->id);
			saveException (self);
			return;
		}

		passer = it->second;
	}

	auto const gil = GIL{};

	auto args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
	{
		saveException (self);
		return;
	}

	auto kwds = PyObjectRef::steal (Py_BuildValue ("{sOsOsOsO}",
	    "arena",
	    self,
	    "shooter",
	    shooter.borrow (),
	    "passer",
	    passer.borrow (),
	    "data",
	    self->shotEventCallbackUserData));

	if (!PyObject_Call (self->shotEventCallback, args.borrow (), kwds.borrow ()))
	{
		saveException (self);
		return;
	}
}

void Arena::HandleGoalEventCallback (::Arena *arena_, ::Car *shooter_, ::Car *passer_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepExceptionType)
		return;

	auto shooter = PyRef<Car>::incObjectRef (Py_None);
	if (shooter_)
	{
		auto const it = self->cars->find (shooter_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", shooter_->id);
			saveException (self);
			return;
		}

		shooter = it->second;
		++shooter->goals;
	}

	auto passer = PyRef<Car>::incObjectRef (Py_None);
	if (passer_)
	{
		auto const it = self->cars->find (passer_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", passer_->id);
			saveException (self);
			return;
		}

		passer = it->second;
		++passer->assists;
	}

	if (self->goalEventCallback == Py_None)
		return;

	auto const gil = GIL{};

	auto args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
	{
		saveException (self);
		return;
	}

	auto kwds = PyObjectRef::steal (Py_BuildValue ("{sOsOsOsO}",
	    "arena",
	    self,
	    "shooter",
	    shooter.borrow (),
	    "passer",
	    passer.borrow (),
	    "data",
	    self->goalEventCallbackUserData));

	if (!PyObject_Call (self->goalEventCallback, args.borrow (), kwds.borrow ()))
	{
		saveException (self);
		return;
	}
}

void Arena::HandleSaveEventCallback (::Arena *arena_, ::Car *saver_, void *userData_) noexcept
{
	auto const self = reinterpret_cast<Arena *> (userData_);
	if (self->stepExceptionType)
		return;

	auto saver = PyRef<Car>::incObjectRef (Py_None);
	if (saver_)
	{
		auto const it = self->cars->find (saver_->id);
		if (it == std::end (*self->cars) || !it->second)
		{
			auto const gil = GIL{};

			PyErr_Format (PyExc_KeyError, "Car with id '%" PRIu32 "' not found", saver_->id);
			saveException (self);
			return;
		}

		saver = it->second;
		++saver->saves;
	}

	if (self->saveEventCallback == Py_None)
		return;

	auto const gil = GIL{};

	auto args = PyObjectRef::steal (PyTuple_New (0));
	if (!args)
	{
		saveException (self);
		return;
	}

	auto kwds = PyObjectRef::steal (
	    Py_BuildValue ("{sOsOsO}", "arena", self, "saver", saver.borrow (), "data", self->saveEventCallbackUserData));

	if (!PyObject_Call (self->saveEventCallback, args.borrow (), kwds.borrow ()))
	{
		saveException (self);
		return;
	}
}

PyObject *Arena::Getgame_mode (Arena *self_, void *) noexcept
{
	return PyLong_FromLong (static_cast<long> (self_->arena->gameMode));
}

PyObject *Arena::Gettick_count (Arena *self_, void *) noexcept
{
	return PyLong_FromUnsignedLongLong (self_->arena->tickCount);
}

PyObject *Arena::Gettick_rate (Arena *self_, void *) noexcept
{
	return PyFloat_FromDouble (self_->arena->GetTickRate ());
}

PyObject *Arena::Gettick_time (Arena *self_, void *) noexcept
{
	return PyFloat_FromDouble (self_->arena->tickTime);
}
}
