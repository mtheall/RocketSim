#!/usr/bin/env python3

import RocketSim as rs

import copy
import glm
import math
import numpy as np
import pickle
import random
import unittest

np.set_printoptions(formatter={"float": lambda x: f"{x: .6f}"}, linewidth=100)

def pickled(v):
  return pickle.loads(pickle.dumps(v))

def random_bool() -> bool:
  return random.randint(0, 1) == 1

def random_int() -> bool:
  return random.randint(1, 1000)

def random_float() -> float:
  return random.uniform(-1.0, 1.0)

def random_vec() -> rs.Vec:
  return rs.Vec(random_float(), random_float(), random_float())

def random_mat() -> rs.RotMat:
  return rs.RotMat(random_vec(), random_vec(), random_vec())

def random_angle() -> rs.Vec:
  return rs.Angle(random_float(), random_float(), random_float())

def random_car(arena: rs.Arena, team = random.randint(0, 1)) -> rs.Car:
  return arena.add_car(team, random.randint(0, 5))

def return_self(args):
  return args

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

class FuzzyTestCase(unittest.TestCase):
  def assertEqual(self, first, second, msg=None):
    if type(first) is np.ndarray and type(second) is np.ndarray:
      if np.array_equal(first, second):
        return
    elif type(first) is rs.Vec and type(second) is rs.Vec:
      self.assertEqual(first=first.as_numpy(), second=second.as_numpy(), msg=msg)
      return
    else:
      unittest.TestCase.assertAlmostEqual(self, first=first, second=second, msg=msg)
      return

    raise self.failureException(f"\n{first}\n\t!=\n{second}")

  def assertAlmostEqual(self, first, second, places=7, msg=None, delta=None):
    if places is not None and delta is not None:
      raise TypeError("specify delta or places not both")

    if type(first) is np.ndarray and type(second) is np.ndarray:
      diff = np.amax(np.absolute(first - second))
    elif type(first) is rs.Vec and type(second) is rs.Vec:
      self.assertAlmostEqual(first=first.as_numpy(), second=second.as_numpy(), places=places, msg=msg, delta=delta)
      return
    else:
      unittest.TestCase.assertAlmostEqual(self, first=first, second=second, places=places, msg=msg, delta=delta)
      return

    if delta is not None:
      if diff <= delta:
        return
    else:
      if places is None:
        places = 7
      if round(diff, places) == 0:
        return

    raise self.failureException(f"\n{first}\n\t!=\n{second}")

class TestGameMode(FuzzyTestCase):
  def test_values(self):
    self.assertEqual(rs.GameMode.SOCCAR, 0)
    self.assertEqual(rs.GameMode.HOOPS, 1)
    self.assertEqual(rs.GameMode.HEATSEEKER, 2)
    self.assertEqual(rs.GameMode.SNOWDAY, 3)
    self.assertEqual(rs.GameMode.THE_VOID, 4)

class TestTeam(FuzzyTestCase):
  def test_values(self):
    self.assertEqual(rs.Team.BLUE, 0)
    self.assertEqual(rs.Team.ORANGE, 1)

class TestDemoMode(FuzzyTestCase):
  def test_values(self):
    self.assertEqual(rs.DemoMode.NORMAL, 0)
    self.assertEqual(rs.DemoMode.ON_CONTACT, 1)
    self.assertEqual(rs.DemoMode.DISABLED, 2)

class TestVec(FuzzyTestCase):
  def compare(self, vec: rs.Vec, x: float, y: float, z: float):
    self.assertAlmostEqual(vec.x, x)
    self.assertAlmostEqual(vec.y, y)
    self.assertAlmostEqual(vec.z, z)
    self.assertEqual(repr(vec), repr((x, y, z)))
    self.assertEqual(str(vec), str((x, y, z)))
    self.assertEqual(f"{vec}", f"{(x, y, z)}")
    self.assertEqual(f"{vec:.3f}", f"({x:.3f}, {y:.3f}, {z:.3f})")
    self.assertEqual(vec.as_tuple(), (x, y, z))
    self.assertTrue(np.array_equal(vec.as_numpy(), np.array([x, y, z])))

  def test_basic(self):
    self.compare(rs.Vec(), 0.0, 0.0, 0.0)
    self.compare(rs.Vec(1, 2, 3), 1.0, 2.0, 3.0)
    self.compare(rs.Vec(z=1, x=2, y=3), 2.0, 3.0, 1.0)
    self.compare(rs.Vec(z=2), 0.0, 0.0, 2.0)
    self.compare(rs.Vec(1, z=2), 1.0, 0.0, 2.0)

  def test_copy(self):
    vec_a = random_vec()
    vec_b = copy.copy(vec_a)

    self.assertEqual(vec_a.x, vec_b.x)
    self.assertEqual(vec_a.y, vec_b.y)
    self.assertEqual(vec_a.z, vec_b.z)

  def test_deep_copy(self):
    vec_a = random_vec()
    vec_b = copy.deepcopy(vec_a)

    self.assertEqual(vec_a.x, vec_b.x)
    self.assertEqual(vec_a.y, vec_b.y)
    self.assertEqual(vec_a.z, vec_b.z)

  def test_pickle(self):
    vec_a = random_vec()
    vec_b = pickled(vec_a)

    self.assertEqual(vec_a.x, vec_b.x)
    self.assertEqual(vec_a.y, vec_b.y)
    self.assertEqual(vec_a.z, vec_b.z)

class TestRotMat(FuzzyTestCase):
  def compare(self, mat, forward, right, up):
    self.assertEqual(mat.forward, forward)
    self.assertEqual(mat.right, right)
    self.assertEqual(mat.up, up)

  def test_basic(self):
    self.compare(rs.RotMat(),
      rs.Vec(1.0, 0.0, 0.0),
      rs.Vec(0.0, 1.0, 0.0),
      rs.Vec(0.0, 0.0, 1.0))
    self.compare(rs.RotMat(0, 1, 2, 3, 4, 5, 6, 7, 8),
      rs.Vec(0, 1, 2),
      rs.Vec(3, 4, 5),
      rs.Vec(6, 7, 8))
    self.compare(rs.RotMat(rs.Vec(0, 1, 2), rs.Vec(3, 4, 5), rs.Vec(6, 7, 8)),
      rs.Vec(0, 1, 2),
      rs.Vec(3, 4, 5),
      rs.Vec(6, 7, 8))
    self.compare(rs.RotMat(forward=rs.Vec(0, 1, 2), right=rs.Vec(3, 4, 5), up=rs.Vec(6, 7, 8)),
      rs.Vec(0, 1, 2),
      rs.Vec(3, 4, 5),
      rs.Vec(6, 7, 8))
    self.compare(rs.RotMat(up=rs.Vec(0, 1, 2), forward=rs.Vec(3, 4, 5), right=rs.Vec(6, 7, 8)),
      rs.Vec(3, 4, 5),
      rs.Vec(6, 7, 8),
      rs.Vec(0, 1, 2))

  def test_as_angle(self):
    forward = glm.normalize(glm.vec3(random_float(), random_float(), random_float()))
    right   = glm.normalize(glm.vec3(random_float(), random_float(), random_float()))
    up      = glm.normalize(glm.cross(forward, right))
    right   = glm.normalize(glm.cross(up, forward))

    src = rs.RotMat(
      forward = rs.Vec(forward.x, forward.y, forward.z),
      right   = rs.Vec(right.x,   right.y,   right.z),
      up      = rs.Vec(up.x,      up.y,      up.z)
    )

    dst = src.as_angle().as_rot_mat()

    self.assertAlmostEqual(src.forward.x, dst.forward.x, places=5)
    self.assertAlmostEqual(src.forward.y, dst.forward.y, places=5)
    self.assertAlmostEqual(src.forward.z, dst.forward.z, places=5)
    self.assertAlmostEqual(src.right.x,   dst.right.x,   places=5)
    self.assertAlmostEqual(src.right.y,   dst.right.y,   places=5)
    self.assertAlmostEqual(src.right.z,   dst.right.z,   places=5)
    self.assertAlmostEqual(src.up.x,      dst.up.x,      places=5)
    self.assertAlmostEqual(src.up.y,      dst.up.y,      places=5)
    self.assertAlmostEqual(src.up.z,      dst.up.z,      places=5)

  def test_copy(self):
    mat_a = rs.RotMat(
      forward = random_vec(),
      right   = random_vec(),
      up      = random_vec(),
    )
    mat_b = copy.copy(mat_a)

    self.assertIs(mat_a.forward, mat_b.forward)
    self.assertIs(mat_a.right,   mat_b.right)
    self.assertIs(mat_a.up,      mat_b.up)

  def test_deep_copy(self):
    mat_a = rs.RotMat(
      forward = random_vec(),
      right   = random_vec(),
      up      = random_vec(),
    )
    mat_b = copy.deepcopy(mat_a)

    self.assertIsNot(mat_a.forward, mat_b.forward)
    self.assertIsNot(mat_a.right,   mat_b.right)
    self.assertIsNot(mat_a.up,      mat_b.up)

    self.assertEqual(mat_a.forward, mat_b.forward)
    self.assertEqual(mat_a.right,   mat_b.right)
    self.assertEqual(mat_a.up,      mat_b.up)

  def test_pickle(self):
    mat_a = rs.RotMat(
      forward = random_vec(),
      right   = random_vec(),
      up      = random_vec(),
    )
    mat_b = pickled(mat_a)

    self.assertEqual(mat_a.forward, mat_b.forward)
    self.assertEqual(mat_a.right,   mat_b.right)
    self.assertEqual(mat_a.up,      mat_b.up)

