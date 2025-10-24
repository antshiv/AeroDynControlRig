#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <array>
#include <glm/glm.hpp>
#include "attitude/euler.h"

struct SimulationState {
    EulerAngles euler{0.0, 0.0, 0.0, EULER_ZYX};
    std::array<double, 4> quaternion{1.0, 0.0, 0.0, 0.0};
    glm::mat4 model_matrix{1.0f};
    glm::dvec3 angular_rate_deg_per_sec{0.0, 0.0, 30.0};
    double time_seconds{0.0};
    double last_dt{0.0};

    struct DynamicsConfig {
        double input_target{1.0};
        bool use_sine{false};
        double sine_frequency_hz{0.5};
        double time_constant{1.0};
        double gain{1.0};
    } dynamics_config;

    struct DynamicsState {
        double input{0.0};
        double output{0.0};
        double internal_state{0.0};
    } dynamics_state;

    struct SensorFrame {
        glm::vec3 gyro_rad_s{0.0f};
        glm::vec3 accel_mps2{0.0f};
    } sensor;

    struct EstimatorState {
        std::array<double, 4> quaternion{1.0, 0.0, 0.0, 0.0};
        EulerAngles euler{0.0, 0.0, 0.0, EULER_ZYX};
    } estimator;

    struct RotorConfig {
        double thrust_coefficient{1.2e-6};
        double torque_coefficient{2.5e-7};
        double arm_length_m{0.2};
    } rotor_config;

    struct RotorTelemetry {
        std::array<double, 4> rpm{0.0, 0.0, 0.0, 0.0};
        std::array<double, 4> thrust_newton{0.0, 0.0, 0.0, 0.0};
        std::array<double, 4> torque_newton_metre{0.0, 0.0, 0.0, 0.0};
        double total_thrust_newton{0.0};
        double total_power_watt{0.0};
    } rotor;

    struct PowerHistory {
        double bus_voltage{22.2};
        double bus_current{0.0};
        double energy_joule{0.0};
    } power;

    struct SimulationControl {
        bool paused{false};
        bool use_fixed_dt{false};
        double fixed_dt{0.01};
        double time_scale{1.0};
    } control;
};

#endif // SIMULATION_STATE_H
