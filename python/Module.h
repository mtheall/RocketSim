#include <Python.h>
#include <structmember.h>

#include "PyRef.h"

#include "../src/Math/Math.h"
#include "../src/Sim/Arena/Arena.h"
#include "../src/Sim/Car/Car.h"

#include <map>
#include <memory>
#include <vector>

template <typename T>
struct TypeHelper
{
};

// clang-format off
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
TYPE_HELPER(long long,          T_ULONGLONG);
TYPE_HELPER(unsigned long long, T_ULONGLONG);
#undef TYPE_HELPER
// clang-format on

static_assert (sizeof (bool) == sizeof (char));

#define GETSET_ENTRY(type, member)                                                                                     \
	{                                                                                                                  \
		.name = #member, .get = reinterpret_cast<getter> (&type::Get##member),                                         \
		.set = reinterpret_cast<setter> (&type::Set##member), .doc = #member, .closure = nullptr                       \
	}

#define GETSET_DECLARE(type, member)                                                                                   \
	static PyObject *Get##member (type *self_, void *) noexcept;                                                       \
	static int Set##member (type *self_, PyObject *value_, void *) noexcept;

#define GETONLY_ENTRY(type, member)                                                                                    \
	{                                                                                                                  \
		.name = #member, .get = reinterpret_cast<getter> (&type::Get##member), .set = nullptr, .doc = #member,         \
		.closure = nullptr                                                                                             \
	}

#define GETONLY_DECLARE(type, member) static PyObject *Get##member (type *self_, void *) noexcept;

namespace RocketSim::Python
{
struct Vec
{
	PyObject_HEAD

	::Vec vec;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<Vec> NewFromVec (::Vec const &vec_ = {}) noexcept;
	static bool InitFromVec (Vec *self_, ::Vec const &vec_ = {}) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Vec *self_) noexcept;
	static PyObject *Repr (Vec *self_) noexcept;
	static PyObject *Format (Vec *self_, PyObject *args_) noexcept;

	static PyObject *AsTuple (Vec *self_) noexcept;
	static PyObject *Round (Vec *self_, PyObject *args_) noexcept;
};

struct RotMat
{
	PyObject_HEAD

	::RotMat mat;
	Vec *forward;
	Vec *right;
	Vec *up;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<RotMat> NewFromRotMat (::RotMat const &mat_ = {}) noexcept;
	static bool InitFromRotMat (RotMat *self_, ::RotMat const &mat_ = {}) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (RotMat *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (RotMat *self_) noexcept;
	static PyObject *Repr (RotMat *self_) noexcept;
	static PyObject *Format (RotMat *self_, PyObject *args_) noexcept;

	static PyObject *AsTuple (RotMat *self_) noexcept;
	static PyObject *AsAngle (RotMat *self_) noexcept;

	GETSET_DECLARE (RotMat, forward)
	GETSET_DECLARE (RotMat, right)
	GETSET_DECLARE (RotMat, up)
};

struct Angle
{
	PyObject_HEAD

	::Angle angle;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<Angle> NewFromAngle (::Angle const &angle_ = {}) noexcept;
	static bool InitFromAngle (Angle *self_, ::Angle const &angle_ = {}) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Angle *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Angle *self_) noexcept;
	static PyObject *Repr (Angle *self_) noexcept;
	static PyObject *Format (Angle *self_, PyObject *args_) noexcept;

	static PyObject *AsTuple (Angle *self_) noexcept;
};

struct BallState
{
	PyObject_HEAD

	::BallState state;

	Vec *pos;
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

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (BallState *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (BallState *self_) noexcept;

	GETSET_DECLARE (BallState, pos)
	GETSET_DECLARE (BallState, vel)
	GETSET_DECLARE (BallState, ang_vel)
};

struct Ball
{
	PyObject_HEAD

	std::shared_ptr<::Arena> arena;
	::Ball *ball;

	static PyTypeObject *Type;
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static Ball *New () noexcept; // internal-use only
	static void Dealloc (Ball *self_) noexcept;

	static PyObject *GetRadius (Ball *self_) noexcept;
	static PyObject *GetState (Ball *self_) noexcept;
	static PyObject *SetState (Ball *self_, PyObject *args_) noexcept;
};

struct BoostPadState
{
	PyObject_HEAD

	::BoostPadState state;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (BoostPadState *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (BoostPadState *self_) noexcept;
};

struct BoostPad
{
	PyObject_HEAD

	std::shared_ptr<::Arena> arena;
	::BoostPad *pad;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static BoostPad *New () noexcept; // internal-use only
	static int Init (BoostPad *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (BoostPad *self_) noexcept;

	GETONLY_DECLARE (BoostPad, is_big)

	static PyObject *GetPos (BoostPad *self_) noexcept;
	static PyObject *GetState (BoostPad *self_) noexcept;
	static PyObject *SetState (BoostPad *self_, PyObject *args_) noexcept;
};

struct WheelPairConfig
{
	PyObject_HEAD

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

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (WheelPairConfig *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (WheelPairConfig *self_) noexcept;

	GETSET_DECLARE (WheelPairConfig, connection_point_offset)
};

struct CarConfig
{
	PyObject_HEAD

	::CarConfig config;

	Vec *hitboxSize;
	Vec *hitboxPosOffset;
	WheelPairConfig *frontWheels;
	WheelPairConfig *backWheels;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<CarConfig> NewFromCarConfig (::CarConfig const &config_ = {}) noexcept;
	static bool InitFromCarConfig (CarConfig *self_, ::CarConfig const &config_ = {}) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (CarConfig *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (CarConfig *self_) noexcept;

	GETSET_DECLARE (CarConfig, hitbox_size)
	GETSET_DECLARE (CarConfig, hitbox_pos_offset)
	GETSET_DECLARE (CarConfig, front_wheels)
	GETSET_DECLARE (CarConfig, back_wheels)
};

struct CarControls
{
	PyObject_HEAD

	::CarControls controls;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyRef<CarControls> NewFromCarControls (::CarControls const &controls_ = {}) noexcept;
	static bool InitFromCarControls (CarControls *self_, ::CarControls const &controls_ = {}) noexcept;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (CarControls *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (CarControls *self_) noexcept;

	static PyObject *ClampFix (CarControls *self_, PyObject *args_) noexcept;
};

struct CarState
{
	PyObject_HEAD

	::CarState state;

	Vec *pos;
	RotMat *rotMat;
	Vec *vel;
	Vec *angVel;
	Vec *lastRelDodgeTorque;
	CarControls *lastControls;
	Vec *worldContactNormal;

	static PyRef<CarState> NewFromCarState (::CarState const &state_ = {}) noexcept;
	static bool InitFromCarState (CarState *self_, ::CarState const &state_ = {}) noexcept;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (CarState *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (CarState *self_) noexcept;

	GETSET_DECLARE (CarState, pos)
	GETSET_DECLARE (CarState, rot_mat)
	GETSET_DECLARE (CarState, vel)
	GETSET_DECLARE (CarState, ang_vel)
	GETSET_DECLARE (CarState, last_rel_dodge_torque)
	GETSET_DECLARE (CarState, last_controls)
	GETSET_DECLARE (CarState, world_contact_normal)
};

struct Car
{
	PyObject_HEAD

	std::shared_ptr<Arena> arena;
	::Car *car;

	static PyTypeObject *Type;
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static Car *New () noexcept; // internal-use only
	static void Dealloc (Car *self_) noexcept;

	GETONLY_DECLARE (Car, id)
	GETONLY_DECLARE (Car, team)

	static PyObject *Demolish (Car *self_) noexcept;
	static PyObject *GetConfig (Car *self_) noexcept;
	static PyObject *GetControls (Car *self_) noexcept;
	static PyObject *GetForwardDir (Car *self_) noexcept;
	static PyObject *GetRightDir (Car *self_) noexcept;
	static PyObject *GetState (Car *self_) noexcept;
	static PyObject *GetUpDir (Car *self_) noexcept;
	static PyObject *Respawn (Car *self_, PyObject *args_) noexcept;
	static PyObject *SetControls (Car *self_, PyObject *args_) noexcept;
	static PyObject *SetState (Car *self_, PyObject *args_) noexcept;
};

struct Arena
{
	PyObject_HEAD

	std::shared_ptr<::Arena> arena;
	std::map<std::uint32_t, PyRef<Car>> *cars;
	std::vector<PyRef<BoostPad>> *boostPads;
	Ball *ball;
	PyObject *goalScoreCallback;
	PyObject *goalScoreCallbackUserData;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Arena *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Arena *self_) noexcept;

	static PyObject *AddCar (Arena *self_, PyObject *args_) noexcept;
	static PyObject *Clone (Arena *self_) noexcept;
	static PyObject *GetBoostPads (Arena *self_) noexcept;
	static PyObject *GetCarFromId (Arena *self_, PyObject *args_) noexcept;
	static PyObject *GetCars (Arena *self_) noexcept;
	static PyObject *RegisterGoalScoreCallback (Arena *self_, PyObject *args_) noexcept;
	static PyObject *RemoveCar (Arena *self_, PyObject *args_) noexcept;
	static PyObject *ResetKickoff (Arena *self_, PyObject *args_) noexcept;
	static PyObject *SetGoalScoreCallback (Arena *self_, PyObject *args_) noexcept;
	static PyObject *Step (Arena *self_, PyObject *args_) noexcept;

	static void HandleGoalScoreCallback (::Arena *arena_, Team scoringTeam_, void *userData_) noexcept;

	GETONLY_DECLARE (Arena, game_mode);
	GETONLY_DECLARE (Arena, tick_count);
	GETONLY_DECLARE (Arena, tick_rate);
	GETONLY_DECLARE (Arena, tick_time);
};
}