class TestAngle(FuzzyTestCase):
  def compare(self, angle: rs.Angle, yaw: float, pitch: float, roll: float):
    self.assertEqual(angle.yaw, yaw)
    self.assertEqual(angle.pitch, pitch)
    self.assertEqual(angle.roll, roll)
    self.assertEqual(repr(angle), repr((yaw, pitch, roll)))
    self.assertEqual(str(angle), str((yaw, pitch, roll)))
    self.assertEqual(f"{angle}", f"{(yaw, pitch, roll)}")
    self.assertEqual(f"{angle:.3f}", f"({yaw:.3f}, {pitch:.3f}, {roll:.3f})")
    self.assertEqual(angle.as_tuple(), (yaw, pitch, roll))
    self.assertTrue(np.array_equal(angle.as_numpy(), np.array([yaw, pitch, roll])))

  def test_basic(self):
    self.compare(rs.Angle(), 0.0, 0.0, 0.0)
    self.compare(rs.Angle(1, 2, 3), 1.0, 2.0, 3.0)
    self.compare(rs.Angle(roll=1, yaw=2, pitch=3), 2.0, 3.0, 1.0)
    self.compare(rs.Angle(roll=2), 0.0, 0.0, 2.0)
    self.compare(rs.Angle(1, roll=2), 1.0, 0.0, 2.0)

  def test_as_rot_mat(self):
    src = random_angle()

    dst = src.as_rot_mat().as_angle()

    self.assertAlmostEqual(src.yaw,   dst.yaw,   places=6)
    self.assertAlmostEqual(src.pitch, dst.pitch, places=6)
    self.assertAlmostEqual(src.roll,  dst.roll,  places=6)

  def test_pickle(self):
    angle_a = random_angle ()
    angle_b = pickled(angle_a)

    self.assertEqual(angle_a.yaw,   angle_b.yaw)
    self.assertEqual(angle_a.pitch, angle_b.pitch)
    self.assertEqual(angle_a.roll,  angle_b.roll)

class TestBallHitInfo(FuzzyTestCase):
  def compare(self, ball_hit_a, ball_hit_b):
    self.assertEqual(ball_hit_a.is_valid,             ball_hit_b.is_valid)
    self.assertEqual(ball_hit_a.relative_pos_on_ball, ball_hit_b.relative_pos_on_ball)
    self.assertEqual(ball_hit_a.ball_pos,             ball_hit_b.ball_pos)
    self.assertEqual(ball_hit_a.extra_hit_vel,        ball_hit_b.extra_hit_vel)
    self.assertEqual(ball_hit_a.tick_count_when_hit,  ball_hit_b.tick_count_when_hit)
    self.assertEqual(ball_hit_a.tick_count_when_extra_impulse_applied,
                     ball_hit_b.tick_count_when_extra_impulse_applied)

  def test_basic(self):
    pass

  def test_copy(self):
    info_a = rs.BallHitInfo(
      is_valid             = random_bool(),
      relative_pos_on_ball = random_vec(),
      ball_pos             = random_vec(),
      extra_hit_vel        = random_vec(),
      tick_count_when_hit  = random_int(),
      tick_count_when_extra_impulse_applied = random_int()
    )

    info_b = copy.copy(info_a)

    self.assertIs(info_a.relative_pos_on_ball, info_b.relative_pos_on_ball)
    self.assertIs(info_a.ball_pos,             info_b.ball_pos)
    self.assertIs(info_a.extra_hit_vel,        info_b.extra_hit_vel)

    self.compare(info_a, info_b)

  def test_deep_copy(self):
    info_a = rs.BallHitInfo(
      is_valid             = random_bool(),
      relative_pos_on_ball = random_vec(),
      ball_pos             = random_vec(),
      extra_hit_vel        = random_vec(),
      tick_count_when_hit  = random_int(),
      tick_count_when_extra_impulse_applied = random_int()
    )

    info_b = copy.deepcopy(info_a)

    self.assertIsNot(info_a.relative_pos_on_ball, info_b.relative_pos_on_ball)
    self.assertIsNot(info_a.ball_pos,             info_b.ball_pos)
    self.assertIsNot(info_a.extra_hit_vel,        info_b.extra_hit_vel)

    self.compare(info_a, info_b)

  def test_pickle(self):
    info_a = rs.BallHitInfo(
      is_valid             = random_bool(),
      relative_pos_on_ball = random_vec(),
      ball_pos             = random_vec(),
      extra_hit_vel        = random_vec(),
      tick_count_when_hit  = random_int(),
      tick_count_when_extra_impulse_applied = random_int()
    )

    info_b = pickled(info_a)

    self.compare(info_a, info_b)

class TestBallState(FuzzyTestCase):
  def compare(self, state_a, state_b):
    self.assertAlmostEqual(state_a.pos,                 state_b.pos, 3)
    self.assertEqual(state_a.rot_mat.forward,           state_b.rot_mat.forward)
    self.assertEqual(state_a.rot_mat.right,             state_b.rot_mat.right)
    self.assertEqual(state_a.rot_mat.up,                state_b.rot_mat.up)
    self.assertAlmostEqual(state_a.vel,                 state_b.vel, 3)
    self.assertEqual(state_a.ang_vel,                   state_b.ang_vel)
    self.assertEqual(state_a.heatseeker_target_dir,     state_b.heatseeker_target_dir)
    self.assertEqual(state_a.heatseeker_target_speed,   state_b.heatseeker_target_speed)
    self.assertEqual(state_a.heatseeker_time_since_hit, state_b.heatseeker_time_since_hit)
    self.assertEqual(state_a.last_hit_car_id,           state_b.last_hit_car_id)
    self.assertEqual(state_a.update_counter,            state_b.update_counter)

  def compare_direct(self, state, pos, vel, ang_vel, car_id):
    self.assertEqual(state.pos, pos)
    self.assertEqual(state.vel, vel)
    self.assertEqual(state.ang_vel, ang_vel)
    self.assertEqual(state.last_hit_car_id, car_id)

  def test_basic(self):
    self.compare_direct(rs.BallState(), rs.Vec(0, 0, 93.15), rs.Vec(), rs.Vec(), 0)
    self.compare_direct(rs.BallState(rs.Vec()), rs.Vec(), rs.Vec(), rs.Vec(), 0)
    self.compare_direct(rs.BallState(vel=rs.Vec(1, 2, 3), last_hit_car_id=10), rs.Vec(0, 0, 93.15), rs.Vec(1, 2, 3), rs.Vec(), 10)

  def test_copy(self):
    state_a = rs.BallState(
      pos             = random_vec(),
      vel             = random_vec(),
      ang_vel         = random_vec(),
      last_hit_car_id = random_int(),
      update_counter  = random_int()
    )

    state_b = copy.copy(state_a)

    self.assertIs(state_a.pos,     state_b.pos)
    self.assertIs(state_a.vel,     state_b.vel)
    self.assertIs(state_a.ang_vel, state_b.ang_vel)

    self.compare(state_a, state_b)

  def test_deep_copy(self):
    state_a = rs.BallState(
      pos             = random_vec(),
      vel             = random_vec(),
      ang_vel         = random_vec(),
      last_hit_car_id = random_int(),
      update_counter  = random_int()
    )

    state_b = copy.deepcopy(state_a)

    self.assertIsNot(state_a.pos,     state_b.pos)
    self.assertIsNot(state_a.vel,     state_b.vel)
    self.assertIsNot(state_a.ang_vel, state_b.ang_vel)

    self.compare(state_a, state_b)

  def test_pickle(self):
    state_a = rs.BallState(
      pos             = random_vec(),
      vel             = random_vec(),
      ang_vel         = random_vec(),
      last_hit_car_id = random_int(),
      update_counter  = random_int()
    )

    state_b = pickled(state_a)

    self.assertIsNot(state_a.pos,     state_b.pos)
    self.assertIsNot(state_a.vel,     state_b.vel)
    self.assertIsNot(state_a.ang_vel, state_b.ang_vel)

    self.compare(state_a, state_b)

class TestBall(FuzzyTestCase):
  def compare(self, ball_a, ball_b):
    self.assertEqual(ball_a.get_radius(), ball_b.get_radius())
    TestBallState.compare(self, ball_a.get_state(), ball_b.get_state())

  def test_create(self):
    with self.assertRaises(TypeError):
      rs.Ball()

class TestBoostPadState(FuzzyTestCase):
  def compare(self, state_a, state_b):
    self.assertEqual(state_a.is_active,          state_b.is_active)
    self.assertEqual(state_a.cooldown,           state_b.cooldown)
    self.assertEqual(state_a.prev_locked_car_id, state_b.prev_locked_car_id)

  def test_basic(self):
    pass

  def test_pickle(self):
    state_a = rs.BoostPadState(
      is_active          = random_bool(),
      cooldown           = random_float(),
      prev_locked_car_id = random_int(),
    )

    state_b = pickled(state_a)

    self.compare(state_a, state_b)

  def test_copy(self):
    state_a = rs.BoostPadState(
      is_active          = random_bool(),
      cooldown           = random_float(),
      prev_locked_car_id = random_int(),
    )

    state_b = copy.copy(state_a)

    self.compare(state_a, state_b)

  def test_deep_copy(self):
    state_a = rs.BoostPadState(
      is_active          = random_bool(),
      cooldown           = random_float(),
      prev_locked_car_id = random_int(),
    )

    state_b = copy.copy(state_a)

    self.compare(state_a, state_b)

class TestBoostPad(FuzzyTestCase):
  def compare(self, pad_a, pad_b):
    self.assertEqual(pad_a.get_pos(), pad_b.get_pos())
    self.assertEqual(pad_a.is_big,    pad_b.is_big)

    TestBoostPadState.compare(self, pad_a.get_state(), pad_b.get_state())

  def test_create(self):
    with self.assertRaises(TypeError):
      rs.BoostPad()

