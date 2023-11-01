#!/usr/bin/env python3

import RocketSim as rs

import glm
import math
import numpy as np
import unittest

# simple actor that drives straight toward a target
def target_chase(target_pos: rs.Vec, car: rs.Car):
  target_pos = glm.vec3(*target_pos.as_tuple())
  car_pos    = glm.vec3(*car.get_state().pos.as_tuple())
  car_fwd    = glm.vec3(*car.get_state().rot_mat.forward.as_tuple())
  target_dir = glm.normalize(target_pos - car_pos)

  steer = math.acos(np.clip(glm.dot(car_fwd, target_dir), -1.0, 1.0))
  if glm.cross(car_fwd, target_dir).z < 0.0:
    steer = -steer

  steer = np.clip(steer, -1.0, 1.0)

  car.set_controls(rs.CarControls(throttle=1.0, steer=steer, boost=True, handbrake=abs(steer)==1.0))

class TestRegression(unittest.TestCase):
  def test_multiple_demos_one_tick(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    orange = arena.add_car(rs.Team.ORANGE, rs.CarConfig(rs.CarConfig.BREAKOUT))
    blue   = arena.add_car(rs.Team.BLUE, rs.CarConfig(rs.CarConfig.HYBRID))

    orange_state     = rs.CarState()
    orange_state.pos = rs.Vec(0, 0, 17)
    orange.set_state(orange_state)

    blue_state       = rs.CarState()
    blue_state.pos   = rs.Vec(-300, 0, 17)
    blue_state.vel   = rs.Vec(2300, 0, 0)
    blue_state.boost = 100
    blue.set_state(blue_state)

    blue.set_controls(rs.CarControls(throttle=1, boost=True))

    demos = set()
    def handle_demo(arena, bumper, victim, data):
      key = (arena.tick_count, bumper.id, victim.id)
      self.assertNotIn(key, demos)
      demos.add(key)

    arena.set_car_demo_callback(handle_demo)

    arena.step(15)
    self.assertEqual(len(demos), 1)
    self.assertEqual({(9, 2, 1)}, demos)

  def test_continuous_boost_pickup(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    pickups = [0]
    def handle_boost(arena, car, boost_pad, data):
      pickups[0] += 1

    arena.set_boost_pickup_callback(handle_boost)

    blue = arena.add_car(rs.Team.BLUE)

    pad = arena.get_boost_pads()[0]

    blue_state     = rs.CarState()
    blue_state.pos = pad.get_pos()

    for i in range(1250):
      blue_state.boost = 0
      blue.set_state(blue_state)
      arena.step()

    if pad.is_big:
      self.assertEqual(pickups[0], 2)
    else:
      self.assertEqual(pickups[0], 3)

  def test_game_state_nan(self):
    SIDE_WALL_X     = 4096
    BACK_WALL_Y     = 5120
    CEILING_Z       = 2044
    BALL_RADIUS     = 92.75
    BALL_MAX_SPEED  = 6000
    CAR_MAX_SPEED   = 2300
    CAR_MAX_ANG_VEL = 5.5

    LIM_X = SIDE_WALL_X - 1152 / 2 - BALL_RADIUS * 2 ** 0.5
    LIM_Y = BACK_WALL_Y - 1152 / 2 - BALL_RADIUS * 2 ** 0.5
    LIM_Z = CEILING_Z - BALL_RADIUS

    PITCH_LIM = np.pi / 2
    YAW_LIM = np.pi
    ROLL_LIM = np.pi

    GOAL_X_MAX = 800.0
    GOAL_X_MIN = -800.0

    PLACEMENT_BOX_X = 5000
    PLACEMENT_BOX_Y = 2000
    PLACEMENT_BOX_Y_OFFSET = 3000

    GOAL_LINE = 5100

    YAW_MAX = np.pi

    def rand_uvec3(rng: np.random.Generator = np.random):
      vec = rng.random(3) - 0.5
      return vec / np.linalg.norm(vec)

    def rand_vec3(max_norm, rng: np.random.Generator = np.random):
      return rand_uvec3(rng) * (rng.random() * max_norm)

    arena = rs.Arena(rs.GameMode.SOCCAR)

    arena.add_car(rs.Team.BLUE)
    arena.add_car(rs.Team.ORANGE)

    ball_speed = np.random.exponential(-BALL_MAX_SPEED / np.log(1 - 0.999))
    vel = rand_vec3(min(ball_speed, BALL_MAX_SPEED))

    ang_vel = rand_vec3(np.random.triangular(0, 0, CAR_MAX_ANG_VEL + 0.5))

    ball_state = arena.ball.get_state()

    ball_state.pos     = rs.Vec(1943.54, -776.12, 406.78)
    ball_state.vel     = rs.Vec(122.79422, -234.43246, 259.7227)
    ball_state.ang_vel = rs.Vec(-0.11868675, 0.10540164, -0.05797875)

    arena.ball.set_state(ball_state)

    for car in arena.get_cars():
      car_state = car.get_state()

      if car.team == 0:
        car_state.pos     = rs.Vec(1943.54, -776.12, 406.78)
        car_state.vel     = rs.Vec(-303.3699951171875, -214.94998168945312, -410.7699890136719)
        car_state.ang_vel = rs.Vec(-0.91367, -1.7385, 0.28524998)
        car_state.rot_mat = rs.Angle(pitch=np.random.triangular(-PITCH_LIM, 0, PITCH_LIM),
                  yaw=np.random.uniform(-YAW_LIM, YAW_LIM),
                  roll=np.random.triangular(-ROLL_LIM, 0, ROLL_LIM)).as_rot_mat()
        car_state.boost   = 0.593655776977539
      else:
        car_state.pos     = rs.Vec(1917.4401, -738.27997, 462.01)
        car_state.vel     = rs.Vec(142.98796579, 83.07732574, -20.63784965)
        car_state.ang_vel = rs.Vec(-0.22647299, 0.10713767, 0.24121591)
        car_state.rot_mat = rs.Angle(pitch=np.random.triangular(-PITCH_LIM, 0, PITCH_LIM),
                  yaw=np.random.uniform(-YAW_LIM, YAW_LIM),
                  roll=np.random.triangular(-ROLL_LIM, 0, ROLL_LIM)).as_rot_mat()
        car_state.boost   = 0.8792996978759766

      car.set_state(car_state)

    arena.step(8)
    gym_state = arena.get_gym_state()

  def test_hoops_boost_pads(self):
    arena = rs.Arena(rs.GameMode.HOOPS)
    arena.get_boost_pads()

  def test_gym_state_inverse(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    car_a = arena.add_car(rs.Team.BLUE)
    car_b = arena.add_car(rs.Team.ORANGE)

    # cars should spawn mirror of each other
    arena.reset_kickoff()

    for i in range(150):
      if i < 10:
        # jump for a few frames to avoid flipping
        controls = rs.CarControls(throttle=1.0, jump=True)
      else:
        controls = rs.CarControls(throttle=1.0, yaw=1.0, pitch=1.0, roll=1.0, jump=True)

      car_a.set_controls(controls)
      car_b.set_controls(controls)
      arena.step()

      gym_state = arena.get_gym_state()

      state_a = gym_state[3][0] # blue team, standard
      state_b = gym_state[4][1] # orange team, inverted

      # pos
      if not np.amax(np.absolute(state_a[11:14] - state_b[11:14])) < 1e-3:
        print(arena.tick_count, state_a[11:14], state_b[11:14])
      self.assertTrue(np.amax(np.absolute(state_a[11:14] - state_b[11:14])) < 1e-3)

      # quat
      q1 = state_a[14:18]
      q2 = state_b[14:18]
      t1 = np.amax(np.absolute(q1 + q2))
      t2 = np.amax(np.absolute(q1 - q2))
      if not min(t1, t2) < 1e-3:
        print(arena.tick_count, q1, q2)
      self.assertTrue(min(t1, t2) < 1e-3)

      # vel
      if not np.amax(np.absolute(state_a[18:21] - state_b[18:21])) < 1e-3:
        print(arena.tick_count, state_a[18:21], state_b[18:21])
      self.assertTrue(np.amax(np.absolute(state_a[18:21] - state_b[18:21])) < 1e-3)

      # ang_vel
      if not np.amax(np.absolute(state_a[21:24] - state_b[21:24])) < 1e-3:
        print(arena.tick_count, state_a[21:24], state_b[21:24])
      self.assertTrue(np.amax(np.absolute(state_a[21:24] - state_b[21:24])) < 1e-3)

      # mat3
      if not np.amax(np.absolute(state_a[24:33] - state_b[24:33])) < 1e-3:
        print(arena.tick_count, state_a[24:33], state_b[24:33])
      self.assertTrue(np.amax(np.absolute(state_a[24:33] - state_b[24:33])) < 1e-3)

      # pyr
      if not np.amax(np.absolute(state_a[33:36] - state_b[33:36])) < 1e-3:
        print(arena.tick_count, state_a[33:36], state_b[33:36])
      self.assertTrue(np.amax(np.absolute(state_a[33:36] - state_b[33:36])) < 1e-3)

  def test_hoops_car_collision(self):
    arena = rs.Arena(rs.GameMode.HOOPS)

    car_a = arena.add_car(rs.Team.BLUE)
    car_b = arena.add_car(rs.Team.ORANGE)

    bumped = [False]
    def car_bump_callback(arena: rs.Arena, bumper: rs.Car, victim: rs.Car, is_demo: bool, data):
      data[0] = True
      arena.stop()

    arena.set_car_bump_callback(car_bump_callback, bumped)

    for i in range(1000):
      target_chase(car_a.get_state().pos, car_b)
      target_chase(car_b.get_state().pos, car_a)
      arena.step()

      if bumped[0]:
        break

    self.assertTrue(bumped[0])

if __name__ == "__main__":
  unittest.main()
