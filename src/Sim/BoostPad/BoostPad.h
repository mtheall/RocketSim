#pragma once
#include "../../BaseInc.h"

#include "../Car/Car.h"

#include "../../DataStream/DataStreamIn.h"
#include "../../DataStream/DataStreamOut.h"

#include "../MutatorConfig/MutatorConfig.h"

struct BoostPadState {
	bool isActive = true;
	float cooldown = 0;

	Car* curLockedCar = nullptr;
	uint32_t prevLockedCarID = 0;

	void Serialize(DataStreamOut& out) const;
	void Deserialize(DataStreamIn& in);
};
#define BOOSTPAD_SERIALIZATION_FIELDS \
isActive, cooldown, prevLockedCarID

class BoostPad {
public:
	bool isBig;
	Vec pos;

	Vec _posBT;
	Vec _boxMinBT, _boxMaxBT;

	BoostPadState _internalState;

	RSAPI BoostPadState GetState() const { return _internalState; }
	RSAPI void SetState(const BoostPadState& state) { _internalState = state; }

	// For construction by Arena
	static BoostPad* _AllocBoostPad();
	void _Setup(bool isBig, Vec pos);

	void _CheckCollide(Car* car);

	void _PreTickUpdate(float tickTime);
	bool _PostTickUpdate(float tickTime, const MutatorConfig& mutatorConfig);
private:
	BoostPad() {}
};
