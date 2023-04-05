#!/usr/bin/env python3

import RocketSim
import numpy as np
import glm
import math
import random

np.set_printoptions(formatter={"float": lambda x: f"{x: .6f}"}, linewidth=100)

def compare_float(a: float, b: float, threshold: float = 1e-4) -> bool:
	error = abs(b - a)
	if error >= threshold:
		print(a)
		print(b)
		print(error)
		return False
	return True

angle = RocketSim.Angle()
assert(angle.yaw == 0)
assert(angle.pitch == 0)
assert(angle.roll == 0)
assert(repr(angle) == "(0.0, 0.0, 0.0)")
assert(str(angle) == "(0.0, 0.0, 0.0)")
assert(f"{angle}" == "(0.0, 0.0, 0.0)")
assert(f"{angle:.3f}" == "(0.000, 0.000, 0.000)")
assert(angle.as_tuple() == (0, 0, 0))
assert(np.array_equal(angle.as_numpy(), np.array([0, 0, 0])))

angle = RocketSim.Angle(1, 2, 3)
assert(angle.yaw == 1)
assert(angle.pitch == 2)
assert(angle.roll == 3)
assert(repr(angle) == "(1.0, 2.0, 3.0)")
assert(str(angle) == "(1.0, 2.0, 3.0)")
assert(f"{angle}" == "(1.0, 2.0, 3.0)")
assert(f"{angle:.3f}" == "(1.000, 2.000, 3.000)")
assert(angle.as_tuple() == (1.0, 2.0, 3.0))
assert(np.array_equal(angle.as_numpy(), np.array([1, 2, 3])))

def compare_angle(angle, yaw, pitch, roll):
	assert(angle.yaw == yaw)
	assert(angle.pitch == pitch)
	assert(angle.roll == roll)
	assert(repr(angle) == repr((yaw, pitch, roll)))
	assert(str(angle) == str((yaw, pitch, roll)))
	assert(f"{angle}" == f"{(yaw, pitch, roll)}")
	assert(f"{angle:.3f}" == f"({yaw:.3f}, {pitch:.3f}, {roll:.3f})")
	assert(angle.as_tuple() == (yaw, pitch, roll))
	assert(np.array_equal(angle.as_numpy(), np.array([yaw, pitch, roll])))

compare_angle(RocketSim.Angle(), 0.0, 0.0, 0.0)
compare_angle(RocketSim.Angle(1, 2, 3), 1.0, 2.0, 3.0)
compare_angle(RocketSim.Angle(roll=1, yaw=2, pitch=3), 2.0, 3.0, 1.0)
compare_angle(RocketSim.Angle(roll=2), 0.0, 0.0, 2.0)
compare_angle(RocketSim.Angle(1, roll=2), 1.0, 0.0, 2.0)

def compare_vec(vec, x, y, z):
	assert(vec.x == x)
	assert(vec.y == y)
	assert(vec.z == z)
	assert(vec == RocketSim.Vec(x, y, z))
	assert(repr(vec) == repr((x, y, z)))
	assert(str(vec) == str((x, y, z)))
	assert(f"{vec}" == f"{(x, y, z)}")
	assert(f"{vec:.3f}" == f"({x:.3f}, {y:.3f}, {z:.3f})")
	assert(vec.as_tuple() == (x, y, z))
	assert(np.array_equal(vec.as_numpy(), np.array([x, y, z])))

compare_vec(RocketSim.Vec(), 0.0, 0.0, 0.0)
compare_vec(RocketSim.Vec(1, 2, 3), 1.0, 2.0, 3.0)
compare_vec(RocketSim.Vec(z=1, x=2, y=3), 2.0, 3.0, 1.0)
compare_vec(RocketSim.Vec(z=2), 0.0, 0.0, 2.0)
compare_vec(RocketSim.Vec(1, z=2), 1.0, 0.0, 2.0)

