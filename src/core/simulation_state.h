/**
 * @file simulation_state.h
 * @brief Centralized simulation state shared across all modules and panels
 */

#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <array>
#include <deque>
#include <limits>
#include <glm/glm.hpp>
#include "attitude/euler.h"

/**
 * @struct SimulationState
 * @brief Central shared state for the entire simulation
 *
 * This structure contains all simulation data, configuration, and telemetry.
 * It is passed to all modules (for read/write) and panels (for visualization).
 * The state is organized into nested structs for logical grouping.
 *
 * Key design principles:
 * - Single source of truth for all simulation data
 * - Modules communicate through state (loose coupling)
 * - Panels have read/write access for interactive control
 */
struct SimulationState {
    // === Attitude Representation ===
    EulerAngles euler{0.0, 0.0, 0.0, EULER_ZYX};              ///< Current attitude in Euler angles (ZYX convention)
    std::array<double, 4> quaternion{1.0, 0.0, 0.0, 0.0};     ///< Current attitude quaternion [w, x, y, z]
    glm::mat4 model_matrix{1.0f};                             ///< Model transformation matrix for rendering
    glm::dvec3 angular_rate_deg_per_sec{0.0, 0.0, 30.0};     ///< Angular velocity (deg/s) about body axes

    /**
     * @struct AttitudeSample
     * @brief Historical sample of attitude for plotting/analysis
     */
    struct AttitudeSample {
        double timestamp{0.0};                      ///< Absolute simulation time of the sample
        std::array<double, 4> quaternion{1.0, 0.0, 0.0, 0.0}; ///< Quaternion components [w, x, y, z]
        double roll{0.0};                           ///< Roll angle (rad)
        double pitch{0.0};                          ///< Pitch angle (rad)
        double yaw{0.0};                            ///< Yaw angle (rad)
        glm::dvec3 angular_rate{0.0};               ///< Angular rates (rad/s) in body frame
    };

    struct AttitudeHistory {
        std::deque<AttitudeSample> samples;     ///< Ring-buffer of samples (oldest at front)
        double window_seconds{15.0};            ///< Time window to retain (seconds)
        double sample_interval{0.016};          ///< Desired sampling period (seconds) - ~60Hz for smooth plots
        double last_sample_time{-std::numeric_limits<double>::infinity()}; ///< Timestamp of last captured sample
    } attitude_history;

    /**
     * @struct RotorSample
     * @brief Historical sample of rotor telemetry for one motor
     */
    struct RotorSample {
        double timestamp{0.0};      ///< Absolute simulation time
        float rpm{0.0f};            ///< Revolutions per minute
        float thrust{0.0f};         ///< Thrust force (N)
        float power{0.0f};          ///< Power consumption (W)
        float temperature{0.0f};    ///< Motor temperature (°C)
        float voltage{0.0f};        ///< Motor voltage (V)
        float current{0.0f};        ///< Motor current (A)
    };

    struct RotorHistory {
        std::deque<RotorSample> rotor1_samples;  ///< Motor 1 telemetry
        std::deque<RotorSample> rotor2_samples;  ///< Motor 2 telemetry
        std::deque<RotorSample> rotor3_samples;  ///< Motor 3 telemetry
        std::deque<RotorSample> rotor4_samples;  ///< Motor 4 telemetry
        double window_seconds{60.0};             ///< Time window (60s for rotor analysis)
        double sample_interval{0.1};             ///< Sample rate (10 Hz)
        double last_sample_time{-std::numeric_limits<double>::infinity()};
    } rotor_history;

    /**
     * @struct SensorSample
     * @brief Historical IMU sensor sample
     */
    struct SensorSample {
        double timestamp{0.0};           ///< Absolute simulation time
        glm::vec3 gyro_rad_s{0.0f};      ///< Gyroscope (rad/s)
        glm::vec3 accel_mps2{0.0f};      ///< Accelerometer (m/s²)
        glm::vec3 mag_gauss{0.0f};       ///< Magnetometer (gauss)
    };

    struct SensorHistory {
        std::deque<SensorSample> samples;
        double window_seconds{30.0};     ///< Time window (30s for sensor plots)
        double sample_interval{0.01};    ///< Sample rate (100 Hz, typical IMU rate)
        double last_sample_time{-std::numeric_limits<double>::infinity()};
    } sensor_history;

    struct AttitudeHistoryVideoConfig {
        bool recording{true};                   ///< Whether video capture is active
        double playback_speed{1.0};             ///< Playback speed multiplier for history replay
        float trail_length_seconds{5.0f};       ///< Duration of on-scene trail overlay
        float trail_width{2.0f};                ///< Pixel width for trail rendering
    } attitude_history_video;


    // === Timing ===
    double time_seconds{0.0};      ///< Elapsed simulation time (seconds)
    double last_dt{0.0};           ///< Last frame's timestep (seconds)

    // === Physics State (6-DOF Rigid Body) ===
    /**
     * @struct PhysicsState
     * @brief Complete 6-DOF rigid body state for quadcopter dynamics
     */
    struct PhysicsState {
        glm::dvec3 position{0.0, 0.0, 0.0};         ///< Position in inertial frame (m)
        glm::dvec3 velocity{0.0, 0.0, 0.0};         ///< Velocity in inertial frame (m/s)
        glm::dvec3 force_body{0.0, 0.0, 0.0};       ///< Total force in body frame (N)
        glm::dvec3 torque_body{0.0, 0.0, 0.0};      ///< Total torque in body frame (N·m)
        glm::dvec3 acceleration{0.0, 0.0, 0.0};     ///< Acceleration in inertial frame (m/s²)
        double mass{0.5};                            ///< Vehicle mass (kg)
        glm::dmat3 inertia{{0.01, 0.0, 0.0},        ///< Inertia tensor (kg·m²)
                           {0.0, 0.01, 0.0},
                           {0.0, 0.0, 0.02}};
    } physics;

