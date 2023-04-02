#!/usr/bin/env python3

import RocketSim
import numpy as np
import glm
import math
import random

np.set_printoptions(formatter={"float": lambda x: f"{x: .6f}"}, linewidth=100)

arena = RocketSim.Arena(RocketSim.SOCCAR)
car1  = arena.add_car(RocketSim.BLUE)
car2  = arena.add_car(RocketSim.ORANGE)

for i in range(100):
	for j in range(10):
		arena.add_car(random.randint(0, 1), random.randint(0, 5))
	while len(arena.get_cars()) > 0:
		arena.remove_car(arena.get_cars()[0])

for i in range(100):
	for j in range(10):
		arena.add_car(random.randint(0, 1), random.randint(0, 5))
	for car in arena.get_cars():
		arena.remove_car(car)

x = glm.vec3(1, 0, 0)
y = glm.vec3(0, 1, 0)
z = glm.vec3(0, 0, 1)

arena = RocketSim.Arena(RocketSim.SOCCAR)
car1  = arena.add_car(RocketSim.BLUE)
car2  = arena.add_car(RocketSim.ORANGE)
ball  = arena.ball

arena.reset_kickoff()

controls = RocketSim.CarControls()
controls.throttle = 1.0
controls.steer    = 0.3

car1.set_controls(controls)
car2.set_controls(controls)

assert(car1.id != car2.id)
assert(car1.team == RocketSim.BLUE)
assert(car2.team == RocketSim.ORANGE)

inv_mtx = glm.mat3(-1,  0,  0,
                    0, -1,  0,
                    0,  0,  1)

def load_mat3(mat: np.ndarray) -> glm.mat3:
	return glm.mat3(*mat)

def invert_vector(vec: np.ndarray) -> np.ndarray:
	return np.array(inv_mtx * glm.vec3(*vec))

def invert_mat3(mat: glm.mat3) -> np.ndarray:
	return np.array(inv_mtx * mat)

def invert_quat(quat: np.ndarray) -> np.ndarray:
	return np.array(glm.angleAxis(math.pi, z) * glm.quat(*quat))

def compare_array(a: np.ndarray, b: np.ndarray, threshold: float = 1e-6) -> bool:
	error = np.amax(np.absolute(a - b))
	if error >= threshold:
		print(a)
		print(b)
		print(error)
		return False
	return True
	
def compare_quat(q1: np.ndarray, q2: np.ndarray) -> bool:
	threshold = 1e-6
	t1 = np.amax(np.absolute(q1 + q2))
	t2 = np.amax(np.absolute(q1 - q2))
	if t1 >= threshold and t2 >= threshold:
		print(q1)
		print(q2)
		print(min(t1, t2))
		return False
	return True

def pyr_to_mat3(pyr: np.ndarray) -> np.ndarray:
	# what
	pitch, yaw, roll = pyr
	q = glm.normalize(glm.angleAxis(yaw, z))
	q = q * glm.normalize(glm.angleAxis(pitch, y))
	q = q * glm.normalize(glm.angleAxis(roll, -x))
	return np.array(glm.mat3_cast(q))

def clamp(v: float, a: float, b: float) -> float:
	return max(min(v, b), a)

def check_pyr(pyr: np.ndarray) -> bool:
	pitch, yaw, roll = pyr

	if abs(pitch) > math.pi / 2.0:
		print(pitch)
		return False

	if abs(yaw) > math.pi:
		print(yaw)
		return False

	if abs(roll) > math.pi:
		print(roll)
		return False

	return True

