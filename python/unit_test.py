#!/usr/bin/env python3

import RocketSim as rs

import glm
import math
import numpy as np
import random
import unittest
import multiprocessing

np.set_printoptions(formatter={"float": lambda x: f"{x: .6f}"}, linewidth=100)

def return_self(args):
  return args

class FuzzyTestCase(unittest.TestCase):
  def assertEqual(self, a, b, threshold = 1e-4):
    if type(a) is float and type(b) is float:
      error = abs(b - a)
      if error >= threshold:
        unittest.TestCase.assertEqual(self, a, b)
    elif type(a) is np.ndarray and type(b) is np.ndarray:
      error = np.amax(np.absolute(a - b))
      if error >= threshold:
        unittest.TestCase.assertEqual(self, a, b)
    else:
      unittest.TestCase.assertEqual(self, a, b)

class TestGameMode(FuzzyTestCase):
  def test_values(self):
    self.assertEqual(rs.GameMode.SOCCAR, 0)
    self.assertEqual(rs.GameMode.THE_VOID, 1)

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
    self.assertEqual(vec.x, x)
    self.assertEqual(vec.y, y)
    self.assertEqual(vec.z, z)
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

  def test_pickle(self):
    vec = rs.Vec(
      x = random.uniform(-1.0, 1.0),
      y = random.uniform(-1.0, 1.0),
      z = random.uniform(-1.0, 1.0)
    )

    with multiprocessing.Pool(1) as p:
      result = p.map(return_self, [vec])
      self.assertEqual(vec.x, result[0].x)
      self.assertEqual(vec.y, result[0].y)
      self.assertEqual(vec.z, result[0].z)

class TestRotMat(FuzzyTestCase):
  def compare(self, mat, forward, right, up):
    self.assertEqual(mat.forward, forward)
    self.assertEqual(mat.right, right)
    self.assertEqual(mat.up, up)

  def test_basic(self):
    self.compare(rs.RotMat(),
      rs.Vec(),
      rs.Vec(),
      rs.Vec())
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
    forward = glm.normalize(glm.vec3(random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0)))
    right   = glm.normalize(glm.vec3(random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0)))
    up      = glm.normalize(glm.cross(forward, right))
    right   = glm.normalize(glm.cross(up, forward))

    src = rs.RotMat(
      forward = rs.Vec(forward.x, forward.y, forward.z),
      right   = rs.Vec(right.x,   right.y,   right.z),
      up      = rs.Vec(up.x,      up.y,      up.z)
    )

    dst = src.as_angle().as_rot_mat()

    self.assertEqual(src.forward.x, dst.forward.x)
    self.assertEqual(src.forward.y, dst.forward.y)
    self.assertEqual(src.forward.z, dst.forward.z)
    self.assertEqual(src.right.x,   dst.right.x)
    self.assertEqual(src.right.y,   dst.right.y)
    self.assertEqual(src.right.z,   dst.right.z)
    self.assertEqual(src.up.x,      dst.up.x)
    self.assertEqual(src.up.y,      dst.up.y)
    self.assertEqual(src.up.z,      dst.up.z)
 
  def test_pickle(self):
    mat = rs.RotMat(
      forward = rs.Vec(random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0)),
      right   = rs.Vec(random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0)),
      up      = rs.Vec(random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0))
    )

    with multiprocessing.Pool(1) as p:
      result = p.map(return_self, [mat])
      self.assertEqual(mat.forward, result[0].forward)
      self.assertEqual(mat.right,   result[0].right)
      self.assertEqual(mat.up,      result[0].up)

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
    src = rs.Angle(
      yaw   = random.uniform(-1.0, 1.0),
      pitch = random.uniform(-1.0, 1.0),
      roll  = random.uniform(-1.0, 1.0)
    )

    dst = src.as_rot_mat().as_angle()

    self.assertEqual(src.yaw,   dst.yaw)
    self.assertEqual(src.pitch, dst.pitch)
    self.assertEqual(src.roll,  dst.roll)

  def test_pickle(self):
    angle = rs.Angle(
      yaw   = random.uniform(-1.0, 1.0),
      pitch = random.uniform(-1.0, 1.0),
      roll  = random.uniform(-1.0, 1.0)
    )

    with multiprocessing.Pool(1) as p:
      result = p.map(return_self, [angle])
      self.assertEqual(angle.yaw,   result[0].yaw)
      self.assertEqual(angle.pitch, result[0].pitch)
      self.assertEqual(angle.roll,  result[0].roll)