class TestWheelPairConfig(FuzzyTestCase):
  def compare(self, config_a, config_b):
    self.assertEqual(config_a.wheel_radius,            config_b.wheel_radius)
    self.assertEqual(config_a.suspension_rest_length,  config_b.suspension_rest_length)
    self.assertEqual(config_a.connection_point_offset, config_b.connection_point_offset)

  def test_basic(self):
    pass

  def test_pickle(self):
    config_a = rs.WheelPairConfig(
      wheel_radius            = random_float(),
      suspension_rest_length  = random_float(),
      connection_point_offset = random_vec()
    )

    config_b = pickled(config_a)

    self.compare(config_a, config_b)

  def test_copy(self):
    config_a = rs.WheelPairConfig(
      wheel_radius            = random_float(),
      suspension_rest_length  = random_float(),
      connection_point_offset = random_vec()
    )

    config_b = copy.copy(config_a)

    self.compare(config_a, config_b)

  def test_deep_copy(self):
    config_a = rs.WheelPairConfig(
      wheel_radius            = random_float(),
      suspension_rest_length  = random_float(),
      connection_point_offset = random_vec()
    )

    config_b = copy.deepcopy(config_a)

    self.compare(config_a, config_b)

class TestCarConfig(FuzzyTestCase):
  def compare(self, config_a, config_b):
    self.assertEqual(config_a.hitbox_size,                   config_b.hitbox_size)
    self.assertEqual(config_a.hitbox_pos_offset,             config_b.hitbox_pos_offset)
    TestWheelPairConfig.compare(self, config_a.front_wheels, config_b.front_wheels)
    TestWheelPairConfig.compare(self, config_a.back_wheels,  config_b.back_wheels)
    self.assertEqual(config_a.dodge_deadzone,                config_b.dodge_deadzone)

  def test_values(self):
    self.assertEqual(rs.CarConfig.OCTANE, 0)
    self.assertEqual(rs.CarConfig.DOMINUS, 1)
    self.assertEqual(rs.CarConfig.PLANK, 2)
    self.assertEqual(rs.CarConfig.BREAKOUT, 3)
    self.assertEqual(rs.CarConfig.HYBRID, 4)
    self.assertEqual(rs.CarConfig.MERC, 5)

  def test_create(self):
    self.assertEqual(rs.CarConfig().dodge_deadzone, 0.5)
    self.assertAlmostEqual(rs.CarConfig().hitbox_size.x, 120.5070, places=4)

    self.assertAlmostEqual(rs.CarConfig(rs.CarConfig.OCTANE).hitbox_size.x,   120.5070, places=4)
    self.assertAlmostEqual(rs.CarConfig(rs.CarConfig.DOMINUS).hitbox_size.x,  130.4270, places=4)
    self.assertAlmostEqual(rs.CarConfig(rs.CarConfig.PLANK).hitbox_size.x,    131.3200, places=4)
    self.assertAlmostEqual(rs.CarConfig(rs.CarConfig.BREAKOUT).hitbox_size.x, 133.9920, places=4)
    self.assertAlmostEqual(rs.CarConfig(rs.CarConfig.HYBRID).hitbox_size.x,   129.5190, places=4)
    self.assertAlmostEqual(rs.CarConfig(rs.CarConfig.MERC).hitbox_size.x,     123.2200, places=4)

  def test_pickle(self):
    config_a = rs.CarConfig(
      hitbox_size       = random_vec(),
      hitbox_pos_offset = random_vec(),
      front_wheels      = rs.WheelPairConfig(
        wheel_radius            = random_float(),
        suspension_rest_length  = random_float(),
        connection_point_offset = random_vec()
      ),
      back_wheels       = rs.WheelPairConfig(
        wheel_radius            = random_float(),
        suspension_rest_length  = random_float(),
        connection_point_offset = random_vec()
      ),
      dodge_deadzone = random_float()
    )

    config_b = pickled(config_a)

    self.compare(config_a, config_b)

  def test_copy(self):
    config_a = rs.CarConfig(
      hitbox_size       = random_vec(),
      hitbox_pos_offset = random_vec(),
      front_wheels      = rs.WheelPairConfig(
        wheel_radius            = random_float(),
        suspension_rest_length  = random_float(),
        connection_point_offset = random_vec()
      ),
      back_wheels       = rs.WheelPairConfig(
        wheel_radius            = random_float(),
        suspension_rest_length  = random_float(),
        connection_point_offset = random_vec()
      ),
      dodge_deadzone = random_float()
    )

    config_b = copy.copy(config_a)

    self.assertIs(config_a.hitbox_size,       config_b.hitbox_size)
    self.assertIs(config_a.hitbox_pos_offset, config_b.hitbox_pos_offset)
    self.assertIs(config_a.front_wheels,      config_b.front_wheels)
    self.assertIs(config_a.back_wheels,       config_b.back_wheels)

    self.compare(config_a, config_b)

  def test_deep_copy(self):
    config_a = rs.CarConfig(
      hitbox_size       = random_vec(),
      hitbox_pos_offset = random_vec(),
      front_wheels      = rs.WheelPairConfig(
        wheel_radius            = random_float(),
        suspension_rest_length  = random_float(),
        connection_point_offset = random_vec()
      ),
      back_wheels       = rs.WheelPairConfig(
        wheel_radius            = random_float(),
        suspension_rest_length  = random_float(),
        connection_point_offset = random_vec()
      ),
      dodge_deadzone = random_float()
    )
    config_b = copy.deepcopy(config_a)

    self.assertIsNot(config_a.hitbox_size,       config_b.hitbox_size)
    self.assertIsNot(config_a.hitbox_pos_offset, config_b.hitbox_pos_offset)
    self.assertIsNot(config_a.front_wheels,      config_b.front_wheels)
    self.assertIsNot(config_a.back_wheels,       config_b.back_wheels)

    self.compare(config_a, config_b)

class TestCarControls(FuzzyTestCase):
  def compare(self, controls_a, controls_b):
    self.assertEqual(controls_a.throttle,  controls_b.throttle)
    self.assertEqual(controls_a.steer,     controls_b.steer)
    self.assertEqual(controls_a.yaw,       controls_b.yaw)
    self.assertEqual(controls_a.pitch,     controls_b.pitch)
    self.assertEqual(controls_a.roll,      controls_b.roll)
    self.assertEqual(controls_a.boost,     controls_b.boost)
    self.assertEqual(controls_a.jump,      controls_b.jump)
    self.assertEqual(controls_a.handbrake, controls_b.handbrake)

  def compare_dict(self, controls, attrs):
    for attr in dir(controls):
      if attr.startswith("__"):
        continue

      val = getattr(controls, attr)
      if callable(val):
        continue

      if attr in attrs:
        self.assertEqual(val, attrs[attr])
      else:
        self.assertFalse(val)

  def test_basic(self):
    self.compare_dict(rs.CarControls(), {})
    self.compare_dict(rs.CarControls(throttle=1.0), {"throttle": 1.0})
    self.compare_dict(rs.CarControls(steer=2.0), {"steer": 2.0})
    self.compare_dict(rs.CarControls(pitch=3.0), {"pitch": 3.0})
    self.compare_dict(rs.CarControls(yaw=4.0), {"yaw": 4.0})
    self.compare_dict(rs.CarControls(roll=5.0), {"roll": 5.0})
    self.compare_dict(rs.CarControls(boost=True), {"boost": True})
    self.compare_dict(rs.CarControls(jump=True), {"jump": True})
    self.compare_dict(rs.CarControls(handbrake=True), {"handbrake": True})
    self.compare_dict(rs.CarControls(jump=True, steer=2.0), {"steer": 2.0, "jump": True})
    self.compare_dict(rs.CarControls(0.125, 0.25, 0.5, boost=True),
      {
        "throttle": 0.125,
        "steer": 0.25,
        "pitch": 0.5,
        "boost": True,
      })

  def test_pickle(self):
    controls_a = rs.CarControls(
      throttle  = random_float(),
      steer     = random_float(),
      yaw       = random_float(),
      pitch     = random_float(),
      roll      = random_float(),
      boost     = random_bool(),
      jump      = random_bool(),
      handbrake = random_bool()
    )

    controls_b = pickled(controls_a)

    self.compare(controls_a, controls_b)

  def test_copy(self):
    controls_a = rs.CarControls(
      throttle  = random_float(),
      steer     = random_float(),
      yaw       = random_float(),
      pitch     = random_float(),
      roll      = random_float(),
      boost     = random_bool(),
      jump      = random_bool(),
      handbrake = random_bool()
    )

    controls_b = copy.copy(controls_a)

    self.compare(controls_a, controls_b)

  def test_deep_copy(self):
    controls_a = rs.CarControls(
      throttle  = random_float(),
      steer     = random_float(),
      yaw       = random_float(),
      pitch     = random_float(),
      roll      = random_float(),
      boost     = random_bool(),
      jump      = random_bool(),
      handbrake = random_bool()
    )

    controls_b = copy.deepcopy(controls_a)

    self.compare(controls_a, controls_b)

