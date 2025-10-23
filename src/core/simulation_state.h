#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <array>
#include <glm/glm.hpp>
#include "attitude/euler.h"

struct SimulationState {
    EulerAngles euler{0.0, 0.0, 0.0, EULER_ZYX};
    std::array<double, 4> quaternion{1.0, 0.0, 0.0, 0.0};
    glm::mat4 model_matrix{1.0f};
    double rotation_speed_deg_per_sec{30.0};
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

    struct SimulationControl {
        bool paused{false};
        bool use_fixed_dt{false};
        double fixed_dt{0.01};
        double time_scale{1.0};
    } control;
};

#endif // SIMULATION_STATE_H
