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

	controls = RocketSim.CarControls()
	controls.throttle = 1
	controls.boost    = True
	blue.set_controls(controls)

	demos = set()
	def handle_demo(arena, car, victim, is_demo, data):
		if not is_demo:
			return

		key = (arena.tick_count, car.id, victim.id)
		assert(key not in demos)
		demos.add(key)

	arena.set_car_bump_callback(handle_demo, None)

	arena.step(15)
	assert(len(demos) == 1)
	assert((9, 2, 1) in demos)

if __name__ == "__main__":
	multiple_demos_one_tick()
