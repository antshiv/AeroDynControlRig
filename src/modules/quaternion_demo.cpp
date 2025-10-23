#include "modules/quaternion_demo.h"

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

#include "attitude/euler.h"
#include "attitude/dcm.h"
#include "attitude/quaternion.h"
#include "attitude/attitude_utils.h"
#include "core/simulation_state.h"

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

void normalize_angle(double& angle) {
    while (angle > kPi) {
        angle -= kTwoPi;
    }
    while (angle < -kPi) {
        angle += kTwoPi;
    }
}
}  // namespace

void QuaternionDemoModule::initialize(SimulationState& state) {
    state.euler.roll = 0.0;
    state.euler.pitch = 0.0;
    state.euler.yaw = 0.0;
    state.euler.order = EULER_ZYX;
    state.quaternion = {1.0, 0.0, 0.0, 0.0};
    state.model_matrix = glm::mat4(1.0f);
    state.rotation_speed_deg_per_sec = 30.0;
}

void QuaternionDemoModule::update(double dt, SimulationState& state) {
    if (dt <= 0.0) {
        return;
    }

    state.time_seconds += dt;

    const double yaw_increment = deg2rad(state.rotation_speed_deg_per_sec) * dt;
    state.euler.yaw += yaw_increment;
    normalize_angle(state.euler.yaw);

    double dcm[3][3];
    euler_to_dcm(&state.euler, dcm);

    glm::mat4 model(1.0f);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            model[col][row] = static_cast<float>(dcm[row][col]);
        }
    }
    state.model_matrix = model;

    double q[4];
    euler_to_quaternion(&state.euler, q);
    state.quaternion = {q[0], q[1], q[2], q[3]};
}
