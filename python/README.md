![image](https://user-images.githubusercontent.com/36944229/219303954-7267bce1-b7c5-4f15-881c-b9545512e65b.png)

**A C++ library for simulating Rocket League games at maximum efficiency**

## Progress
**Coming Soon:**
- Bumps and demos
- Auto-flip when upside-down
- Significant collision optimizations
- Decent half-percision 60tps simulation

**Done:**
- Car suspension
- Car driving
- Car jumps and flips
- Arena collision
- Proper ball bounces
- Car-ball collision with proper forces (will be refined more in the near future)
- Boost usage and boost pads

## Simulation Accuracy
RocketSim is not perfectly accurate, but it's close enough that it shouldnt matter.
Bots that work well in RocketSim will work well in the actual game, and visa-versa.

## Example Usage
```python
import RocketSim

# Make an arena instance (this is where our simulation takes place, has its own btDynamicsWorld instance)
arena = RocketSim.Arena(RocketSim.SOCCAR)

# Make a new car
car = arena.add_car(RocketSim.BLUE)

# Set up an initial state for our car
car_state = RocketSim.CarState()
car_state.pos = RocketSim.Vec(0, 0, 17)
car_state.vel = RocketSim.Vec(50, 0, 0)
car.set_state(car_state)

# Setup a ball state
ball_state = RocketSim.BallState()
ball_state.pos = RocketSim.Vec (0, 400, 100)
arena.ball.set_state(ball_state);

# Make our car drive forward and turn
car_controls = RocketSim.CarControls()
car_controls.throttle = 1
car_controls.steer = 1
car.set_controls(car_controls)

# Simulate for 100 ticks
arena.step(100);

# Lets see where our car went!
print(f"After {arena.tick_count} ticks, our car is at: {car.get_state().pos:.2f}")
```

## Issues & PRs
Feel free to make issues and pull requests if you encounter any issues!

You can also contact me on Discord if you have questions: `mtheall#6174`

## Legal Notice
RocketSim was written to replicate Rocket League's game logic, but does not actually contain any code from the game.
To Epic Games/Psyonix: If any of you guys have an issue with this, let me know on Discord and we can resolve it.