def compare_rot_mat(mat, forward, right, up):
	assert(mat.forward == forward)
	assert(mat.right == right)
	assert(mat.up == up)

compare_rot_mat(RocketSim.RotMat(), RocketSim.Vec(), RocketSim.Vec(), RocketSim.Vec())
compare_rot_mat(RocketSim.RotMat(0, 1, 2, 3, 4, 5, 6, 7, 8), RocketSim.Vec(0, 1, 2), RocketSim.Vec(3, 4, 5), RocketSim.Vec(6, 7, 8))
compare_rot_mat(RocketSim.RotMat(RocketSim.Vec(0, 1, 2), RocketSim.Vec(3, 4, 5), RocketSim.Vec(6, 7, 8)),
	RocketSim.Vec(0, 1, 2), RocketSim.Vec(3, 4, 5), RocketSim.Vec(6, 7, 8))
compare_rot_mat(RocketSim.RotMat(forward=RocketSim.Vec(0, 1, 2), right=RocketSim.Vec(3, 4, 5), up=RocketSim.Vec(6, 7, 8)),
	RocketSim.Vec(0, 1, 2), RocketSim.Vec(3, 4, 5), RocketSim.Vec(6, 7, 8))
compare_rot_mat(RocketSim.RotMat(up=RocketSim.Vec(0, 1, 2), forward=RocketSim.Vec(3, 4, 5), right=RocketSim.Vec(6, 7, 8)),
	RocketSim.Vec(3, 4, 5), RocketSim.Vec(6, 7, 8), RocketSim.Vec(0, 1, 2))

def compare_controls(controls, attrs):
	for attr in dir(controls):
		if attr.startswith("__"):
			continue

		val = getattr(controls, attr)
		if callable(val):
			continue

		if attr in attrs:
			if type(val) is float:
				compare_float(val, attrs[attr])
			else:
				assert(val == attrs[attr])
		else:
			assert(not val)

compare_controls(RocketSim.CarControls(), {})
compare_controls(RocketSim.CarControls(throttle=1.0), {"throttle": 1.0})
compare_controls(RocketSim.CarControls(steer=2.0), {"steer": 2.0})
compare_controls(RocketSim.CarControls(pitch=3.0), {"pitch": 3.0})
compare_controls(RocketSim.CarControls(yaw=4.0), {"yaw": 4.0})
compare_controls(RocketSim.CarControls(roll=5.0), {"roll": 5.0})
compare_controls(RocketSim.CarControls(boost=True), {"boost": True})
compare_controls(RocketSim.CarControls(jump=True), {"jump": True})
compare_controls(RocketSim.CarControls(handbrake=True), {"handbrake": True})
compare_controls(RocketSim.CarControls(jump=True, steer=2.0), {"steer": 2.0, "jump": True})
compare_controls(RocketSim.CarControls(0.1, 0.2, 0.3, boost=True),
	{
		"throttle": 0.1,
		"steer": 0.2,
		"pitch": 0.3,
		"boost": True,
	})

def compare_ball_state(state, pos, vel, ang_vel):
	assert(state.pos == pos)
	assert(state.vel == vel)
	assert(state.ang_vel == ang_vel)

compare_ball_state(RocketSim.BallState(), RocketSim.Vec(0, 0, 93.15), RocketSim.Vec(), RocketSim.Vec())
compare_ball_state(RocketSim.BallState(RocketSim.Vec()), RocketSim.Vec(), RocketSim.Vec(), RocketSim.Vec())
compare_ball_state(RocketSim.BallState(vel=RocketSim.Vec(1, 2, 3)), RocketSim.Vec(0, 0, 93.15), RocketSim.Vec(1, 2, 3), RocketSim.Vec())

try:
	ball = RocketSim.Ball()
except TypeError as e:
	assert(str(e) == "cannot create 'RocketSim.Ball' instances")

try:
	pad = RocketSim.BoostPad()
