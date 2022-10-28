#pragma once

// BulletLink.h: Includes needed bullet headers, and defines some convenient typedefs/wrappers/etc.

// Bullet v3 (https://github.com/bulletphysics/bullet3)
#include "../libsrc/bullet3-3.24/btBulletCollisionCommon.h";
#include "../libsrc/bullet3-3.24/btBulletDynamicsCommon.h";
#include "../libsrc/bullet3-3.24/BulletDynamics/Dynamics/btRigidBody.h"

typedef btVector3 Vec;

//  BulletPhysics Units (1m) to Unreal Units (2cm) conversion scale
#define BT_TO_UU (50.f)

// Unreal Units (2cm) to BulletPhysics Units (1m) conversion scale
#define UU_TO_BT (1.f/50.f)

#define TICKRATE 120
#define TICKTIME (1.f/TICKRATE)

// NOTE: Values should be in radians
struct Angle {
	float yaw, pitch, roll;
};