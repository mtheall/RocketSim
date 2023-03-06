#include <Python.h>
#include <structmember.h>

#include "PyRef.h"

#include "../src/Math/Math.h"
#include "../src/Sim/Arena/Arena.h"
#include "../src/Sim/Car/Car.h"

#include <map>
#include <memory>
#include <vector>

static_assert (sizeof (GameMode) == sizeof (int));
static_assert (sizeof (bool) == sizeof (char));
static_assert (sizeof (unsigned) >= sizeof (std::uint32_t));

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
struct Angle
{
	PyObject_HEAD

	::Angle angle;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Angle *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Angle *self_) noexcept;
	static PyObject *Repr (Angle *self_) noexcept;
	static PyObject *Format (Angle *self_, PyObject *args_) noexcept;

	static PyObject *AsTuple (Angle *self_) noexcept;
};

struct Vec
{
	PyObject_HEAD

	::Vec vec;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (Vec *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (Vec *self_) noexcept;
	static PyObject *Repr (Vec *self_) noexcept;
	static PyObject *Format (Vec *self_, PyObject *args_) noexcept;

	static PyObject *AsTuple (Vec *self_) noexcept;
	static PyObject *Round (Vec *self_, PyObject *args_) noexcept;
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
	Angle *angles;
	Vec *vel;
	Vec *angVel;
	Vec *lastRelDodgeTorque;
	CarControls *lastControls;
	Vec *worldContactNormal;

	static PyTypeObject *Type;
	static PyMemberDef Members[];
	static PyMethodDef Methods[];
	static PyGetSetDef GetSet[];
	static PyType_Slot Slots[];
	static PyType_Spec Spec;

	static PyObject *New (PyTypeObject *subtype_, PyObject *args_, PyObject *kwds_) noexcept;
	static int Init (CarState *self_, PyObject *args_, PyObject *kwds_) noexcept;
	static void Dealloc (CarState *self_) noexcept;

	GETSET_DECLARE (CarState, pos)
	GETSET_DECLARE (CarState, angles)
	GETSET_DECLARE (CarState, vel)
	GETSET_DECLARE (CarState, ang_vel)
	GETSET_DECLARE (CarState, last_rel_dodge_torque)
	GETSET_DECLARE (CarState, last_controls)
	GETSET_DECLARE (CarState, world_contact_normal)
};

struct Car
{
	PyObject_HEAD

	std::shared_ptr<::Arena> arena;
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

	static PyObject *GetConfig (Car *self_) noexcept;
	static PyObject *GetControls (Car *self_) noexcept;
	static PyObject *GetState (Car *self_) noexcept;
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
	static PyObject *Step (Arena *self_, PyObject *args_) noexcept;

	GETONLY_DECLARE (Arena, game_mode);
	GETONLY_DECLARE (Arena, tick_count);
	GETONLY_DECLARE (Arena, tick_rate);
	GETONLY_DECLARE (Arena, tick_time);
};
}
