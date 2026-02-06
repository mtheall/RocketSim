import numpy
import types
from _typeshed import Incomplete
from typing import Any, ClassVar, overload

RLConst: types.SimpleNamespace

class Angle:
    """
    RocketSim euler angle struct.

    Has the same order of application as Rotators in Rocket League (YPR),
    values are in radians.
    """

    # Rotation around the vertical axis (Y-axis)
    yaw: float

    # Rotation around the lateral axis (X-axis)
    pitch: float

    # Rotation around the longitudinal axis (Z-axis)
    roll: float

    def __init__(self, yaw: float, pitch: float, roll: float, *args, **kwargs) -> None:
        """
        Create an angle from yaw, pitch, and roll values (in radians).

        Default values are zero if not provided.
        """
        ...

    def as_numpy(self) -> numpy.ndarray:
        """
        Return the angle as a NumPy array in (yaw, pitch, roll) order.
        """
        ...

    def as_rot_mat(self) -> RotMat:
        """
        Convert this angle into a rotation matrix.
        """
        ...

    def as_tuple(self) -> tuple:
        """
        Return the angle as a tuple in (yaw, pitch, roll) order.
        """
        ...

    def __copy__(self) -> Angle:
        """
        Return a shallow copy of this angle.
        """
        ...

    def __deepcopy__(self, memo) -> Angle:
        """
        Return a deep copy of this angle.
        """
        ...

    def __delitem__(self, other) -> None:
        """
        Item deletion is not supported for Angle.
        """
        ...

    def __format__(self, *args, **kwargs) -> str:
        """
        Format the angle as a string.

        Matches the C++ stream output format.
        """
        ...

    def __getitem__(self, index):
        """
        Get an angle component by index.

        Index mapping:
            0 -> yaw
            1 -> pitch
            2 -> roll
        """
        ...

    def __len__(self) -> int:
        """
        Return the number of components in the angle (always 3).
        """
        ...

    def __setitem__(self, index, object) -> None:
        """
        Set an angle component by index.

        Index mapping:
            0 -> yaw
            1 -> pitch
            2 -> roll
        """
        ...