class TestBallHitInfo(FuzzyTestCase):
  def test_basic(self):
    pass

class TestBallState(FuzzyTestCase):
  def compare(self, state, pos, vel, ang_vel, car_id):
    self.assertEqual(state.pos, pos)
    self.assertEqual(state.vel, vel)
    self.assertEqual(state.ang_vel, ang_vel)
    self.assertEqual(state.last_hit_car_id, car_id)

  def test_basic(self):
    self.compare(rs.BallState(), rs.Vec(0, 0, 93.15), rs.Vec(), rs.Vec(), 0)
    self.compare(rs.BallState(rs.Vec()), rs.Vec(), rs.Vec(), rs.Vec(), 0)
    self.compare(rs.BallState(vel=rs.Vec(1, 2, 3), last_hit_car_id=10), rs.Vec(0, 0, 93.15), rs.Vec(1, 2, 3), rs.Vec(), 10)

class TestBall(FuzzyTestCase):
  def test_create(self):
    with self.assertRaises(TypeError):
      rs.Ball()

class TestBoostPadState(FuzzyTestCase):
  def test_basic(self):
    pass

class TestBoostPad(FuzzyTestCase):
  def test_create(self):
    with self.assertRaises(TypeError):
      rs.BoostPad()

class TestWheelPairConfig(FuzzyTestCase):
  def test_basic(self):
    pass

class TestCarConfig(FuzzyTestCase):
  def test_values(self):
    self.assertEqual(rs.CarConfig.OCTANE, 0)
    self.assertEqual(rs.CarConfig.DOMINUS, 1)
    self.assertEqual(rs.CarConfig.PLANK, 2)
    self.assertEqual(rs.CarConfig.BREAKOUT, 3)
    self.assertEqual(rs.CarConfig.HYBRID, 4)
    self.assertEqual(rs.CarConfig.MERC, 5)

  def test_create(self):
    self.assertEqual(rs.CarConfig().dodge_deadzone, 0.5)
    self.assertEqual(rs.CarConfig().hitbox_size.x, 120.5070)

    self.assertEqual(rs.CarConfig(rs.CarConfig.OCTANE).hitbox_size.x, 120.5070)
    self.assertEqual(rs.CarConfig(rs.CarConfig.DOMINUS).hitbox_size.x, 130.4270)
    self.assertEqual(rs.CarConfig(rs.CarConfig.PLANK).hitbox_size.x, 131.3200)
    self.assertEqual(rs.CarConfig(rs.CarConfig.BREAKOUT).hitbox_size.x, 133.9920)
    self.assertEqual(rs.CarConfig(rs.CarConfig.HYBRID).hitbox_size.x, 129.5190)
    self.assertEqual(rs.CarConfig(rs.CarConfig.MERC).hitbox_size.x, 123.22)