class TestCarState(FuzzyTestCase):
  def compare(self, state_a, state_b):
    self.assertAlmostEqual(state_a.pos,                  state_b.pos, 3)
    self.assertEqual(state_a.rot_mat.forward,            state_b.rot_mat.forward)
    self.assertEqual(state_a.rot_mat.right,              state_b.rot_mat.right)
    self.assertEqual(state_a.rot_mat.up,                 state_b.rot_mat.up)
    self.assertAlmostEqual(state_a.vel,                  state_b.vel, 3)
    self.assertEqual(state_a.ang_vel,                    state_b.ang_vel)
    self.assertEqual(state_a.is_on_ground,               state_b.is_on_ground)
    self.assertEqual(state_a.has_jumped,                 state_b.has_jumped)
    self.assertEqual(state_a.has_double_jumped,          state_b.has_double_jumped)
    self.assertEqual(state_a.has_flipped,                state_b.has_flipped)
    self.assertEqual(state_a.last_rel_dodge_torque,      state_b.last_rel_dodge_torque)
    self.assertEqual(state_a.jump_time,                  state_b.jump_time)
    self.assertEqual(state_a.flip_time,                  state_b.flip_time)
    self.assertEqual(state_a.is_jumping,                 state_b.is_jumping)
    self.assertEqual(state_a.air_time_since_jump,        state_b.air_time_since_jump)
    self.assertEqual(state_a.boost,                      state_b.boost)
    self.assertEqual(state_a.time_spent_boosting,        state_b.time_spent_boosting)
    self.assertEqual(state_a.is_supersonic,              state_b.is_supersonic)
    self.assertEqual(state_a.supersonic_time,            state_b.supersonic_time)
    self.assertEqual(state_a.handbrake_val,              state_b.handbrake_val)
    self.assertEqual(state_a.is_auto_flipping,           state_b.is_auto_flipping)
    self.assertEqual(state_a.auto_flip_timer,            state_b.auto_flip_timer)
    self.assertEqual(state_a.has_world_contact,          state_b.has_world_contact)
    self.assertEqual(state_a.world_contact_normal,       state_b.world_contact_normal)
    self.assertEqual(state_a.car_contact_id,             state_b.car_contact_id)
    self.assertEqual(state_a.car_contact_cooldown_timer, state_b.car_contact_cooldown_timer)
    self.assertEqual(state_a.is_demoed,                  state_b.is_demoed)
    self.assertEqual(state_a.demo_respawn_timer,         state_b.demo_respawn_timer)
    self.assertEqual(state_a.update_counter,             state_b.update_counter)

    TestBallHitInfo.compare(self, state_a.ball_hit_info, state_b.ball_hit_info)
    TestCarControls.compare(self, state_a.last_controls, state_b.last_controls)

  def test_basic(self):
    pass

  def test_pickle(self):
    state_a = rs.CarState(
      pos                        = random_vec(),
      rot_mat                    = random_mat(),
      vel                        = random_vec(),
      ang_vel                    = random_vec(),
      is_on_ground               = random_bool(),
      has_jumped                 = random_bool(),
      has_double_jumped          = random_bool(),
      has_flipped                = random_bool(),
      last_rel_dodge_torque      = random_vec(),
      jump_time                  = random_float(),
      flip_time                  = random_float(),
      is_flipping                = random_bool(),
      is_jumping                 = random_bool(),
      air_time_since_jump        = random_float(),
      boost                      = random_float(),
      time_spent_boosting        = random_float(),
      is_supersonic              = random_bool(),
      supersonic_time            = random_float(),
      handbrake_val              = random_float(),
      is_auto_flipping           = random_bool(),
      auto_flip_timer            = random_float(),
      has_world_contact          = random_bool(),
      world_contact_normal       = random_vec(),
      car_contact_id             = random_int(),
      car_contact_cooldown_timer = random_float(),
      is_demoed                  = random_bool(),
      demo_respawn_timer         = random_float(),
      ball_hit_info = rs.BallHitInfo(
        is_valid             = random_bool(),
        relative_pos_on_ball = random_vec(),
        ball_pos             = random_vec(),
        extra_hit_vel        = random_vec(),
        tick_count_when_hit  = random_int(),
        tick_count_when_extra_impulse_applied = random_int()
      ),
      last_controls = rs.CarControls(
        throttle  = random_float(),
        steer     = random_float(),
        yaw       = random_float(),
        pitch     = random_float(),
        roll      = random_float(),
        boost     = random_bool(),
        jump      = random_bool(),
        handbrake = random_bool()
      ),
      update_counter = random_int()
    )

    state_b = pickled(state_a)

    self.compare(state_a, state_b)

  def test_copy(self):
    state_a = rs.CarState(
      pos                        = random_vec(),
      rot_mat                    = random_mat(),
      vel                        = random_vec(),
      ang_vel                    = random_vec(),
      is_on_ground               = random_bool(),
      has_jumped                 = random_bool(),
      has_double_jumped          = random_bool(),
      has_flipped                = random_bool(),
      last_rel_dodge_torque      = random_vec(),
      jump_time                  = random_float(),
      flip_time                  = random_float(),
      is_flipping                = random_bool(),
      is_jumping                 = random_bool(),
      air_time_since_jump        = random_float(),
      boost                      = random_float(),
      time_spent_boosting        = random_float(),
      is_supersonic              = random_bool(),
      supersonic_time            = random_float(),
      handbrake_val              = random_float(),
      is_auto_flipping           = random_bool(),
      auto_flip_timer            = random_float(),
      has_world_contact          = random_bool(),
      world_contact_normal       = random_vec(),
      car_contact_id             = random_int(),
      car_contact_cooldown_timer = random_float(),
      is_demoed                  = random_bool(),
      demo_respawn_timer         = random_float(),
      ball_hit_info = rs.BallHitInfo(
        is_valid             = random_bool(),
        relative_pos_on_ball = random_vec(),
        ball_pos             = random_vec(),
        extra_hit_vel        = random_vec(),
        tick_count_when_hit  = random_int(),
        tick_count_when_extra_impulse_applied = random_int()
      ),
      last_controls = rs.CarControls(
        throttle  = random_float(),
        steer     = random_float(),
        yaw       = random_float(),
        pitch     = random_float(),
        roll      = random_float(),
        boost     = random_bool(),
        jump      = random_bool(),
        handbrake = random_bool()
      ),
      update_counter = random_int()
    )

    state_b = copy.copy(state_a)

    self.assertIs(state_a.pos,                  state_b.pos)
    self.assertIs(state_a.rot_mat,              state_b.rot_mat)
    self.assertIs(state_a.vel,                  state_b.vel)
    self.assertIs(state_a.ang_vel,              state_b.ang_vel)
    self.assertIs(state_a.world_contact_normal, state_b.world_contact_normal)
    self.assertIs(state_a.last_controls,        state_b.last_controls)

    self.compare(state_a, state_b)

  def test_deep_copy(self):
    state_a = rs.CarState(
      pos                        = random_vec(),
      rot_mat                    = random_mat(),
      vel                        = random_vec(),
      ang_vel                    = random_vec(),
      is_on_ground               = random_bool(),
      has_jumped                 = random_bool(),
      has_double_jumped          = random_bool(),
      has_flipped                = random_bool(),
      last_rel_dodge_torque      = random_vec(),
      jump_time                  = random_float(),
      flip_time                  = random_float(),
      is_flipping                = random_bool(),
      is_jumping                 = random_bool(),
      air_time_since_jump        = random_float(),
      boost                      = random_float(),
      time_spent_boosting        = random_float(),
      is_supersonic              = random_bool(),
      supersonic_time            = random_float(),
      handbrake_val              = random_float(),
      is_auto_flipping           = random_bool(),
      auto_flip_timer            = random_float(),
      has_world_contact          = random_bool(),
      world_contact_normal       = random_vec(),
      car_contact_id             = random_int(),
      car_contact_cooldown_timer = random_float(),
      is_demoed                  = random_bool(),
      demo_respawn_timer         = random_float(),
      ball_hit_info = rs.BallHitInfo(
        is_valid             = random_bool(),
        relative_pos_on_ball = random_vec(),
        ball_pos             = random_vec(),
        extra_hit_vel        = random_vec(),
        tick_count_when_hit  = random_int(),
        tick_count_when_extra_impulse_applied = random_int()
      ),
      last_controls = rs.CarControls(
        throttle  = random_float(),
        steer     = random_float(),
        yaw       = random_float(),
        pitch     = random_float(),
        roll      = random_float(),
        boost     = random_bool(),
        jump      = random_bool(),
        handbrake = random_bool()
      ),
      update_counter = random_int()
    )

    state_b = copy.deepcopy(state_a)

    self.assertIsNot(state_a.pos,                  state_b.pos)
    self.assertIsNot(state_a.rot_mat,              state_b.rot_mat)
    self.assertIsNot(state_a.vel,                  state_b.vel)
    self.assertIsNot(state_a.ang_vel,              state_b.ang_vel)
    self.assertIsNot(state_a.world_contact_normal, state_b.world_contact_normal)
    self.assertIsNot(state_a.last_controls,        state_b.last_controls)

    self.compare(state_a, state_b)

