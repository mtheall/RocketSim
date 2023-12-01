#pragma once

#include <Python.h>
#include <structmember.h>

#include "PyRef.h"

#include "Math/Math.h"
#include "Sim/Arena/Arena.h"
#include "Sim/BallPredTracker/BallPredTracker.h"
#include "Sim/Car/Car.h"
#include "Sim/GameEventTracker/GameEventTracker.h"

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#ifndef Py_UNREACHABLE
#ifndef NDEBUG
#define Py_UNREACHABLE() Py_FatalError ("Unreachable C code path reached")
#elif defined(_MSC_VER)
#define Py_UNREACHABLE() __assume (0)
#else
#define Py_UNREACHABLE() __builtin_unreachable ()
#endif
#endif

// clang-format off
#ifndef Py_RETURN_RICHCOMPARE
#define Py_RETURN_RICHCOMPARE(val1, val2, op)                               \
    do {                                                                    \
        switch (op) {                                                       \
        case Py_EQ: if ((val1) == (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        case Py_NE: if ((val1) != (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        case Py_LT: if ((val1) < (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;   \
        case Py_GT: if ((val1) > (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;   \
        case Py_LE: if ((val1) <= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        case Py_GE: if ((val1) >= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE;  \
        default:                                                            \
            Py_UNREACHABLE();                                               \
        }                                                                   \
    } while (0)
#endif
// clang-format on

// clang-format off
template <typename T>
struct TypeHelper{};

#define TYPE_HELPER(a_, b_) \
	template<> struct TypeHelper<a_> { constexpr static auto type = b_; }

TYPE_HELPER(short,              T_SHORT);
TYPE_HELPER(int,                T_INT);
TYPE_HELPER(long,               T_LONG);
TYPE_HELPER(float,              T_FLOAT);
TYPE_HELPER(double,             T_DOUBLE);
TYPE_HELPER(char const *,       T_STRING);
TYPE_HELPER(signed char,        T_BYTE);
TYPE_HELPER(unsigned char,      T_UBYTE);
TYPE_HELPER(unsigned short,     T_USHORT);
TYPE_HELPER(unsigned int,       T_UINT);
TYPE_HELPER(unsigned long,      T_ULONG);
TYPE_HELPER(bool,               T_BOOL);
TYPE_HELPER(long long,          T_LONGLONG);
TYPE_HELPER(unsigned long long, T_ULONGLONG);
#undef TYPE_HELPER
// clang-format on

static_assert (sizeof (bool) == sizeof (char));

#define GETSET_ENTRY(type_, member_, doc_)                                                                             \
	{                                                                                                                  \
		.name = #member_, .get = reinterpret_cast<getter> (&type_::Get##member_),                                      \
		.set = reinterpret_cast<setter> (&type_::Set##member_), .doc = doc_, .closure = nullptr                        \
	}

#define GETSET_DECLARE(type_, member_)                                                                                 \
	static PyObject *Get##member_ (type_ *self_, void *) noexcept;                                                     \
	static int Set##member_ (type_ *self_, PyObject *value_, void *) noexcept;

#define GETONLY_ENTRY(type_, member_, doc_)                                                                            \
	{                                                                                                                  \
		.name = #member_, .get = reinterpret_cast<getter> (&type_::Get##member_), .set = nullptr, .doc = doc_,         \
		.closure = nullptr                                                                                             \
	}

#define GETONLY_DECLARE(type_, member_) static PyObject *Get##member_ (type_ *self_, void *) noexcept;

namespace RocketSim::Python
{
void InitInternal (char const *path_);

bool DictSetValue (PyObject *dict_, char const *key_, PyObject *value_) noexcept;

PyObject *PyDeepCopy (void *obj_, PyObject *memo_) noexcept;

class GIL
{
public:
	~GIL () noexcept
	{
		PyGILState_Release (m_state);
	}

	GIL () noexcept : m_state (PyGILState_Ensure ())
	{
	}

private:
	PyGILState_STATE m_state;
};

struct GameMode
{
	PyObject_HEAD;

	static PyTypeObject *Type;
	static PyType_Slot Slots[];
	static PyType_Spec Spec;
};

struct Team
{
	PyObject_HEAD;

	static PyTypeObject *Type;
	static PyType_Slot Slots[];
	static PyType_Spec Spec;
};

struct DemoMode
{
	PyObject_HEAD;

	static PyTypeObject *Type;
	static PyType_Slot Slots[];
	static PyType_Spec Spec;
};

struct MemoryWeightMode
{
	PyObject_HEAD;

	static PyTypeObject *Type;
	static PyType_Slot Slots[];
	static PyType_Spec Spec;
};

struct Vec
{
	PyObject_HEAD;

	::Vec vec;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<Vec> NewFromVec (::Vec const &vec_ = {}) noexcept;
	static bool InitFromVec (Vec *self_, ::Vec const &vec_ = {}) noexcept;
	static ::Vec ToVec (Vec *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Vec *self_) noexcept;
	static PyObject *RichCompare (Vec *self_, PyObject *other_, int op_) noexcept;
	static PyObject *Repr (Vec *self_) noexcept;
	static PyObject *Format (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *Pickle (Vec *self_) noexcept;
	static PyObject *Unpickle (Vec *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (Vec *self_) noexcept;
	static PyObject *DeepCopy (Vec *self_, PyObject *memo_) noexcept;

	static PyObject *AsTuple (Vec *self_) noexcept;
	static PyObject *AsNumpy (Vec *self_) noexcept;
	static PyObject *Round (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept;
};

struct RotMat
{
	PyObject_HEAD;

	Vec *forward;
	Vec *right;
	Vec *up;

	static PyTypeObject *Type;
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<RotMat> NewFromRotMat (::RotMat const &mat_ = {}) noexcept;
	static bool InitFromRotMat (RotMat *self_, ::RotMat const &mat_ = {}) noexcept;
	static ::RotMat ToRotMat (RotMat *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (RotMat *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (RotMat *self_) noexcept;
	static PyObject *Repr (RotMat *self_) noexcept;
	static PyObject *Format (RotMat *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *Pickle (RotMat *self_) noexcept;
	static PyObject *Unpickle (RotMat *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (RotMat *self_) noexcept;
	static PyObject *DeepCopy (RotMat *self_, PyObject *memo_) noexcept;

	static PyObject *AsTuple (RotMat *self_) noexcept;
	static PyObject *AsAngle (RotMat *self_) noexcept;
	static PyObject *AsNumpy (RotMat *self_) noexcept;

	GETSET_DECLARE (RotMat, forward)
	GETSET_DECLARE (RotMat, right)
	GETSET_DECLARE (RotMat, up)
};

struct Angle
{
	PyObject_HEAD;

	::Angle angle;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<Angle> NewFromAngle (::Angle const &angle_ = {}) noexcept;
	static bool InitFromAngle (Angle *self_, ::Angle const &angle_ = {}) noexcept;
	static ::Angle ToAngle (Angle *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Angle *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Angle *self_) noexcept;
	static PyObject *Repr (Angle *self_) noexcept;
	static PyObject *Format (Angle *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *Pickle (Angle *self_) noexcept;
	static PyObject *Unpickle (Angle *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (Angle *self_) noexcept;
	static PyObject *DeepCopy (Angle *self_, PyObject *memo_) noexcept;

	static PyObject *AsTuple (Angle *self_) noexcept;
	static PyObject *AsRotMat (Angle *self_) noexcept;
	static PyObject *AsNumpy (Angle *self_) noexcept;
};

struct BallHitInfo
{
	PyObject_HEAD;

	::BallHitInfo info;

	Vec *relativePosOnBall;
	Vec *ballPos;
	Vec *extraHitVel;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<BallHitInfo> NewFromBallHitInfo (::BallHitInfo const &info_ = {}) noexcept;
	static bool InitFromBallHitInfo (BallHitInfo *self_, ::BallHitInfo const &info_ = {}) noexcept;
	static ::BallHitInfo ToBallHitInfo (BallHitInfo *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (BallHitInfo *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (BallHitInfo *self_) noexcept;
	static PyObject *Pickle (BallHitInfo *self_) noexcept;
	static PyObject *Unpickle (BallHitInfo *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (BallHitInfo *self_) noexcept;
	static PyObject *DeepCopy (BallHitInfo *self_, PyObject *memo_) noexcept;

	GETSET_DECLARE (BallHitInfo, relative_pos_on_ball)
	GETSET_DECLARE (BallHitInfo, ball_pos)
	GETSET_DECLARE (BallHitInfo, extra_hit_vel)
};

struct BallState
{
	PyObject_HEAD;

	::BallState state;

	Vec *pos;
	RotMat *rotMat;
	Vec *vel;
	Vec *angVel;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<BallState> NewFromBallState (::BallState const &state_ = {}) noexcept;
	static bool InitFromBallState (BallState *self_, ::BallState const &state_ = {}) noexcept;
	static ::BallState ToBallState (BallState *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (BallState *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (BallState *self_) noexcept;
	static PyObject *Pickle (BallState *self_) noexcept;
	static PyObject *Unpickle (BallState *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (BallState *self_) noexcept;
	static PyObject *DeepCopy (BallState *self_, PyObject *memo_) noexcept;

	GETSET_DECLARE (BallState, pos)
	GETSET_DECLARE (BallState, rot_mat)
	GETSET_DECLARE (BallState, vel)
	GETSET_DECLARE (BallState, ang_vel)
};

struct Ball
{
	PyObject_HEAD;

	std::shared_ptr<::Arena> arena;
	::Ball *ball;

	static PyTypeObject *Type;
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static Ball *New () noexcept; // internal-use only
	static PyObject *NewStub (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Ball *self_) noexcept;

	static PyObject *GetRot (Ball *self_) noexcept;
	static PyObject *GetRadius (Ball *self_) noexcept;
	static PyObject *GetState (Ball *self_) noexcept;
	static PyObject *SetState (Ball *self_, PyObject *args_, PyObject *kwds_) noexcept;
};

struct BoostPadState
{
	PyObject_HEAD;

	::BoostPadState state;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<BoostPadState> NewFromBoostPadState (::BoostPadState const &state_ = {}) noexcept;
	static bool InitFromBoostPadState (BoostPadState *self_, ::BoostPadState const &state_ = {}) noexcept;
	static ::BoostPadState ToBoostPadState (BoostPadState *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (BoostPadState *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (BoostPadState *self_) noexcept;
	static PyObject *Pickle (BoostPadState *self_) noexcept;
	static PyObject *Unpickle (BoostPadState *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (BoostPadState *self_) noexcept;
	static PyObject *DeepCopy (BoostPadState *self_, PyObject *memo_) noexcept;
};

struct BoostPad
{
	PyObject_HEAD;

	std::shared_ptr<::Arena> arena;
	::BoostPad *pad;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static BoostPad *New () noexcept; // internal-use only
	static PyObject *NewStub (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (BoostPad *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (BoostPad *self_) noexcept;

	GETONLY_DECLARE (BoostPad, is_big)

	static PyObject *GetPos (BoostPad *self_) noexcept;
	static PyObject *GetState (BoostPad *self_) noexcept;
	static PyObject *SetState (BoostPad *self_, PyObject *args_, PyObject *kwds_) noexcept;
};

struct WheelPairConfig
{
	PyObject_HEAD;

	::WheelPairConfig config;
	Vec *connectionPointOffset;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<WheelPairConfig> NewFromWheelPairConfig (::WheelPairConfig const &config_ = {}) noexcept;
	static bool InitFromWheelPairConfig (WheelPairConfig *self_, ::WheelPairConfig const &config_ = {}) noexcept;
	static ::WheelPairConfig ToWheelPairConfig (WheelPairConfig *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (WheelPairConfig *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (WheelPairConfig *self_) noexcept;
	static PyObject *Pickle (WheelPairConfig *self_) noexcept;
	static PyObject *Unpickle (WheelPairConfig *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (WheelPairConfig *self_) noexcept;
	static PyObject *DeepCopy (WheelPairConfig *self_, PyObject *memo_) noexcept;

	GETSET_DECLARE (WheelPairConfig, connection_point_offset)
};

struct CarConfig
{
	enum class Index
	{
		OCTANE,
		DOMINUS,
		PLANK,
		BREAKOUT,
		HYBRID,
		MERC,
	};

	PyObject_HEAD;

	::CarConfig config;

	Vec *hitboxSize;
	Vec *hitboxPosOffset;
	WheelPairConfig *frontWheels;
	WheelPairConfig *backWheels;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static bool FromIndex (Index index_, ::CarConfig &config_) noexcept;
	static PyRef<CarConfig> NewFromCarConfig (::CarConfig const &config_ = {}) noexcept;
	static bool InitFromCarConfig (CarConfig *self_, ::CarConfig const &config_ = {}) noexcept;
	static ::CarConfig ToCarConfig (CarConfig *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (CarConfig *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (CarConfig *self_) noexcept;
	static PyObject *Pickle (CarConfig *self_) noexcept;
	static PyObject *Unpickle (CarConfig *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (CarConfig *self_) noexcept;
	static PyObject *DeepCopy (CarConfig *self_, PyObject *memo_) noexcept;

	GETSET_DECLARE (CarConfig, hitbox_size)
	GETSET_DECLARE (CarConfig, hitbox_pos_offset)
	GETSET_DECLARE (CarConfig, front_wheels)
	GETSET_DECLARE (CarConfig, back_wheels)
};

struct CarControls
{
	PyObject_HEAD;

	::CarControls controls;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<CarControls> NewFromCarControls (::CarControls const &controls_ = {}) noexcept;
	static bool InitFromCarControls (CarControls *self_, ::CarControls const &controls_ = {}) noexcept;
	static ::CarControls ToCarControls (CarControls *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (CarControls *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (CarControls *self_) noexcept;
	static PyObject *Pickle (CarControls *self_) noexcept;
	static PyObject *Unpickle (CarControls *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (CarControls *self_) noexcept;
	static PyObject *DeepCopy (CarControls *self_, PyObject *memo_) noexcept;

	static PyObject *ClampFix (CarControls *self_) noexcept;
};

struct CarState
{
	PyObject_HEAD;

	::CarState state;

	Vec *pos;
	RotMat *rotMat;
	Vec *vel;
	Vec *angVel;
	Vec *lastRelDodgeTorque;
	CarControls *lastControls;
	Vec *worldContactNormal;
	BallHitInfo *ballHitInfo;

	static PyRef<CarState> NewFromCarState (::CarState const &state_ = {}) noexcept;
	static bool InitFromCarState (CarState *self_, ::CarState const &state_ = {}) noexcept;
	static ::CarState ToCarState (CarState *self_) noexcept;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (CarState *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (CarState *self_) noexcept;
	static PyObject *Pickle (CarState *self_) noexcept;
	static PyObject *Unpickle (CarState *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (CarState *self_) noexcept;
	static PyObject *DeepCopy (CarState *self_, PyObject *memo_) noexcept;

	GETSET_DECLARE (CarState, pos)
	GETSET_DECLARE (CarState, rot_mat)
	GETSET_DECLARE (CarState, vel)
	GETSET_DECLARE (CarState, ang_vel)
	GETSET_DECLARE (CarState, last_rel_dodge_torque)
	GETSET_DECLARE (CarState, last_controls)
	GETSET_DECLARE (CarState, world_contact_normal)
	GETSET_DECLARE (CarState, ball_hit_info)
};

struct Car
{
	PyObject_HEAD;

	::CarState demoState;

	std::shared_ptr<Arena> arena;
	::Car *car;
	unsigned goals;
	unsigned demos;
	unsigned boostPickups;
	unsigned shots;
	unsigned saves;
	unsigned assists;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static Car *New () noexcept; // internal-use only
	static PyObject *NewStub (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Car *self_) noexcept;
	static PyObject *InternalPickle (Car *self_) noexcept;
	static PyObject *InternalUnpickle (std::shared_ptr<::Arena> arena_, Car *self_, PyObject *dict_) noexcept;

	GETONLY_DECLARE (Car, id)
	GETONLY_DECLARE (Car, team)

	static PyObject *Demolish (Car *self_) noexcept;
	static PyObject *GetConfig (Car *self_) noexcept;
	static PyObject *GetControls (Car *self_) noexcept;
	static PyObject *GetForwardDir (Car *self_) noexcept;
	static PyObject *GetRightDir (Car *self_) noexcept;
	static PyObject *GetState (Car *self_) noexcept;
	static PyObject *GetUpDir (Car *self_) noexcept;
	static PyObject *Respawn (Car *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetControls (Car *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetState (Car *self_, PyObject *args_, PyObject *kwds_) noexcept;
};

struct MutatorConfig
{
	PyObject_HEAD;

	::MutatorConfig config;
	Vec *gravity;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<MutatorConfig> NewFromMutatorConfig (::MutatorConfig const &config_ = {::GameMode::SOCCAR}) noexcept;
	static bool InitFromMutatorConfig (MutatorConfig *self_,
	    ::MutatorConfig const &config_ = {::GameMode::SOCCAR}) noexcept;
	static ::MutatorConfig ToMutatorConfig (MutatorConfig *self_) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (MutatorConfig *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (MutatorConfig *self_) noexcept;
	static PyObject *Pickle (MutatorConfig *self_) noexcept;
	static PyObject *Unpickle (MutatorConfig *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (MutatorConfig *self_) noexcept;
	static PyObject *DeepCopy (MutatorConfig *self_, PyObject *memo_) noexcept;

	GETSET_DECLARE (MutatorConfig, gravity)
};

struct Arena
{
	class ThreadPool;

	PyObject_HEAD;

	std::shared_ptr<::Arena> arena;
	std::shared_ptr<ThreadPool> threadPool;
	std::map<std::uint32_t, PyRef<Car>> *cars;
	std::unordered_map<::BoostPad *, PyRef<BoostPad>> *boostPads;
	std::vector<PyRef<BoostPad>> *boostPadsByIndex;
	::BallPredTracker *ballPrediction;
	::GameEventTracker *gameEvent;

	Ball *ball;
	PyObject *ballTouchCallback;
	PyObject *ballTouchCallbackUserData;
	PyObject *boostPickupCallback;
	PyObject *boostPickupCallbackUserData;
	PyObject *carBumpCallback;
	PyObject *carBumpCallbackUserData;
	PyObject *carDemoCallback;
	PyObject *carDemoCallbackUserData;
	PyObject *goalScoreCallback;
	PyObject *goalScoreCallbackUserData;
	PyObject *shotEventCallback;
	PyObject *shotEventCallbackUserData;
	PyObject *goalEventCallback;
	PyObject *goalEventCallbackUserData;
	PyObject *saveEventCallback;
	PyObject *saveEventCallbackUserData;

	unsigned blueScore;
	unsigned orangeScore;

	std::uint64_t lastGoalTick;
	std::uint64_t lastGymStateTick;

	mutable PyObject *stepExceptionType;
	mutable PyObject *stepExceptionValue;
	mutable PyObject *stepExceptionTraceback;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Arena *self_) noexcept;
	static PyObject *Pickle (Arena *self_) noexcept;
	static PyObject *Unpickle (Arena *self_, PyObject *dict_) noexcept;
	static PyObject *Copy (Arena *self_) noexcept;
	static PyObject *DeepCopy (Arena *self_, PyObject *memo_) noexcept;

	static PyObject *AddCar (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *Clone (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *CloneInto (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *GetBallPrediction (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *GetBoostPads (Arena *self_) noexcept;
	static PyObject *GetCarFromId (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *GetCars (Arena *self_) noexcept;
	static PyObject *GetGymState (Arena *self_) noexcept;
	static PyObject *GetMutatorConfig (Arena *self_) noexcept;
	static PyObject *IsBallProbablyGoingIn (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *RemoveCar (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *ResetKickoff (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetBallTouchCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetBoostPickupCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetCarBallCollision (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetCarBumpCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetCarCarCollision (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetCarDemoCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetGoalScoreCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetShotEventCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetGoalEventCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetSaveEventCallback (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *SetMutatorConfig (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *Step (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static PyObject *Stop (Arena *self_) noexcept;

	static PyObject *MultiStep (PyObject *dummy_, PyObject *args_, PyObject *kwds_) noexcept;

	static void HandleBallTouchCallback (::Arena *arena_, ::Car *car_, void *userData_) noexcept;
	static void
	    HandleBoostPickupCallback (::Arena *arena_, ::Car *car_, ::BoostPad *boostPad_, void *userData_) noexcept;
	static void
	    HandleCarBumpCallback (::Arena *arena_, ::Car *bumper_, ::Car *victim_, bool isDemo_, void *userData_) noexcept;
	static void HandleGoalScoreCallback (::Arena *arena_, ::Team scoringTeam_, void *userData_) noexcept;

	static void HandleShotEventCallback (::Arena *arena_, ::Car *shooter_, ::Car *passer_, void *userData_) noexcept;
	static void HandleGoalEventCallback (::Arena *arena_, ::Car *shooter_, ::Car *passer_, void *userData_) noexcept;
	static void HandleSaveEventCallback (::Arena *arena_, ::Car *saver_, void *userData_) noexcept;

	GETONLY_DECLARE (Arena, game_mode);
	GETONLY_DECLARE (Arena, tick_count);
	GETONLY_DECLARE (Arena, tick_rate);
	GETONLY_DECLARE (Arena, tick_time);
};
}