    /**
     * @struct VehicleConfig
     * @brief Physical parameters for quadcopter model
     */
    struct VehicleConfig {
        double mass{0.5};                ///< Total mass including battery (kg)
        double arm_length{0.225};        ///< Distance from center to rotor (m)
        double gravity{9.81};            ///< Gravitational acceleration (m/s²)
        double drag_coefficient{0.01};   ///< Linear drag coefficient

        // Inertia tensor (kg·m²) - typical 450mm quadcopter
        double Ixx{0.0075};  ///< Moment of inertia about X axis
        double Iyy{0.0075};  ///< Moment of inertia about Y axis
        double Izz{0.0130};  ///< Moment of inertia about Z axis
    } vehicle_config;

    /**
     * @struct MotorCommands
     * @brief Commanded rotor speeds for control input
     */
    struct MotorCommands {
        std::array<double, 4> omega_rad_s{0.0, 0.0, 0.0, 0.0};  ///< Commanded angular velocities (rad/s)
        std::array<double, 4> throttle_0_1{0.0, 0.0, 0.0, 0.0}; ///< Throttle commands [0, 1]
    } motor_commands;

    /**
     * @struct DynamicsConfig
     * @brief Configuration for first-order dynamics test module
     */
    struct DynamicsConfig {
        double input_target{1.0};           ///< Target input value (constant mode)
        bool use_sine{false};               ///< Enable sinusoidal input signal
        double sine_frequency_hz{0.5};      ///< Frequency for sinusoidal input (Hz)
        double time_constant{1.0};          ///< First-order system time constant (seconds)
        double gain{1.0};                   ///< System gain
    } dynamics_config;

    /**
     * @struct DynamicsState
     * @brief State variables for first-order dynamics module
     */
    struct DynamicsState {
        double input{0.0};          ///< Current input to the dynamics system
        double output{0.0};         ///< Current output of the dynamics system
        double internal_state{0.0}; ///< Internal state variable
    } dynamics_state;

    /**
     * @struct SensorFrame
     * @brief Simulated IMU sensor measurements
     */
    struct SensorFrame {
        glm::vec3 gyro_rad_s{0.0f};   ///< Gyroscope measurement (rad/s) in body frame
        glm::vec3 accel_mps2{0.0f};   ///< Accelerometer measurement (m/s²) in body frame
    } sensor;

    /**
     * @struct EstimatorState
     * @brief State estimate from sensor fusion algorithm
     */
    struct EstimatorState {
        std::array<double, 4> quaternion{1.0, 0.0, 0.0, 0.0}; ///< Estimated attitude quaternion [w, x, y, z]
        EulerAngles euler{0.0, 0.0, 0.0, EULER_ZYX};         ///< Estimated attitude in Euler angles
    } estimator;

    /**
     * @struct RotorConfig
     * @brief Physical configuration for rotor/propeller models
     */
    struct RotorConfig {
        double thrust_coefficient{1.2e-6};  ///< Thrust coefficient (N/(rad/s)²)
        double torque_coefficient{2.5e-7};  ///< Torque coefficient (N·m/(rad/s)²)
        double arm_length_m{0.2};           ///< Distance from rotor to center of mass (meters)
    } rotor_config;

    /**
     * @struct RotorTelemetry
     * @brief Computed rotor performance metrics
     */
    struct RotorTelemetry {
        std::array<double, 4> rpm{0.0, 0.0, 0.0, 0.0};                  ///< Rotor speeds (RPM)
        std::array<double, 4> thrust_newton{0.0, 0.0, 0.0, 0.0};        ///< Individual thrust per rotor (N)
        std::array<double, 4> torque_newton_metre{0.0, 0.0, 0.0, 0.0}; ///< Individual torque per rotor (N·m)
        double total_thrust_newton{0.0};                                ///< Sum of all rotor thrust (N)
        double total_power_watt{0.0};                                   ///< Total electrical power consumption (W)
    } rotor;

    /**
     * @struct PowerHistory
     * @brief Electrical power consumption tracking
     */
    struct PowerHistory {
        double bus_voltage{22.2};   ///< Battery/bus voltage (V)
        double bus_current{0.0};    ///< Total current draw (A)
        double energy_joule{0.0};   ///< Cumulative energy consumed (J)
    } power;

    /**
     * @struct SimulationControl
     * @brief User-controlled simulation playback parameters
     */
    struct SimulationControl {
        bool paused{false};              ///< Pause simulation updates
        bool use_legacy_ui{false};       ///< Toggle between legacy and dashboard layouts
        bool use_fixed_dt{false};        ///< Use fixed timestep instead of real-time
        double fixed_dt{0.01};           ///< Fixed timestep value (seconds)
        double time_scale{1.0};          ///< Simulation speed multiplier
        bool manual_rotation_mode{false}; ///< If true: discrete step rotation (W/A/S/D/Q/E). If false: continuous angular rates (arrow keys)
    } control;
};

#endif // SIMULATION_STATE_H
