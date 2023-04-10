#pragma once
#include "../BaseInc.h"

#include <cstring>

// Stores all control inputs to a car
struct CarControls {
	// Driving control
	float throttle = 0.0f, steer = 0.0f;

	// Air orientation control
	float pitch = 0.0f, yaw = 0.0f, roll = 0.0f;

	// Boolean action inputs
	bool boost = false, jump = false, handbrake = false;

	// Maybe someday...
	// bool useItem;

	// Makes all values range-valid (clamps from -1 to 1)
	void ClampFix() {
		throttle	= RS_CLAMP(throttle,	-1, 1);
		steer		= RS_CLAMP(steer,		-1, 1);
		pitch		= RS_CLAMP(pitch,		-1, 1);
		yaw			= RS_CLAMP(yaw,		-1, 1);
		roll		= RS_CLAMP(roll,		-1, 1);
	}
};

#define CAR_CONTROLS_SERIALIZATION_FIELDS(name) \
name.throttle, name.steer, \
name.pitch, name.yaw, name.roll, \
name.boost, name.jump, name.handbrake
