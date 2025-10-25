/**
 * @file simulation_state.h
 * @brief Centralized simulation state shared across all modules and panels
 */

#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <array>
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

    // === Timing ===
    double time_seconds{0.0};      ///< Elapsed simulation time (seconds)
    double last_dt{0.0};           ///< Last frame's timestep (seconds)

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
        bool paused{false};          ///< Pause simulation updates
        bool use_legacy_ui{false};   ///< Toggle between legacy and dashboard layouts
        bool use_fixed_dt{false};    ///< Use fixed timestep instead of real-time
        double fixed_dt{0.01};       ///< Fixed timestep value (seconds)
        double time_scale{1.0};      ///< Simulation speed multiplier
    } control;
};

#endif // SIMULATION_STATE_H