class Arena:
    """
    The container for all game simulation.

    Stores cars, the ball, all arena collisions, and manages the overall game state.
    """

    # The ball instance owned by this arena
    ball: Ball

    # Total score for the blue team
    blue_score: int

    # The current game mode
    game_mode: GameMode

    # Total score for the orange team
    orange_score: int

    # Total ticks this arena instance has been simulated for, never resets
    tick_count: int

    # Returns (1 / tick_time)
    tick_rate: float

    # Time in seconds each tick (1 / tick_rate)
    tick_time: float

    def __init__(
        self,
        game_mode: GameMode,
        config: ArenaConfig,
        tick_rate: float = 120.0,
        *args,
        **kwargs
    ) -> None:
        """
        Create a new arena instance.

        NOTE: Arena should be destroyed after use.
        """
        ...

    @overload
    def add_car(self, team: int, config: int = ...) -> Car:
        """
        Add a car to the arena.

        Returns the newly added car.
        """
        ...

    @overload
    def add_car(self, team: int, config: CarConfig) -> Car:
        """
        Add a car to the arena using a full CarConfig.

        Returns the newly added car.
        """
        ...

    def clone(self, copy_callbacks: bool = ...) -> Arena:
        """
        Get a deep copy of the arena.
        """
        ...

    def clone_into(self, target: Arena, copy_callbacks: bool = ...) -> Any:
        """
        Get a deep copy of the arena into an existing target arena.
        """
        ...

    def get_ball_prediction(
        self,
        num_ticks: int = ...,
        tick_interval: int = ...
    ) -> list[BallState]:
        """
        Get a list of ball states for the next num_ticks ticks.
        """
        ...

    def get_boost_pads(self) -> list[BoostPad]:
        """
        Get all boost pads in the arena.
        """
        ...

    @overload
    def get_car_from_id(self, car_id: int) -> Car:
        """
        Get a car by its ID.
        """
        ...

    @overload
    def get_car_from_id(self, car_id: int, default) -> Car:
        """
        Get a car by its ID, or return default if not found.
        """
        ...

    def get_cars(self) -> list[Car]:
        """
        Get all cars currently in the arena.
        """
        ...

    def get_config(self) -> ArenaConfig:
        """
        Get the arena configuration.
        """
        ...

    def get_gym_state(self) -> tuple:
        """
        Get the current arena state in an RLGym-compatible format.
        """
        ...

    def get_mutator_config(self) -> MutatorConfig:
        """
        Get the mutator configuration.
        """
        ...

    def is_ball_probably_going_in(self, max_time: float = ...) -> bool:
        """
        Returns true if the ball is probably going in.

        NOTE: Purposefully overestimates, just like the real Rocket League shot prediction.
        Does not account for wall or ceiling bounces.
        """
        ...

    @staticmethod
    def multi_step(arenas: Sequence[Arena] = ..., ticks: int = ...) -> Any:
        """
        Simulate multiple arenas for a given number of ticks.
        """
        ...

    @overload
    def remove_car(self, car: Car) -> Any:
        """
        Remove a car from the arena.

        NOTE: If the car was removed, the car will be freed and the reference made invalid.
        """
        ...

    @overload
    def remove_car(self, car: int) -> Any:
        """
        Remove a car from the arena by ID.

        Returns false if the car ID was not found.
        """
        ...

    def reset_kickoff(self, seed=...) -> Any:
        """
        Reset the arena to a random kickoff.

        Seed may be provided for determinism.
        """
        ...

    def set_ball_touch_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for ball touch events.
        """
        ...

    def set_boost_pickup_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for boost pickup events.
        """
        ...

    def set_car_ball_collision(self, enable=...) -> None:
        """
        Enable or disable car-ball collisions.
        """
        ...

    def set_car_bump_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for car bump events.
        """
        ...

    def set_car_car_collision(self, enable=...) -> None:
        """
        Enable or disable car-car collisions.
        """
        ...

    def set_car_demo_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for car demo events.
        """
        ...

    def set_goal_event_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for goal events.
        """
        ...

    def set_goal_score_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for goal score events.
        """
        ...

    def set_mutator_config(self, config: MutatorConfig) -> Any:
        """
        Set the mutator configuration.
        """
        ...

    def set_save_event_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for save events.
        """
        ...

    def set_shot_event_callback(self, callback, data=...) -> tuple:
        """
        Set the callback for shot events.
        """
        ...

    def step(self, ticks: int = ...) -> Any:
        """
        Simulate everything in the arena for a given number of ticks.
        """
        ...

    def stop(self) -> Any:
        """
        Stop the arena simulation.
        """
        ...

    def __copy__(self) -> Arena:
        """
        Get a deep copy of the arena.
        """
        ...

    def __deepcopy__(self, memo) -> Arena:
        """
        Get a deep copy of the arena.
        """
        ...


class ArenaConfig:
    """
    Configuration for an Arena.

    Stores all physics bounds, boost pad settings, and performance-related options.
    """

    # Custom boost pads to use if use_custom_boost_pads is True
    custom_boost_pads: list[BoostPadConfig]

    # Maximum length of any object (distance from AABB min to AABB max)
    max_aabb_len: float

    # Maximum number of objects allowed in the arena
    max_objects: int

    # Maximum position of any physics object
    max_pos: Vec

    # Memory weight mode: HEAVY or LIGHT
    memory_weight_mode: bool

    # Minimum position of any physics object
    min_pos: Vec

    # Skip ball rotation updates to improve performance (Disabled in Snowday)
    no_ball_rot: bool

    # Use a custom broadphase designed for RocketSim
    # Improves performance, but inefficient on very large maps
    use_custom_broadphase: bool

    def __init__(self, *args, **kwargs) -> None:
        """
        Create a new ArenaConfig instance with optional overrides.
        """
        ...

    def __copy__(self) -> ArenaConfig:
        """
        Return a shallow copy of the arena configuration.
        """
        ...

    def __deepcopy__(self, memo) -> ArenaConfig:
        """
        Return a deep copy of the arena configuration.
        """
        ...

class Ball:
    """
    Represents the ball in RocketSim.

    Stores physics state, collision info, and handles physics interactions
    within the arena.
    """

    _internal_state: BallState
    _rigid_body: Any  # Underlying Bullet rigid body
    _collision_shape: Any  # Pointer to Bullet collision shape

    @classmethod
    def __init__(cls, *args, **kwargs) -> None:
        """
        Construct a new Ball instance.

        Note: Direct construction is private; use Arena to create balls.
        """
        ...

    def get_radius(self) -> float:
        """
        Return the ball's radius in Unreal Engine units (uu).

        Mirrors C++ GetRadius(), which multiplies Bullet units by BT_TO_UU.
        """
        ...

    def get_rot(self, *args, **kwargs):
        """
        Return the rotation of the ball.

        Uses Bullet physics orientation internally.
        """
        ...

    def get_state(self) -> BallState:
        """
        Return the current state of the ball (position, velocity, rotation, etc.).
        """
        ...

    def set_state(self, state: BallState) -> Any:
        """
        Set the ball's state to a given BallState.

        Mirrors C++ SetState().
        """
        ...

class BallHitInfo:
    """
    Stores information about a single ball hit in the arena.

    All fields are only valid if `is_valid` is True.
    """

    # Position of the hit relative to the ball's position
    relative_pos_on_ball: Vec

    # World position of the ball when the hit occurred
    ball_pos: Vec

    # Extra velocity added on top of the base collision velocity
    extra_hit_vel: Vec

    # True if this hit info is valid; otherwise, other fields should not be trusted
    is_valid: bool

    # Arena tick count when the hit occurred
    tick_count_when_hit: int

    # Arena tick count when the last extra ball-car hit impulse was applied
    # Used to prevent extra impulses on consecutive ticks
    tick_count_when_extra_impulse_applied: int

    def __init__(self, *args, **kwargs) -> None:
        """
        Construct a new BallHitInfo instance.
        """
        ...

    def __copy__(self) -> BallHitInfo:
        """
        Return a shallow copy of this BallHitInfo.
        """
        ...

    def __deepcopy__(self, memo) -> BallHitInfo:
        """
        Return a deep copy of this BallHitInfo.
        """
        ...

class BallPredictor:
    def __init__(self, *args, **kwargs) -> None: ...
    def get_ball_prediction(self, ball_state: BallState, ticks_since_last_update: int, num_states: int = ..., tick_interval: int = ...) -> list[BallState]: ...
    def __copy__(self) -> BallPredictor: ...
    def __deepcopy__(self, memo) -> BallPredictor: ...

class BallState:
    """
    Stores the full physics state of the ball in the arena.

    Inherits from PhysState (position, rotation, velocity, angular velocity, etc.)
    """

    # Linear velocity of the ball
    vel: Vec

    # Angular velocity of the ball
    ang_vel: Vec

    # Position of the ball in world space
    pos: Vec

    # Rotation of the ball as a rotation matrix
    rot_mat: RotMat

    # Heatseeker target info (used in Heatseeker gamemode)
    heatseeker_target_dir: Vec  # Direction of the target net, 0 if none
    heatseeker_target_speed: float  # Current target speed
    heatseeker_time_since_hit: float  # Time since last hit for Heatseeker tracking

    # ID of the last car that hit the ball
    last_hit_car_id: int

    # Incremented every physics update, reset when set via set_state()
    # Used internally to detect state changes
    update_counter: int

    def __init__(self, *args, **kwargs) -> None:
        """
        Construct a new BallState.
        Sets default z-position to BALL_REST_Z.
        """
        ...

    def __copy__(self) -> BallState:
        """
        Return a shallow copy of this BallState.
        """
        ...

    def __deepcopy__(self, memo) -> BallState:
        """
        Return a deep copy of this BallState.
        """
        ...

class BoostPad:
    is_big: bool
    @classmethod
    def __init__(cls, *args, **kwargs) -> None: ...
    def get_pos(self) -> Vec: """Get position of the boost pad.""" ...
    def get_state(self) -> BoostPadState: """Get the current state of the boost pad.""" ...
    def set_state(self, state: BoostPadState) -> Any: """Set the current state of the boost pad.""" ...

class BoostPadConfig:
    is_big: bool
    pos: Vec
    def __init__(self, *args, **kwargs) -> None: ...
    def __copy__(self) -> BoostPadConfig: ...
    def __deepcopy__(self, memo) -> BoostPadConfig: ...

class BoostPadState:
    cooldown: float
    is_active: bool
    prev_locked_car_id: int
    def __init__(self, *args, **kwargs) -> None: ...
    def __copy__(self) -> BoostPadState: ...
    def __deepcopy__(self, memo) -> BoostPadState: ...

class Car:
    """
    Represents a single car in the arena.

    Stores its physics state, controls, configuration, and statistics.
    """

    # Statistics
    assists: int        # Number of assists this car has
    boost_pickups: int  # Number of boost pickups collected
    demos: int          # Number of demolitions inflicted
    goals: int          # Number of goals scored
    saves: int          # Number of saves made
    shots: int          # Number of shots taken

    # Unique ID assigned by the arena, always >0
    id: int

    # Team of the car (BLUE or ORANGE)
    team: Team

    # Current configuration for this car
    config: CarConfig

    @classmethod
    def __init__(cls, *args, **kwargs) -> None:
        """
        Construct a Car. Typically allocated by Arena.
        """
        ...

    def demolish(self) -> Any:
        """
        Demolish the car. After DEMO_RESPAWN_TIME, it can be respawned.
        """
        ...

    def get_config(self) -> CarConfig:
        """
        Get the car's configuration (hitbox, wheels, etc.).
        """
        ...

    def get_controls(self) -> CarControls:
        """
        Get the current controls being applied to this car.
        """
        ...

    def get_forward_dir(self) -> Vec:
        """
        Get the forward direction as a unit vector.
        Mirrors _internalState.rotMat.forward.
        """
        ...

    def get_right_dir(self) -> Vec:
        """
        Get the rightward direction as a unit vector.
        Mirrors _internalState.rotMat.right.
        """
        ...

    def get_up_dir(self) -> Vec:
        """
        Get the upward direction as a unit vector.
        Mirrors _internalState.rotMat.up.
        """
        ...

    def get_state(self) -> CarState:
        """
        Get the current physics state of the car.
        Only updates values needed for external simulation.
        """
        ...

    def set_state(self, state: CarState) -> Any:
        """
        Set the physics state of the car.
        """
        ...

    def respawn(self, seed: int = ..., boost: float = ...) -> Any:
        """
        Respawn the car after demolition.
        Resets position, boost, and other relevant state.
        """
        ...

    def set_controls(self, controls: CarControls) -> Any:
        """
        Apply control inputs to this car for the next simulation step.
        """
        ...

class CarConfig:
    BREAKOUT: ClassVar[int] = ...
    DOMINUS: ClassVar[int] = ...
    HYBRID: ClassVar[int] = ...
    MERC: ClassVar[int] = ...
    OCTANE: ClassVar[int] = ...
    PLANK: ClassVar[int] = ...
    back_wheels: WheelPairConfig
    dodge_deadzone: float # (|yaw| + |pitch| + |roll|) will need to be >= this in order to flip
    front_wheels: WheelPairConfig
    hitbox_pos_offset: Vec # Offset of the hitbox (from it's origin) NOTE: Does not effect car's center of mass, that's always at local (0,0,0)
    hitbox_size: Vec # Full size of the hitbox (NOT the half-size/extent)
    def __init__(self, *args, **kwargs) -> None: ...
    def __copy__(self) -> CarConfig: ...
    def __deepcopy__(self, memo) -> CarConfig: ...

class CarControls:
    """Stores all control inputs to a car"""
    boost: bool
    handbrake: bool
    jump: bool
    pitch: float
    roll: float
    steer: float
    throttle: float
    yaw: float
    def __init__(self, 
                 throttle: float = 0.0, steer: float = 0.0,
                 pitch: float = 0.0, yaw: float = 0.0, roll: float = 0.0,
                 jump: bool = False, boost: bool = False, handbrake: bool = False,
                 *args, **kwargs) -> None: ...
    def clamp_fix(self, *args, **kwargs): """Makes all values range-valid (champs from -1 to 1) """ ...
    def __copy__(self) -> CarControls: ...
    def __deepcopy__(self, memo) -> CarControls: ...

class CarState:
    """
    Stores the full physics state of a car.

    Includes position, rotation, velocity, angular velocity, boost, jump/flip state,
    wheel contacts, supersonic info, last controls, and ball interaction info.
    """

    # Physics and movement
    pos: Vec                    # World position of the car
    vel: Vec                    # Linear velocity
    ang_vel: Vec                # Angular velocity
    rot_mat: RotMat             # Rotation matrix representing orientation

    # Jump / Flip / Air
    is_on_ground: bool          # True if 3 or more wheels are in contact
    wheels_with_contact: int    # Number of wheels currently touching the ground
    has_jumped: bool            # True if the car has jumped into the air
    has_double_jumped: bool     # True if double jump has been used
    has_flipped: bool           # True if a flip was performed or is in progress
    is_flipping: bool           # True while actively flipping (not auto-flip)
    is_jumping: bool            # True while actively jumping
    air_time: float             # Total time spent airborne
    air_time_since_jump: float  # Time spent in the air since last jump
    jump_time: float            # Time since the jump started
    flip_time: float            # Time since the flip started
    flip_rel_torque: Vec        # Relative torque applied during flip
    is_auto_flipping: bool      # True if auto-flipping (e.g., dodge assists)
    auto_flip_timer: float      # Counts down during auto-flip
    auto_flip_torque_scale: float  # Torque scale for auto-flips

    # Boost / Supersonic
    boost: float                # Boost amount [0,100]
    time_spent_boosting: float  # Total boosting time this tick
    is_supersonic: bool         # True if supersonic
    supersonic_time: float      # Time spent supersonic

    # Handbrake
    handbrake_val: float        # Handbrake input value, rises/falls gradually

    # Demo / Respawn
    is_demoed: bool             # True if the car is demoed
    demo_respawn_timer: float   # Countdown timer until respawn after demo

    # Ball interactions
    ball_hit_info: BallHitInfo  # Stores ball hit info for last hit

    # World / Car contact
    has_world_contact: bool      # True if car touches world geometry
    world_contact_normal: Vec    # Normal vector at contact point
    car_contact_id: int          # ID of car this car contacted
    car_contact_cooldown_timer: float  # Cooldown timer for repeated car contact

    # Controls
    last_controls: CarControls   # Last tick's controls applied

    # Misc
    update_counter: int          # Incremented every tick, used to detect state sets

    def __init__(self, *args, **kwargs) -> None:
        """
        Construct a CarState. Typically created internally or when cloning cars.
        """
        ...

    def has_flip_or_jump(self, *args, **kwargs):
        """
        Returns True if the car is currently able to jump, double-jump, or flip.
        Mirrors C++ CarState::HasFlipOrJump().
        """
        ...

    def has_flip_reset(self, *args, **kwargs):
        """
        Returns True if the car currently has a flip from a flip reset (not a jump).
        Mirrors C++ CarState::HasFlipReset().
        """
        ...

    def got_flip_reset(self, *args, **kwargs):
        """
        Returns True if the car became airborne due to a flip reset.
        Mirrors C++ CarState::GotFlipReset().
        """
        ...

    def __copy__(self) -> CarState:
        """
        Create a shallow copy of the CarState.
        """
        ...

    def __deepcopy__(self, memo) -> CarState:
        """
        Create a deep copy of the CarState.
        """
        ...

class DemoMode:
    DISABLED: ClassVar[int] = ...
    NORMAL: ClassVar[int] = ...
    ON_CONTACT: ClassVar[int] = ...

class GameMode:
    HEATSEEKER: ClassVar[int] = ...
    HOOPS: ClassVar[int] = ...
    SNOWDAY: ClassVar[int] = ...
    SOCCAR: ClassVar[int] = ...
    THE_VOID: ClassVar[int] = ...

class MemoryWeightMode:
    HEAVY: ClassVar[int] = ...
    LIGHT: ClassVar[int] = ...

class MutatorConfig:
    ball_drag: float
    ball_hit_extra_force_scale: float
    ball_mass: float
    ball_max_speed: float
    ball_radius: float
    ball_world_friction: float
    ball_world_restitution: float
    boost_accel_air: float
    boost_accel_ground: float
    boost_pad_cooldown_big: float
    boost_pad_cooldown_small: float
    boost_used_per_second: float
    bump_cooldown_time: float
    bump_force_scale: float
    car_mass: float
    car_spawn_boost_amount: float
    car_world_friction: float
    car_world_restitution: float
    demo_mode: bool
    enable_car_ball_collision: bool
    enable_car_car_collision: bool
    enable_team_demos: bool
    goal_base_threshold_y: float
    gravity: float
    jump_accel: float
    jump_immediate_force: float
    respawn_delay: float
    unlimited_double_jumps: bool
    unlimited_flips: bool
    def __init__(self, *args, **kwargs) -> None: ...
    def __copy__(self) -> MutatorConfig: ...
    def __deepcopy__(self, memo) -> MutatorConfig: ...

class RotMat:
    """
    RocketSim 3x3 rotation matrix struct
    NOTE: Column-major.
    """
    forward: Vec
    right: Vec
    up: Vec
    def __init__(self, forward: Vec, right: Vec, up: Vec, *args, **kwargs) -> RotMat: ...
    def as_angle(self) -> Angle: ...
    def as_numpy(self) -> numpy.ndarray: ...
    def as_tuple(self) -> tuple: ...
    def __copy__(self) -> RotMat: ...
    def __deepcopy__(self, memo) -> RotMat: ...
    def __delitem__(self, other) -> None: ...
    def __format__(self, *args, **kwargs) -> str: ...
    def __getitem__(self, index): ...
    def __len__(self) -> int: ...
    def __setitem__(self, index, object) -> None: ...

class Team:
    BLUE: ClassVar[int] = ...
    ORANGE: ClassVar[int] = ...

class Vec:
    """RocketSim 3D vector struct"""
    x: float
    y: float
    z: float
    def __init__(self, x: float, y: float, z: float, *args, **kwargs) -> None: ...
    def as_numpy(self) -> numpy.ndarray: ...
    def as_tuple(self) -> tuple: ...
    def round(self, precision: float) -> float: ...
    def __copy__(self) -> Vec: ...
    def __deepcopy__(self, memo) -> Vec: ...
    def __delitem__(self, other) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __format__(self, *args, **kwargs) -> str: ...
    def __ge__(self, other: object) -> bool: ...
    def __getitem__(self, index): ...
    def __gt__(self, other: object) -> bool: ...
    def __le__(self, other: object) -> bool: ...
    def __len__(self) -> int: ...
    def __lt__(self, other: object) -> bool: ...
    def __ne__(self, other: object) -> bool: ...
    def __setitem__(self, index, object) -> None: ...

class WheelPairConfig:
    connection_point_offset: Vec # Where the wheel actually connects (suspension start position) NOTE: Y should ALWAYS be positive. It will be automatically negated when creating the second wheel.
    suspension_rest_length: float # How far out the suspension rests
    wheel_radius: float # Radius of both wheels
    def __init__(self, *args, **kwargs) -> None: ...
    def __copy__(self) -> WheelPairConfig: ...
    def __deepcopy__(self, memo) -> WheelPairConfig: ...

def init(*args, **kwargs): ...
