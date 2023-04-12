![image](https://user-images.githubusercontent.com/36944229/219303954-7267bce1-b7c5-4f15-881c-b9545512e65b.png)

**A C++ library for simulating Rocket League games at maximum efficiency**

## Installation
- Clone this repo and build it
- Use https://github.com/ZealanL/RLArenaCollisionDumper to dump all of Rocket League's arena collision meshes
- Move those assets into RocketSim's executing directory

## Documentation
Documentation is available at: https://zealanl.github.io/RocketSimDocs/

## Progress
**Coming Soon:**
- More collision optimizations
- Proper documentation
- Improved collision accuracy

**Done:**
- Car suspension
- Car driving
- Car jumps and flips
- Arena collision
- Proper ball bounces
- Car-ball collision with proper forces (will be refined more in the near future)
- Boost usage and boost pads
- Bumps and demos
- Auto-flip when upside-down
- Serialization of cars/ball/boost pads/arena
- Boost pad/suspension ray optimization using lookup grid

## Bindings
If you don't want to work in C++, here are some (unofficial) bindings written in other languages:
- **Rust**: https://github.com/VirxEC/rocketsim-rs by `VirxEC`
- **Python**: `pip3 install git+https://github.com/mtheall/RocketSim@python-dev`

## Performance
RocketSim already heavily outperforms the speed of Rocket League's physics tick step without optimization.

Version performance comparison:
```
OS: Windows 10 (Process Priority = Normal)
CPU: Intel i5-11400 @ 2.60GHz
Ram Speed: 3200MZ
Compiler: MSVC 14.16
=================================
Arena: Default (Soccar)
Cars: 2 on each team (2v2)
Inputs: Randomly pregenerated, changed every 2-60 ticks for each car
=================================
Single-thread performance (calculated using average CPU cycles per tick on the RocketSim thread) (1M ticks simulated):
v1.0.0 = 30,334tps
```

## Simulation Accuracy
RocketSim is not perfectly accurate, but it's close enough that it shouldnt matter (for ML bots or humans).
Bots that work well in RocketSim will work well in the actual game, and visa-versa.

## Example Usage
```python
#!/usr/bin/env python3

import RocketSim as rs

# Make an arena instance (this is where our simulation takes place, has its own btDynamicsWorld instance)
arena = rs.Arena(rs.GameMode.SOCCAR)

# Make a new car
car = arena.add_car(rs.Team.BLUE)

# Set up an initial state for our car
car.set_state(rs.CarState(pos=rs.Vec(z=17), vel=rs.Vec(x=50)))

# Setup a ball state
arena.ball.set_state(rs.BallState(pos=rs.Vec(y=400, z=100)))

# Make our car drive forward and turn
car.set_controls(rs.CarControls(throttle=1, steer=1))

# Simulate for 100 ticks
arena.step(100)

# Lets see where our car went!
print(f"After {arena.tick_count} ticks, our car is at: {car.get_state().pos:.2f}")
```

## Issues & PRs
Feel free to make issues and pull requests if you encounter any issues!

You can also contact me on Discord if you have questions: `mtheall#6174`

## Legal Notice
RocketSim was written to replicate Rocket League's game logic, but does not actually contain any code from the game.
To Epic Games/Psyonix: If any of you guys have an issue with this, let me know on Discord and we can resolve it.