except TypeError as e:
	assert(str(e) == "cannot create 'RocketSim.BoostPad' instances")

try:
	car = RocketSim.Car()
except TypeError as e:
	assert(str(e) == "cannot create 'RocketSim.Car' instances")

assert(RocketSim.GameMode.SOCCAR == 0)
assert(RocketSim.GameMode.THE_VOID == 1)

assert(RocketSim.Team.BLUE == 0)
assert(RocketSim.Team.ORANGE == 1)

assert(RocketSim.DemoMode.NORMAL == 0)
assert(RocketSim.DemoMode.ON_CONTACT == 1)
assert(RocketSim.DemoMode.DISABLED == 2)

assert(RocketSim.CarConfig.OCTANE == 0)
assert(RocketSim.CarConfig.DOMINUS == 1)
assert(RocketSim.CarConfig.PLANK == 2)
assert(RocketSim.CarConfig.BREAKOUT == 3)
assert(RocketSim.CarConfig.HYBRID == 4)
assert(RocketSim.CarConfig.MERC == 5)

assert(compare_float(RocketSim.CarConfig().dodge_deadzone, 0.5))
assert(compare_float(RocketSim.CarConfig().hitbox_size.x, 120.5070))

assert(compare_float(RocketSim.CarConfig(RocketSim.CarConfig.OCTANE).hitbox_size.x, 120.5070))
assert(compare_float(RocketSim.CarConfig(RocketSim.CarConfig.DOMINUS).hitbox_size.x, 130.4270))
assert(compare_float(RocketSim.CarConfig(RocketSim.CarConfig.PLANK).hitbox_size.x, 131.3200))
assert(compare_float(RocketSim.CarConfig(RocketSim.CarConfig.BREAKOUT).hitbox_size.x, 133.9920))
assert(compare_float(RocketSim.CarConfig(RocketSim.CarConfig.HYBRID).hitbox_size.x, 129.5190))
assert(compare_float(RocketSim.CarConfig(RocketSim.CarConfig.MERC).hitbox_size.x, 123.22))

arena = RocketSim.Arena(RocketSim.GameMode.SOCCAR)
arena.get_mutator_config()

arena = RocketSim.Arena(RocketSim.GameMode.SOCCAR)
car1  = arena.add_car(RocketSim.Team.BLUE, RocketSim.CarConfig.DOMINUS)
car2  = arena.add_car(RocketSim.Team.ORANGE, RocketSim.CarConfig(RocketSim.CarConfig.DOMINUS))

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

arena = RocketSim.Arena(RocketSim.GameMode.SOCCAR)
car1  = arena.add_car(RocketSim.Team.BLUE)
car2  = arena.add_car(RocketSim.Team.ORANGE)
ball  = arena.ball

arena.reset_kickoff()

controls = RocketSim.CarControls()
controls.throttle = 1.0
controls.steer    = 0.3

car1.set_controls(controls)
car2.set_controls(controls)

assert(car1.id != car2.id)
assert(car1.team == RocketSim.Team.BLUE)
assert(car2.team == RocketSim.Team.ORANGE)

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
		assert(compare_array(np.array(m), pyr_to_mat3(pyr), 1e-5))
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
		assert(compare_array(np.array(m), pyr_to_mat3(pyr), 1e-5))
		assert(check_pyr(pyr))

		assert(compare_array(invert_vector(np.array(car_dir_x)), np.array(m * x)))
		assert(compare_array(invert_vector(np.array(car_dir_y)), np.array(m * y)))
		assert(compare_array(invert_vector(np.array(car_dir_z)), np.array(m * z)))
		assert(compare_array(invert_vector(np.array(car_dir_x)), np.array(q * x)))
		assert(compare_array(invert_vector(np.array(car_dir_y)), np.array(q * y)))
		assert(compare_array(invert_vector(np.array(car_dir_z)), np.array(q * z)))