for i in range(10000):
	arena.step(10)
	state = arena.get_gym_state()

	assert(state[0][0] == arena.game_mode)
	#assert(state[0][1] == ball.last_hit_id)
	assert(state[0][2] == arena.blue_score)
	assert(state[0][3] == arena.orange_score)

	if int(state[0][1]):
		last_hit = arena.get_car_from_id(int(state[0][1])).get_state().ball_hit_info
		assert(last_hit.is_valid)

		for car in arena.get_cars():
			car_hit = car.get_state().ball_hit_info
			if not car_hit.is_valid:
				continue
			assert(car_hit.tick_count_when_hit <= last_hit.tick_count_when_hit)

	ball_state = ball.get_state()

	gym_state = state[2][0]
	assert(compare_array(gym_state[0:3], ball_state.pos.as_numpy()))
	#assert(compare_array(gym_state[3:7], ball_state.quat.as_numpy()))
	assert(compare_array(gym_state[7:10], ball_state.vel.as_numpy()))
	assert(compare_array(gym_state[10:13], ball_state.ang_vel.as_numpy()))
	#assert(compare_array(gym_state[13:22].reshape(3, 3), ball_state.rot_mat.as_numpy()))
	#assert(compare_array(gym_state[22:25], ball_state.pyr.as_numpy()))

	gym_state = state[2][1]
	assert(compare_array(gym_state[0:3], invert_vector(ball_state.pos.as_numpy())))
	##assert(compare_array(gym_state[3:7], ball_state.quat.as_numpy()))
	assert(compare_array(gym_state[7:10], invert_vector(ball_state.vel.as_numpy())))
	assert(compare_array(gym_state[10:13], ball_state.ang_vel.as_numpy()))
	##assert(compare_array(gym_state[13:22].reshape(3, 3), ball_state.rot_mat.as_numpy()))
	##assert(compare_array(gym_state[22:25], ball_state.pyr.as_numpy()))

	for i, car in enumerate(arena.get_cars()):
		car_state = car.get_state()

		gym_state = state[i + 3]
		for j in range(2):
			assert(car.id == gym_state[j][0])
			assert(car.team == gym_state[j][1])
			assert(car.goals == gym_state[j][2])
			#assert(car.saves == gym_state[j][3])
			#assert(car.shots == gym_state[j][4])
			assert(car.demos == gym_state[j][5])
			assert(car.boost_pickups == gym_state[j][6])
			assert(car_state.is_demoed == gym_state[j][7])
			assert(car_state.is_on_ground == gym_state[j][8])
			#assert(car_state.hit_last_step == gym_state[j][9])
			assert(car_state.boost == gym_state[j][10])

		car_dir_x = glm.vec3(car_state.rot_mat.forward.as_numpy())
		car_dir_y = glm.vec3(car_state.rot_mat.right.as_numpy())
		car_dir_z = glm.vec3(car_state.rot_mat.up.as_numpy())

		gym_state = state[i + 3][0]
		assert(compare_array(gym_state[11:14], car_state.pos.as_numpy()))
		assert(compare_array(gym_state[18:21], car_state.vel.as_numpy()))
		assert(compare_array(gym_state[21:24], car_state.ang_vel.as_numpy()))
		assert(compare_array(gym_state[24:33].reshape(3, 3), car_state.rot_mat.as_numpy()))

		m   = load_mat3(gym_state[24:33])
		q   = glm.quat_cast(m)
		pyr = gym_state[33:36]
		assert(compare_quat(gym_state[14:18], np.array(q)))
		assert(compare_array(np.array(m), pyr_to_mat3(pyr), 1e-4))
		assert(check_pyr(pyr))

		assert(compare_array(np.array(car_dir_x), np.array(m * x)))
		assert(compare_array(np.array(car_dir_y), np.array(m * y)))
		assert(compare_array(np.array(car_dir_z), np.array(m * z)))
		assert(compare_array(np.array(car_dir_x), np.array(q * x)))
		assert(compare_array(np.array(car_dir_y), np.array(q * y)))
		assert(compare_array(np.array(car_dir_z), np.array(q * z)))

		# check inversion
		gym_state = state[i + 3][1]
		assert(compare_array(gym_state[11:14], invert_vector(car_state.pos.as_numpy())))
		assert(compare_quat(gym_state[14:18], invert_quat(q)))
		assert(compare_array(gym_state[18:21], invert_vector(car_state.vel.as_numpy())))
		assert(compare_array(gym_state[21:24], car_state.ang_vel.as_numpy()))
		assert(compare_array(np.array(load_mat3(gym_state[24:33])), invert_mat3(m)))

		m = load_mat3(gym_state[24:33])
		q = glm.quat_cast(m)
		pyr = gym_state[33:36]
		assert(compare_quat(gym_state[14:18], np.array(q)))
		assert(compare_array(np.array(m), pyr_to_mat3(pyr), 1e-4))
		assert(check_pyr(pyr))

		assert(compare_array(invert_vector(np.array(car_dir_x)), np.array(m * x)))
		assert(compare_array(invert_vector(np.array(car_dir_y)), np.array(m * y)))
		assert(compare_array(invert_vector(np.array(car_dir_z)), np.array(m * z)))
		assert(compare_array(invert_vector(np.array(car_dir_x)), np.array(q * x)))
		assert(compare_array(invert_vector(np.array(car_dir_y)), np.array(q * y)))
		assert(compare_array(invert_vector(np.array(car_dir_z)), np.array(q * z)))