class TestCarControls(FuzzyTestCase):
  def compare(self, controls, attrs):
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
    self.compare(rs.CarControls(), {})
    self.compare(rs.CarControls(throttle=1.0), {"throttle": 1.0})
    self.compare(rs.CarControls(steer=2.0), {"steer": 2.0})
    self.compare(rs.CarControls(pitch=3.0), {"pitch": 3.0})
    self.compare(rs.CarControls(yaw=4.0), {"yaw": 4.0})
    self.compare(rs.CarControls(roll=5.0), {"roll": 5.0})
    self.compare(rs.CarControls(boost=True), {"boost": True})
    self.compare(rs.CarControls(jump=True), {"jump": True})
    self.compare(rs.CarControls(handbrake=True), {"handbrake": True})
    self.compare(rs.CarControls(jump=True, steer=2.0), {"steer": 2.0, "jump": True})
    self.compare(rs.CarControls(0.125, 0.25, 0.5, boost=True),
      {
        "throttle": 0.125,
        "steer": 0.25,
        "pitch": 0.5,
        "boost": True,
      })

  def test_pickle(self):
    controls = rs.CarControls(
      throttle  = random.uniform(-1.0, 1.0),
      steer     = random.uniform(-1.0, 1.0),
      yaw       = random.uniform(-1.0, 1.0),
      pitch     = random.uniform(-1.0, 1.0),
      roll      = random.uniform(-1.0, 1.0),
      boost     = random.randrange(0, 1) == 1,
      jump      = random.randrange(0, 1) == 1,
      handbrake = random.randrange(0, 1) == 1
    )

    with multiprocessing.Pool(1) as p:
      result = p.map(return_self, [controls])
      self.assertEqual(controls.throttle,  result[0].throttle)
      self.assertEqual(controls.steer,     result[0].steer)
      self.assertEqual(controls.yaw,       result[0].yaw)
      self.assertEqual(controls.pitch,     result[0].pitch)
      self.assertEqual(controls.roll,      result[0].roll)
      self.assertEqual(controls.boost,     result[0].boost)
      self.assertEqual(controls.jump,      result[0].jump)
      self.assertEqual(controls.handbrake, result[0].handbrake)

class TestCarState(FuzzyTestCase):
  def test_basic(self):
    pass

class TestCar(FuzzyTestCase):
  def test_create(self):
    with self.assertRaises(TypeError):
      rs.Car()

class TestMutatorConfig(FuzzyTestCase):
  def test_basic(self):
    pass

class TestArena(FuzzyTestCase):
  def test_basic(self):
    pass

  def test_get_car_from_id(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    with self.assertRaises(KeyError):
      arena.get_car_from_id(0)

    self.assertIs(arena.get_car_from_id(0, None), None)

    tmp = object()
    self.assertIs(arena.get_car_from_id(0, tmp), tmp)

    cars = {}
    for i in range(10):
      car = arena.add_car(random.randint(0, 1), random.randint(0, 5))
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
        car = arena.add_car(random.randint(0, 1), random.randint(0, 5))
        self.assertNotIn(car.id, cars)
        cars[car.id] = car

      for car in arena.get_cars():
        self.assertIn(car.id, cars)
        self.assertIs(cars[car.id], car)

  def test_get_mutator_config(self):
    pass

  def test_remove_car(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    cars = {}
    for i in range(5):
      car = arena.add_car(random.randint(0, 1), random.randint(0, 5))
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

  def test_get_gym_state(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)
    arena.get_mutator_config()

    arena = rs.Arena(rs.GameMode.SOCCAR)
    car1  = arena.add_car(rs.Team.BLUE, rs.CarConfig.DOMINUS)
    car2  = arena.add_car(rs.Team.ORANGE, rs.CarConfig(rs.CarConfig.DOMINUS))

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

    arena = rs.Arena(rs.GameMode.SOCCAR)
    car1  = arena.add_car(rs.Team.BLUE)
    car2  = arena.add_car(rs.Team.ORANGE)
    ball  = arena.ball

    arena.reset_kickoff()

    controls = rs.CarControls()
    controls.throttle = 1.0
    controls.steer    = 0.3

    car1.set_controls(controls)
    car2.set_controls(controls)

    self.assertNotEqual(car1.id, car2.id)
    self.assertEqual(car1.team, rs.Team.BLUE)
    self.assertEqual(car2.team, rs.Team.ORANGE)

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
      with self.subTest(i=i):
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

if __name__ == "__main__":
  unittest.main()