class TestCar(FuzzyTestCase):
  def compare(self, car_a, car_b):
    self.assertEqual(car_a.id,            car_b.id)
    self.assertEqual(car_a.team,          car_b.team)
    self.assertEqual(car_a.goals,         car_b.goals)
    self.assertEqual(car_a.demos,         car_b.demos)
    self.assertEqual(car_a.boost_pickups, car_b.boost_pickups)
    self.assertEqual(car_a.shots,         car_b.shots)
    self.assertEqual(car_a.saves,         car_b.saves)
    self.assertEqual(car_a.assists,       car_b.assists)

    TestCarState.compare(self, car_a.get_state(), car_b.get_state())
    TestCarConfig.compare(self, car_a.get_config(), car_b.get_config())

  def test_create(self):
    with self.assertRaises(TypeError):
      rs.Car()

  def test_demo_pos(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    car_a = arena.add_car(rs.Team.BLUE)
    car_b = arena.add_car(rs.Team.ORANGE)

    car_state = car_a.get_state()
    car_state.pos.x = -1000
    car_state.pos.y = 1000
    car_state.rot_mat = rs.RotMat()
    car_state.vel.x = 2300
    car_a.set_state(car_state)

    car_state = car_b.get_state()
    car_state.pos.x = 1000
    car_state.pos.y = 1000
    car_b.set_state(car_state)

    car_a.set_controls(rs.CarControls(throttle=1.0, boost=True))

    demo_pos = []
    def handle_demo(arena, bumper, victim, data):
      demo_pos.append(victim.get_state().pos)

    arena.set_car_demo_callback(handle_demo)

    for i in range(120):
      arena.step()
      if len(demo_pos):
        state = car_b.get_state()
        self.assertEqual(state.pos, demo_pos[0])

    self.assertNotEqual(len(demo_pos), 0)

class TestMutatorConfig(FuzzyTestCase):
  def compare(self, config_a, config_b):
    self.assertEqual(config_a.gravity,                    config_b.gravity)
    self.assertEqual(config_a.car_mass,                   config_b.car_mass)
    self.assertEqual(config_a.car_world_friction,         config_b.car_world_friction)
    self.assertEqual(config_a.car_world_restitution,      config_b.car_world_restitution)
    self.assertEqual(config_a.ball_mass,                  config_b.ball_mass)
    self.assertEqual(config_a.ball_max_speed,             config_b.ball_max_speed)
    self.assertEqual(config_a.ball_drag,                  config_b.ball_drag)
    self.assertEqual(config_a.ball_world_friction,        config_b.ball_world_friction)
    self.assertEqual(config_a.ball_world_restitution,     config_b.ball_world_restitution)
    self.assertEqual(config_a.jump_accel,                 config_b.jump_accel)
    self.assertEqual(config_a.jump_immediate_force,       config_b.jump_immediate_force)
    self.assertEqual(config_a.boost_accel,                config_b.boost_accel)
    self.assertEqual(config_a.boost_used_per_second,      config_b.boost_used_per_second)
    self.assertEqual(config_a.respawn_delay,              config_b.respawn_delay)
    self.assertEqual(config_a.bump_cooldown_time,         config_b.bump_cooldown_time)
    self.assertEqual(config_a.boost_pad_cooldown_big,     config_b.boost_pad_cooldown_big)
    self.assertEqual(config_a.boost_pad_cooldown_small,   config_b.boost_pad_cooldown_small)
    self.assertEqual(config_a.car_spawn_boost_amount,     config_b.car_spawn_boost_amount)
    self.assertEqual(config_a.ball_hit_extra_force_scale, config_b.ball_hit_extra_force_scale)
    self.assertEqual(config_a.bump_force_scale,           config_b.bump_force_scale)
    self.assertEqual(config_a.ball_radius,                config_b.ball_radius)
    self.assertEqual(config_a.unlimited_flips,            config_b.unlimited_flips)
    self.assertEqual(config_a.unlimited_double_jumps,     config_b.unlimited_double_jumps)
    self.assertEqual(config_a.demo_mode,                  config_b.demo_mode)
    self.assertEqual(config_a.enable_team_demos,          config_b.enable_team_demos)
    self.assertEqual(config_a.enable_car_car_collision,   config_b.enable_car_car_collision)
    self.assertEqual(config_a.enable_car_ball_collision,  config_b.enable_car_ball_collision)

  def test_basic(self):
    config = rs.MutatorConfig(rs.GameMode.SOCCAR)
    config = rs.MutatorConfig(rs.GameMode.HOOPS)
    config = rs.MutatorConfig(rs.GameMode.HEATSEEKER)
    config = rs.MutatorConfig(rs.GameMode.SNOWDAY)
    config = rs.MutatorConfig(rs.GameMode.THE_VOID)

  def test_pickle(self):
    config_a = rs.MutatorConfig(
      gravity                    = random_vec(),
      car_mass                   = random_float(),
      car_world_friction         = random_float(),
      car_world_restitution      = random_float(),
      ball_mass                  = random_float(),
      ball_max_speed             = random_float(),
      ball_drag                  = random_float(),
      ball_world_friction        = random_float(),
      ball_world_restitution     = random_float(),
      jump_accel                 = random_float(),
      jump_immediate_force       = random_float(),
      boost_accel                = random_float(),
      boost_used_per_second      = random_float(),
      respawn_delay              = random_float(),
      bump_cooldown_time         = random_float(),
      boost_pad_cooldown_big     = random_float(),
      boost_pad_cooldown_small   = random_float(),
      car_spawn_boost_amount     = random_float(),
      ball_hit_extra_force_scale = random_float(),
      bump_force_scale           = random_float(),
      ball_radius                = random_float(),
      unlimited_flips            = random_bool(),
      unlimited_double_jumps     = random_bool(),
      demo_mode                  = random.randint(0, 2),
      enable_team_demos          = random_bool(),
      enable_car_car_collision   = random_bool(),
      enable_car_ball_collision  = random_bool()
    )
    config_b = pickled(config_a)

    self.assertIsNot(config_a.gravity, config_b.gravity)
    self.compare(config_a, config_b)

  def test_copy(self):
    config_a = rs.MutatorConfig(
      gravity                    = random_vec(),
      car_mass                   = random_float(),
      car_world_friction         = random_float(),
      car_world_restitution      = random_float(),
      ball_mass                  = random_float(),
      ball_max_speed             = random_float(),
      ball_drag                  = random_float(),
      ball_world_friction        = random_float(),
      ball_world_restitution     = random_float(),
      jump_accel                 = random_float(),
      jump_immediate_force       = random_float(),
      boost_accel                = random_float(),
      boost_used_per_second      = random_float(),
      respawn_delay              = random_float(),
      bump_cooldown_time         = random_float(),
      boost_pad_cooldown_big     = random_float(),
      boost_pad_cooldown_small   = random_float(),
      car_spawn_boost_amount     = random_float(),
      ball_hit_extra_force_scale = random_float(),
      bump_force_scale           = random_float(),
      ball_radius                = random_float(),
      unlimited_flips            = random_bool(),
      unlimited_double_jumps     = random_bool(),
      demo_mode                  = random.randint(0, 2),
      enable_team_demos          = random_bool(),
      enable_car_car_collision   = random_bool(),
      enable_car_ball_collision  = random_bool()
    )
    config_b = copy.copy(config_a)

    self.assertIs(config_a.gravity, config_b.gravity)
    self.compare(config_a, config_b)

  def test_deep_copy(self):
    config_a = rs.MutatorConfig(
      gravity                    = random_vec(),
      car_mass                   = random_float(),
      car_world_friction         = random_float(),
      car_world_restitution      = random_float(),
      ball_mass                  = random_float(),
      ball_max_speed             = random_float(),
      ball_drag                  = random_float(),
      ball_world_friction        = random_float(),
      ball_world_restitution     = random_float(),
      jump_accel                 = random_float(),
      jump_immediate_force       = random_float(),
      boost_accel                = random_float(),
      boost_used_per_second      = random_float(),
      respawn_delay              = random_float(),
      bump_cooldown_time         = random_float(),
      boost_pad_cooldown_big     = random_float(),
      boost_pad_cooldown_small   = random_float(),
      car_spawn_boost_amount     = random_float(),
      ball_hit_extra_force_scale = random_float(),
      bump_force_scale           = random_float(),
      ball_radius                = random_float(),
      unlimited_flips            = random_bool(),
      unlimited_double_jumps     = random_bool(),
      demo_mode                  = random.randint(0, 2),
      enable_team_demos          = random_bool(),
      enable_car_car_collision   = random_bool(),
      enable_car_ball_collision  = random_bool()
    )
    config_b = copy.deepcopy(config_a)

    self.assertIsNot(config_a.gravity, config_b.gravity)
    self.compare(config_a, config_b)

class TestArena(FuzzyTestCase):
  def compare(self, arena_a, arena_b):
    self.assertEqual(arena_a.game_mode,    arena_b.game_mode)
    self.assertEqual(arena_a.tick_time,    arena_b.tick_time)
    self.assertEqual(arena_a.tick_count,   arena_b.tick_count)
    self.assertEqual(arena_a.blue_score,   arena_b.blue_score)
    self.assertEqual(arena_a.orange_score, arena_b.orange_score)

    TestMutatorConfig.compare(self, arena_a.get_mutator_config(), arena_b.get_mutator_config())
    TestBall.compare(self, arena_a.ball, arena_b.ball)

    cars_a = arena_a.get_cars()
    cars_b = arena_b.get_cars()
    self.assertEqual(len(cars_a), len(cars_b))
    self.assertGreater(len(cars_a), 0)

    for car_a, car_b in zip(cars_a, cars_b):
      car_b = arena_b.get_car_from_id(car_a.id, None)
      self.assertIsNot(car_a, car_b)
      TestCar.compare(self, car_a, car_b)

    pads_a = arena_a.get_boost_pads()
    pads_b = arena_b.get_boost_pads()
    self.assertEqual(len(pads_a), len(pads_b))

    for pad_a, pad_b in zip(pads_a, pads_b):
      self.assertIsNot(pad_a, pad_b)
      TestBoostPad.compare(self, pad_a, pad_b)

  def test_basic(self):
    arena = rs.Arena(rs.GameMode.THE_VOID, rs.MemoryWeightMode.LIGHT)
    arena = rs.Arena(rs.GameMode.THE_VOID, rs.MemoryWeightMode.HEAVY)

    arena = rs.Arena(rs.GameMode.SOCCAR)
    arena = rs.Arena(rs.GameMode.HOOPS)
    arena = rs.Arena(rs.GameMode.HEATSEEKER)
    arena = rs.Arena(rs.GameMode.SNOWDAY)

  def test_boost_pad_order(self):
    SOCCAR_BOOST_PADS = [
      (    0.0, -4240.0),
      (-1792.0, -4184.0),
      ( 1792.0, -4184.0),
      (-3072.0, -4096.0),
      ( 3072.0, -4096.0),
      (- 940.0, -3308.0),
      (  940.0, -3308.0),
      (    0.0, -2816.0),
      (-3584.0, -2484.0),
      ( 3584.0, -2484.0),
      (-1788.0, -2300.0),
      ( 1788.0, -2300.0),
      (-2048.0, -1036.0),
      (    0.0, -1024.0),
      ( 2048.0, -1036.0),
      (-3584.0,     0.0),
      (-1024.0,     0.0),
      ( 1024.0,     0.0),
      ( 3584.0,     0.0),
      (-2048.0,  1036.0),
      (    0.0,  1024.0),
      ( 2048.0,  1036.0),
      (-1788.0,  2300.0),
      ( 1788.0,  2300.0),
      (-3584.0,  2484.0),
      ( 3584.0,  2484.0),
      (    0.0,  2816.0),
      (- 940.0,  3308.0),
      (  940.0,  3308.0),
      (-3072.0,  4096.0),
      ( 3072.0,  4096.0),
      (-1792.0,  4184.0),
      ( 1792.0,  4184.0),
      (    0.0,  4240.0),
    ]

    HOOPS_BOOST_PADS = [
      (-2176.0,  -2944.0),
      ( 2176.0,  -2944.0),
      (    0.0,  -2816.0),
      (-1280.0,  -2304.0),
      ( 1280.0,  -2304.0),
      (-1536.0,  -1024.0),
      ( 1536.0,  -1024.0),
      (- 512.0,  - 512.0),
      (  512.0,  - 512.0),
      (-2432.0,      0.0),
      ( 2432.0,      0.0),
      (- 512.0,    512.0),
      (  512.0,    512.0),
      (-1536.0,   1024.0),
      ( 1536.0,   1024.0),
      (-1280.0,   2304.0),
      ( 1280.0,   2304.0),
      (    0.0,   2816.0),
      (-2176.0,   2944.0),
      ( 2175.99,  2944.0),
    ]

    for mode, locations in (
      (rs.GameMode.SOCCAR, SOCCAR_BOOST_PADS),
      (rs.GameMode.HOOPS, HOOPS_BOOST_PADS),
      (rs.GameMode.HEATSEEKER, SOCCAR_BOOST_PADS),
      (rs.GameMode.SNOWDAY, SOCCAR_BOOST_PADS),
      (rs.GameMode.THE_VOID, [])
    ):
      arena = rs.Arena(mode)
      arena_pads = arena.get_boost_pads()
      self.assertEqual(len(arena_pads), len(locations))

      for pad, pos in zip(arena_pads, locations):
        pad_pos = pad.get_pos()
        self.assertAlmostEqual (pad_pos.x, pos[0], 4)
        self.assertAlmostEqual (pad_pos.y, pos[1], 4)

  def test_car_order(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    cars = []
    for i in range(100):
      cars.append(random_car(arena))

    arena_cars = arena.get_cars()
    self.assertEqual(len(cars), len(arena_cars))
    self.assertEqual(len(cars), 100)

    for car, arena_car in zip(cars, arena_cars):
      self.assertIs(car, arena_car)

  def test_stop(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    car = random_car(arena)

    tick = [0]
    def ball_touch_callback(arena: rs.Arena, car: rs.Car, data):
      arena.stop()
      data[0] = arena.tick_count

    arena.set_ball_touch_callback(ball_touch_callback, tick)

    for i in range(100):
      target_chase(arena.ball.get_state().pos, car)
      arena.step(8)
      if tick[0] > 0:
        break

    self.assertEqual(arena.tick_count, tick[0] + 1)

  def test_multi_step(self):
    arenas = [rs.Arena(rs.GameMode.SOCCAR, rs.MemoryWeightMode.HEAVY) for i in range(24)]

    for arena in arenas:
      car = arena.add_car(rs.Team.BLUE)
      arena.reset_kickoff(seed=999)

    for arena in arenas[1:]:
      self.compare(arena, arenas[0])

    rs.Arena.multi_step(arenas, 1)

    for i in range(10000 // 8):
      for arena in arenas:
        for car in arena.get_cars():
          target_chase(arena.ball.get_state().pos, car)
      rs.Arena.multi_step(arenas, 8)

    for arena in arenas[1:]:
      self.compare(arena, arenas[0])

  def test_multi_step_exception(self):
    class BallTouchError(Exception):
      def __init__(self):
        super().__init__()

    def ball_touch_callback(arena: rs.Arena, car: rs.Car, data):
      data[0][data[1]] = arena.tick_count
      raise BallTouchError()

    arenas = [rs.Arena(rs.GameMode.SOCCAR, rs.MemoryWeightMode.HEAVY) for i in range(8)]
    touched = [0 for _ in arenas]

    for i, arena in enumerate(arenas):
      car = arena.add_car(rs.Team.BLUE)
      arena.reset_kickoff(seed=999)
      arena.set_ball_touch_callback(ball_touch_callback, [touched, i])

    for arena in arenas[1:]:
      self.compare(arena, arenas[0])

    with self.assertRaises(BallTouchError):
      for i in range(10000 // 8):
        for arena in arenas:
          for car in arena.get_cars():
            target_chase(arena.ball.get_state().pos, car)
        rs.Arena.multi_step(arenas, 8)

    # Each arena should stop on the tick that touched
    for i, arena in enumerate(arenas):
      self.assertEqual(arena.tick_count, touched[i] + 1)

    before = [arena.tick_count for arena in arenas]
    with self.assertRaisesRegex(RuntimeError, "Unexpected type"):
      arenas.append(0)
      rs.Arena.multi_step(arenas, 8)
    self.assertEqual(before, [arena.tick_count for arena in arenas[:-1]])

    before = [arena.tick_count for arena in arenas[:-1]]
    with self.assertRaisesRegex(RuntimeError, "Duplicate arena detected"):
      arenas[-1] = arenas[0]
      rs.Arena.multi_step(arenas, 8)
    self.assertEqual(before, [arena.tick_count for arena in arenas[:-1]])

  def test_set_car_ball_collision(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    ball = arena.ball
    car = arena.add_car(rs.Team.BLUE)
    arena.set_car_ball_collision(False)

    touched = [False]
    def ball_touch_callback(arena: rs.Arena, car: rs.Car, data):
      data[0] = True
      arena.stop()

    arena.set_ball_touch_callback(ball_touch_callback, touched)

    for i in range(1000):
      target_chase(ball.get_state().pos, car)
      arena.step(8)

      self.assertFalse(touched[0])

      ball_pos = glm.vec3(*ball.get_state().pos.as_tuple())
      car_pos  = glm.vec3(*car.get_state().pos.as_tuple())

      if glm.distance(glm.vec2(ball_pos), glm.vec2(car_pos)) < ball.get_radius():
        break

      if abs(ball_pos.z - car_pos.z) < ball.get_radius():
        break

    self.assertFalse(touched[0])

    arena.reset_kickoff()
    arena.set_car_ball_collision(True)

    for i in range(1000):
      target_chase(ball.get_state().pos, car)
      arena.step(8)

      if touched[0]:
        break

    self.assertTrue(touched[0])

  def test_set_car_car_collision(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    car_a = arena.add_car(rs.Team.BLUE)
    car_b = arena.add_car(rs.Team.ORANGE)

    arena.set_car_ball_collision(False)
    arena.set_car_car_collision(False)

    bumped = [False]
    def car_bump_callback(arena: rs.Arena, bumper: rs.Car, victim: rs.Car, is_demo: bool, data):
      data[0] = True
      arena.stop()

    arena.set_car_bump_callback(car_bump_callback, bumped)

    for i in range(1000):
      target_chase(car_a.get_state().pos, car_b)
      target_chase(car_b.get_state().pos, car_a)
      arena.step(8)

      self.assertFalse(bumped[0])

      pos_a = glm.vec3(*car_a.get_state().pos.as_tuple())
      pos_b = glm.vec3(*car_b.get_state().pos.as_tuple())

      if glm.distance(pos_a, pos_b) < 50.0:
        break

    self.assertFalse(bumped[0])

    arena.reset_kickoff()
    arena.set_car_car_collision(True)

    for i in range(1000):
      target_chase(car_a.get_state().pos, car_b)
      target_chase(car_b.get_state().pos, car_a)
      arena.step()

      if bumped[0]:
        break

    self.assertTrue(bumped[0])

  def test_ball_prediction(self):
    for mode in (rs.GameMode.SOCCAR, rs.GameMode.HOOPS, rs.GameMode.HEATSEEKER, rs.GameMode.SNOWDAY, rs.GameMode.THE_VOID):
      arena = rs.Arena(mode)

      ball_state = arena.ball.get_state()
      ball_state.vel = rs.Vec(x=500.0, y=500.0, z=500.0)
      arena.ball.set_state(ball_state)

      pred = arena.get_ball_prediction()
      self.assertEqual(len(pred), 60)

      pred = arena.get_ball_prediction(num_ticks=240, tick_skip=0)
      self.assertEqual(len(pred), 240)

  def test_clone(self):
    for mode in (rs.GameMode.SOCCAR, rs.GameMode.HOOPS, rs.GameMode.HEATSEEKER, rs.GameMode.SNOWDAY, rs.GameMode.THE_VOID):
      arena = rs.Arena(mode)

      for i in range(4):
        random_car(arena, rs.Team.BLUE)
        random_car(arena, rs.Team.ORANGE)

      # state setting for the void gets out of hand quickly
      for i in range(10 if mode == rs.GameMode.THE_VOID else 1000):
        for car in arena.get_cars():
          target_chase(arena.ball.get_state().pos, car)
        arena.step(7)

      self.compare(arena, arena.clone())

  def test_clone_into(self):
    for mode in (rs.GameMode.SOCCAR, rs.GameMode.HOOPS, rs.GameMode.HEATSEEKER, rs.GameMode.SNOWDAY, rs.GameMode.THE_VOID):
      arena = rs.Arena(mode)

      for i in range(4):
        random_car(arena, rs.Team.BLUE)
        random_car(arena, rs.Team.ORANGE)

      clone = arena.clone()

      # state setting for the void gets out of hand quickly
      for i in range(10 if mode == rs.GameMode.THE_VOID else 1000):
        for car in arena.get_cars():
          target_chase(arena.ball.get_state().pos, car)
        arena.step(7)

      arena.clone_into(clone)
      self.compare(arena, clone)

  def test_pickle(self):
    for mode in (rs.GameMode.SOCCAR, rs.GameMode.HOOPS, rs.GameMode.HEATSEEKER, rs.GameMode.SNOWDAY, rs.GameMode.THE_VOID):
      arena = rs.Arena(mode)
      for i in range(4):
        random_car(arena, rs.Team.BLUE)
        random_car(arena, rs.Team.ORANGE)

      # state setting for the void gets out of hand quickly
      for i in range(10 if mode == rs.GameMode.THE_VOID else 1000):
        for car in arena.get_cars():
          target_chase(arena.ball.get_state().pos, car)
        arena.step(7)

      self.compare(arena, pickled(arena))

  def test_copy(self):
    for mode in (rs.GameMode.SOCCAR, rs.GameMode.HOOPS, rs.GameMode.HEATSEEKER, rs.GameMode.SNOWDAY, rs.GameMode.THE_VOID):
      arena = rs.Arena(mode)
      for i in range(4):
        random_car(arena, rs.Team.BLUE)
        random_car(arena, rs.Team.ORANGE)

      # state setting for the void gets out of hand quickly
      for i in range(10 if mode == rs.GameMode.THE_VOID else 1000):
        for car in arena.get_cars():
          target_chase(arena.ball.get_state().pos, car)
        arena.step(7)

      self.compare(arena, copy.copy(arena))

  def test_deep_copy(self):
    for mode in (rs.GameMode.SOCCAR, rs.GameMode.HOOPS, rs.GameMode.HEATSEEKER, rs.GameMode.SNOWDAY, rs.GameMode.THE_VOID):
      arena = rs.Arena(mode)
      for i in range(4):
        random_car(arena, rs.Team.BLUE)
        random_car(arena, rs.Team.ORANGE)

      # state setting for the void gets out of hand quickly
      for i in range(10 if mode == rs.GameMode.THE_VOID else 1000):
        for car in arena.get_cars():
          target_chase(arena.ball.get_state().pos, car)
        arena.step(7)

      self.compare(arena, copy.deepcopy(arena))

  def test_get_car_from_id(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    with self.assertRaises(KeyError):
      arena.get_car_from_id(0)

    self.assertIs(arena.get_car_from_id(0, None), None)

    tmp = object()
    self.assertIs(arena.get_car_from_id(0, tmp), tmp)

    cars = {}
    for i in range(10):
      car = random_car(arena)
      self.assertNotIn(car.id, cars)
      cars[car.id] = car

    for car_id, car in cars.items():
      self.assertEqual(car_id, car.id)
      self.assertIs(car, arena.get_car_from_id(car_id))

  def test_add_car(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    car = arena.add_car(rs.Team.BLUE)
    self.assertEqual(car.team, rs.Team.BLUE)

    car2 = arena.add_car(rs.Team.ORANGE);
    self.assertEqual(car2.team, rs.Team.ORANGE)
    self.assertIsNot(car, car2)
    self.assertNotEqual(car.id, car2.id)

  def test_get_cars(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    cars = {}
    for j in range(2):
      for i in range(5):
        car = random_car(arena)
        self.assertNotIn(car.id, cars)
        cars[car.id] = car

      for car in arena.get_cars():
        self.assertIn(car.id, cars)
        self.assertIs(cars[car.id], car)

  def test_get_mutator_config(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    arena.get_mutator_config()

  def test_remove_car(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    cars = {}
    for i in range(5):
      car = random_car(arena)
      self.assertNotIn(car.id, cars)
      cars[car.id] = car

    arena_cars = arena.get_cars()
    self.assertEqual(len(cars), len(arena_cars))
    for car in arena_cars:
      self.assertIn(car.id, cars)
      self.assertIs(cars[car.id], car)

    while len(arena_cars) > 0:
      with self.subTest(i=len(arena_cars)):
        car = arena_cars[0]
        car_id = car.id
        arena.remove_car(car)
        with self.assertRaises(RuntimeError):
          arena.remove_car(car)
        with self.assertRaises(KeyError):
          arena.get_car_from_id(car_id)
      arena_cars = arena.get_cars()

  def test_get_ball_rot(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    # ensure it doesn't crash/output only 0
    ball_rot = arena.ball.get_rot()
    self.assertNotEqual(sum(ball_rot), 0)

  def test_shot_event(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    car = arena.add_car(rs.Team.BLUE)

    ball_state = arena.ball.get_state()
    ball_state.pos = rs.Vec(0.0, 4000.0, ball_state.pos.z)
    arena.ball.set_state(ball_state)

    car_state = car.get_state()
    car_state.pos     = rs.Vec(0.0, 3000.0, car_state.pos.z)
    car_state.vel     = rs.Vec(0.0, 1000.0, 0.0)
    car_state.ang_vel = rs.Vec(0.0, 0.0, 0.0)

    car_state.rot_mat.forward = rs.Vec( 0.0, 1.0, 0.0)
    car_state.rot_mat.right   = rs.Vec(-1.0, 0.0, 0.0)
    car_state.rot_mat.up      = rs.Vec( 0.0, 0.0, 1.0)

    car.set_state(car_state)

    shot = [False]
    def handle_shot_event(arena: rs.Arena, shooter: rs.Car, passer: rs.Car, data):
      data.assertIs(shooter, car)
      data.assertIsNone(passer)
      shot[0] = True
      arena.stop()

    arena.set_shot_event_callback(handle_shot_event, self)

    for i in range(1000):
      target_chase(arena.ball.get_state().pos, car)
      arena.step(1)

      if shot[0]:
        break

    self.assertTrue(shot[0])
    self.assertEqual(car.shots, 1)

  def test_save_event(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    car1 = arena.add_car(rs.Team.BLUE)
    car2 = arena.add_car(rs.Team.ORANGE)

    ball_state = arena.ball.get_state()
    ball_state.pos = rs.Vec(0.0, 4000.0, ball_state.pos.z)
    arena.ball.set_state(ball_state)

    car_state = car1.get_state()
    car_state.pos     = rs.Vec(0.0, 3000.0, car_state.pos.z)
    car_state.vel     = rs.Vec(0.0, 1000.0, 0.0)
    car_state.ang_vel = rs.Vec(0.0, 0.0, 0.0)

    car_state.rot_mat.forward = rs.Vec( 0.0, 1.0, 0.0)
    car_state.rot_mat.right   = rs.Vec(-1.0, 0.0, 0.0)
    car_state.rot_mat.up      = rs.Vec( 0.0, 0.0, 1.0)

    car1.set_state(car_state)

    car_state = car2.get_state()
    car_state.pos     = rs.Vec(0.0, 5000.0, car_state.pos.z)
    car_state.vel     = rs.Vec(0.0, 0.0, 0.0)
    car_state.ang_vel = rs.Vec(0.0, 0.0, 0.0)

    car_state.rot_mat.forward = rs.Vec(0.0, -1.0, 0.0)
    car_state.rot_mat.right   = rs.Vec(1.0,  0.0, 0.0)
    car_state.rot_mat.up      = rs.Vec(0.0,  0.0, 1.0)

    car2.set_state(car_state)
    car2.set_controls(rs.CarControls(jump=True))

    saved = [False]
    def handle_save_event(arena: rs.Arena, saver: rs.Car, data):
      data.assertIs(saver, car2)
      saved[0] = True
      arena.stop()

    arena.set_save_event_callback(handle_save_event, self)

    for i in range(1000):
      target_chase(arena.ball.get_state().pos, car1)
      arena.step(1)

      if saved[0]:
        break

    self.assertTrue(saved[0])
    self.assertEqual(car1.shots, 1)
    self.assertEqual(car2.saves, 1)

  def test_goal_event(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    car1 = arena.add_car(rs.Team.BLUE)
    car2 = arena.add_car(rs.Team.BLUE)

    ball_state = arena.ball.get_state()
    ball_state.pos = rs.Vec(0.0, 4000.0, ball_state.pos.z)
    arena.ball.set_state(ball_state)

    car_state = car1.get_state()
    car_state.pos     = rs.Vec(0.0, 3800.0, car_state.pos.z)
    car_state.vel     = rs.Vec(0.0, 0.0, 0.0)
    car_state.ang_vel = rs.Vec(0.0, 0.0, 0.0)

    car_state.rot_mat.forward = rs.Vec( 0.0, 1.0, 0.0)
    car_state.rot_mat.right   = rs.Vec(-1.0, 0.0, 0.0)
    car_state.rot_mat.up      = rs.Vec( 0.0, 0.0, 1.0)

    car1.set_state(car_state)

    car_state = car2.get_state()
    car_state.pos     = rs.Vec(-100.0, 5120.0, car_state.pos.z)
    car_state.vel     = rs.Vec(0.0, 0.0, 0.0)
    car_state.ang_vel = rs.Vec(0.0, 0.0, 0.0)

    car_state.rot_mat.forward = rs.Vec(1.0, 0.0, 0.0)
    car_state.rot_mat.right   = rs.Vec(0.0, 1.0, 0.0)
    car_state.rot_mat.up      = rs.Vec(0.0, 0.0, 1.0)

    car2.set_state(car_state)

    goal = [False]
    def handle_goal_event(arena: rs.Arena, shooter: rs.Car, passer: rs.Car, data):
      data.assertIs(shooter, car2)
      data.assertIs(passer, car1)
      goal[0] = True
      arena.stop()

    arena.set_goal_event_callback(handle_goal_event, self)

    for i in range(1000):
      target_chase(arena.ball.get_state().pos, car1)

      if i > 20:
        car2.set_controls(rs.CarControls(jump=True))

      arena.step(1)

      if goal[0]:
        break

    self.assertTrue(goal[0])
    self.assertEqual(car1.assists, 1)
    self.assertEqual(car2.goals, 1)
    self.assertEqual(arena.blue_score, 1)

  def test_get_gym_state(self):
    def load_mat3(mat: np.ndarray) -> glm.mat3:
      return glm.mat3(*mat)

    def invert_vector(vec):
      if type(vec) is np.ndarray:
        return np.array(inv_mtx * glm.vec3(*vec))
      return inv_mtx * vec

    def invert_mat3(mat: glm.mat3) -> np.ndarray:
      return np.array(inv_mtx * mat)

    def invert_quat(quat: np.ndarray) -> np.ndarray:
      return np.array(glm.angleAxis(math.pi, z) * glm.quat(*quat))

    def compare_quat(q1: np.ndarray, q2: np.ndarray) -> bool:
      t1 = np.amax(np.absolute(q1 + q2))
      t2 = np.amax(np.absolute(q1 - q2))
      if min(t1, t2) >= 1e-6:
        raise self.failureException(f"\n{q1}\n\t!=\n{q2}")

    def pyr_to_mat3(pyr: np.ndarray) -> np.ndarray:
      # what
      pitch, yaw, roll = pyr
      q = glm.normalize(glm.angleAxis(yaw, z))
      q = q * glm.normalize(glm.angleAxis(pitch, y))
      q = q * glm.normalize(glm.angleAxis(roll, -x))
      return np.array(glm.mat3_cast(q))

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

    for mode in (rs.GameMode.SOCCAR, rs.GameMode.HOOPS, rs.GameMode.HEATSEEKER, rs.GameMode.SNOWDAY, rs.GameMode.THE_VOID):
      arena = rs.Arena(mode)
      car1  = arena.add_car(rs.Team.BLUE, rs.CarConfig.DOMINUS)
      car2  = arena.add_car(rs.Team.ORANGE, rs.CarConfig(rs.CarConfig.DOMINUS))

      for i in range(100):
        for j in range(10):
          random_car(arena)
        while len(arena.get_cars()) > 0:
          arena.remove_car(arena.get_cars()[0])

      for i in range(100):
        for j in range(10):
          random_car(arena)
        for car in arena.get_cars():
          arena.remove_car(car)

      x = glm.vec3(1, 0, 0)
      y = glm.vec3(0, 1, 0)
      z = glm.vec3(0, 0, 1)

      arena = rs.Arena(rs.GameMode.SOCCAR)
      car1  = arena.add_car(rs.Team.BLUE)
      car2  = arena.add_car(rs.Team.ORANGE)
      ball  = arena.ball

      arena.reset_kickoff()

      self.assertNotEqual(car1.id, car2.id)
      self.assertEqual(car1.team, rs.Team.BLUE)
      self.assertEqual(car2.team, rs.Team.ORANGE)

      inv_mtx = glm.mat3(-1,  0,  0,
                          0, -1,  0,
                          0,  0,  1)

      for i in range(10000):
        with self.subTest(i=i):
          target_chase(ball.get_state().pos, car1)
          target_chase(ball.get_state().pos, car2)

          prev_tick = arena.tick_count
          arena.step(4)
          state = arena.get_gym_state()

          ball_state = ball.get_state()

          self.assertEqual(state[0][0], arena.game_mode)
          if state[0][1]: # possibly multiple cars hit ball on same tick
            car      = arena.get_car_from_id(int(state[0][1]))
            hit_info = car.get_state().ball_hit_info
            self.assertTrue(hit_info.is_valid)

            for c in arena.get_cars():
              info = c.get_state().ball_hit_info
              if c is car or not info.is_valid:
                continue
              self.assertGreaterEqual(hit_info.tick_count_when_hit, info.tick_count_when_hit)
          self.assertEqual(state[0][2], arena.blue_score)
          self.assertEqual(state[0][3], arena.orange_score)

          # validate last ball-car hit
          if int(state[0][1]):
            last_hit = arena.get_car_from_id(int(state[0][1])).get_state().ball_hit_info
            self.assertTrue(last_hit.is_valid)

            for car in arena.get_cars():
              car_hit = car.get_state().ball_hit_info
              if not car_hit.is_valid:
                continue
              self.assertLessEqual(car_hit.tick_count_when_hit, last_hit.tick_count_when_hit)
          else:
            for car in arena.get_cars():
              car_hit = car.get_state().ball_hit_info
              if not car_hit.is_valid:
                continue
              self.assertLessEqual(car_hit.tick_count_when_hit, prev_tick)

          gym_state = state[2][0]
          self.assertEqual(gym_state[0:3], ball_state.pos.as_numpy())
          #self.assertEqual(gym_state[3:7], ball_state.quat.as_numpy())
          self.assertEqual(gym_state[7:10], ball_state.vel.as_numpy())
          self.assertEqual(gym_state[10:13], ball_state.ang_vel.as_numpy())
          self.assertEqual(gym_state[13:22].reshape(3, 3), ball_state.rot_mat.as_numpy())
          #self.assertEqual(gym_state[22:25], ball_state.pyr.as_numpy())

          gym_state = state[2][1]
          self.assertEqual(gym_state[0:3], invert_vector(ball_state.pos.as_numpy()))
          #self.assertEqual(gym_state[3:7], ball_state.quat.as_numpy())
          self.assertEqual(gym_state[7:10], invert_vector(ball_state.vel.as_numpy()))
          self.assertEqual(gym_state[10:13], invert_vector(ball_state.ang_vel.as_numpy()))
          self.assertEqual(invert_mat3(load_mat3(gym_state[13:22])).transpose(), ball_state.rot_mat.as_numpy())
          #self.assertEqual(gym_state[22:25], ball_state.pyr.as_numpy())

          cars = arena.get_cars()
          self.assertEqual(len(cars), len(state[3:]))

          # ensure each gym state car is in the arena
          for s in state[3:]:
            self.assertIsNotNone(arena.get_car_from_id(int(s[0][0]), None))

          # ensure each arena car is in the gym state
          for car in arena.get_cars():
            self.assertTrue(car.id in [x[0][0] for x in state[3:]])

          for i in range(3, len(state)):
            gym_state = state[i]
            car       = arena.get_car_from_id(int(gym_state[0][0]))
            car_state = car.get_state()

            for j in range(2):
              self.assertEqual(car.id, gym_state[j][0])
              self.assertEqual(car.team, gym_state[j][1])
              self.assertEqual(car.goals, gym_state[j][2])
              self.assertEqual(car.saves, gym_state[j][3])
              self.assertEqual(car.shots, gym_state[j][4])
              self.assertEqual(car.demos, gym_state[j][5])
              self.assertEqual(car.boost_pickups, gym_state[j][6])
              self.assertEqual(car_state.is_demoed, gym_state[j][7])
              self.assertEqual(car_state.is_on_ground, gym_state[j][8])
              if car_state.ball_hit_info.is_valid:
                self.assertEqual(car_state.ball_hit_info.tick_count_when_hit >= prev_tick, gym_state[j][9])
              else:
                self.assertFalse(gym_state[j][9])
              self.assertEqual(car_state.boost, gym_state[j][10])

            car_dir_x = glm.vec3(car_state.rot_mat.forward.as_numpy())
            car_dir_y = glm.vec3(car_state.rot_mat.right.as_numpy())
            car_dir_z = glm.vec3(car_state.rot_mat.up.as_numpy())

            gym_state = state[i][0]
            self.assertEqual(gym_state[11:14], car_state.pos.as_numpy())
            self.assertEqual(gym_state[18:21], car_state.vel.as_numpy())
            self.assertEqual(gym_state[21:24], car_state.ang_vel.as_numpy())

            if not car_state.is_demoed:
              self.assertEqual(gym_state[24:33].reshape(3, 3), car_state.rot_mat.as_numpy())

              m   = load_mat3(gym_state[24:33])
              q   = glm.quat_cast(m)
              pyr = gym_state[33:36]
              compare_quat(gym_state[14:18], np.array(q))
              self.assertAlmostEqual(np.array(m), pyr_to_mat3(pyr), 4)
              self.assertTrue(check_pyr(pyr))

              self.assertAlmostEqual(glm.dot(car_dir_x, m * x), 1.0, 5)
              self.assertAlmostEqual(glm.dot(car_dir_y, m * y), 1.0, 5)
              self.assertAlmostEqual(glm.dot(car_dir_z, m * z), 1.0, 5)
              self.assertAlmostEqual(glm.dot(car_dir_x, q * x), 1.0, 5)
              self.assertAlmostEqual(glm.dot(car_dir_y, q * y), 1.0, 5)
              self.assertAlmostEqual(glm.dot(car_dir_z, q * z), 1.0, 5)
              self.assertAlmostEqual(np.array(car_dir_x), np.array(m * x), 5)
              self.assertAlmostEqual(np.array(car_dir_y), np.array(m * y), 5)
              self.assertAlmostEqual(np.array(car_dir_z), np.array(m * z), 5)
              self.assertAlmostEqual(np.array(car_dir_x), np.array(q * x), 5)
              self.assertAlmostEqual(np.array(car_dir_y), np.array(q * y), 5)
              self.assertAlmostEqual(np.array(car_dir_z), np.array(q * z), 5)

            # check inversion
            gym_state = state[i][1]
            self.assertEqual(gym_state[11:14], invert_vector(car_state.pos.as_numpy()))
            self.assertEqual(gym_state[18:21], invert_vector(car_state.vel.as_numpy()))
            self.assertEqual(gym_state[21:24], invert_vector(car_state.ang_vel.as_numpy()))

            if car_state.is_demoed:
              continue

            compare_quat(gym_state[14:18], invert_quat(q))
            self.assertEqual(np.array(load_mat3(gym_state[24:33])), invert_mat3(m))

            m = load_mat3(gym_state[24:33])
            q = glm.quat_cast(m)
            pyr = gym_state[33:36]
            compare_quat(gym_state[14:18], np.array(q))
            self.assertAlmostEqual(np.array(m), pyr_to_mat3(pyr), 4)
            self.assertTrue(check_pyr(pyr))

            self.assertAlmostEqual(glm.dot(invert_vector(car_dir_x), m * x), 1.0, 5)
            self.assertAlmostEqual(glm.dot(invert_vector(car_dir_y), m * y), 1.0, 5)
            self.assertAlmostEqual(glm.dot(invert_vector(car_dir_z), m * z), 1.0, 5)
            self.assertAlmostEqual(glm.dot(invert_vector(car_dir_x), q * x), 1.0, 5)
            self.assertAlmostEqual(glm.dot(invert_vector(car_dir_y), q * y), 1.0, 5)
            self.assertAlmostEqual(glm.dot(invert_vector(car_dir_z), q * z), 1.0, 5)
            self.assertAlmostEqual(invert_vector(np.array(car_dir_x)), np.array(m * x), 5)
            self.assertAlmostEqual(invert_vector(np.array(car_dir_y)), np.array(m * y), 5)
            self.assertAlmostEqual(invert_vector(np.array(car_dir_z)), np.array(m * z), 5)
            self.assertAlmostEqual(invert_vector(np.array(car_dir_x)), np.array(q * x), 5)
            self.assertAlmostEqual(invert_vector(np.array(car_dir_y)), np.array(q * y), 5)
            self.assertAlmostEqual(invert_vector(np.array(car_dir_z)), np.array(q * z), 5)

      if mode != rs.GameMode.THE_VOID:
        # we definitely should have hit the ball at least once with our actor
        self.assertNotEqual(state[0][1], 0)

if __name__ == "__main__":
  unittest.main()
