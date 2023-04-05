#!/usr/bin/env python3

import RocketSim

def multiple_demos_one_tick():
	arena = RocketSim.Arena(RocketSim.GameMode.SOCCAR)

	orange = arena.add_car(RocketSim.Team.ORANGE, RocketSim.CarConfig(RocketSim.CarConfig.BREAKOUT))
	blue   = arena.add_car(RocketSim.Team.BLUE, RocketSim.CarConfig(RocketSim.CarConfig.HYBRID))

	orange_state     = RocketSim.CarState()
	orange_state.pos = RocketSim.Vec(0, 0, 17)
	orange.set_state(orange_state)

	blue_state       = RocketSim.CarState()
	blue_state.pos   = RocketSim.Vec(-300, 0, 17)
	blue_state.vel   = RocketSim.Vec(2300, 0, 0)
	blue_state.boost = 100
	blue.set_state(blue_state)

	blue.set_controls(RocketSim.CarControls(throttle=1, boost=True))

	demos = set()
	def handle_demo(arena, car, victim, is_demo, data):
		if not is_demo:
			return

		key = (arena.tick_count, car.id, victim.id)
		assert(key not in demos)
		demos.add(key)

	arena.set_car_bump_callback(handle_demo)

	arena.step(15)
	assert(len(demos) == 1)
	assert((9, 2, 1) in demos)

def continuous_boost_pickup():
	arena = RocketSim.Arena(RocketSim.GameMode.SOCCAR)

	pickups = [0]
	def handle_boost(arena, car, pad, data):
		pickups[0] += 1

	arena.set_boost_pickup_callback(handle_boost)

	blue = arena.add_car(RocketSim.Team.BLUE)

	pad = arena.get_boost_pads()[0]

	blue_state     = RocketSim.CarState()
	blue_state.pos = pad.get_pos()

	for i in range(1250):
		blue_state.boost = 0
		blue.set_state(blue_state)
		arena.step()

	if pad.is_big:
		assert(pickups[0] == 2)
	else:
		assert(pickups[0] == 3)

if __name__ == "__main__":
	multiple_demos_one_tick()
	continuous_boost_pickup()
