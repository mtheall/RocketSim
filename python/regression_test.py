#!/usr/bin/env python3

import RocketSim as rs

import unittest

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
    def handle_demo(arena, car, victim, is_demo, data):
      if not is_demo:
        return

      key = (arena.tick_count, car.id, victim.id)
      self.assertNotIn(key, demos)
      demos.add(key)

    arena.set_car_bump_callback(handle_demo)

    arena.step(15)
    self.assertEqual(len(demos), 1)
    self.assertEqual({(9, 2, 1)}, demos)

  def test_continuous_boost_pickup(self):
    arena = rs.Arena(rs.GameMode.SOCCAR)

    pickups = [0]
    def handle_boost(arena, car, pad, data):
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

if __name__ == "__main__":
  unittest.main()
