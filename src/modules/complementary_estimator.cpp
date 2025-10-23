#include "modules/complementary_estimator.h"

#include <cmath>

#include "attitude/quaternion.h"
#include "attitude/attitude_utils.h"
#include "attitude/dcm.h"
#include "core/simulation_state.h"

namespace {
void normalize_quaternion(std::array<double, 4>& q) {
    double norm = std::sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    if (norm <= 0.0) {
        q = {1.0, 0.0, 0.0, 0.0};
    } else {
        double inv = 1.0 / norm;
        for (double& v : q) {
            v *= inv;
        }
    }
}
}

void ComplementaryEstimatorModule::initialize(SimulationState& state) {
    q_est_ = state.quaternion;
    normalize_quaternion(q_est_);
    bias_ = glm::vec3(0.0f);
    state.estimator.quaternion = q_est_;
    quaternion_to_euler(q_est_.data(), &state.estimator.euler.roll, &state.estimator.euler.pitch, &state.estimator.euler.yaw);
    state.estimator.euler.order = EULER_ZYX;
}

void ComplementaryEstimatorModule::update(double dt, SimulationState& state) {
    if (dt <= 0.0f) {
        return;
    }

    glm::vec3 gyro = state.sensor.gyro_rad_s - bias_;

    double dq[4];
    double q0 = q_est_[0];
    double q1 = q_est_[1];
    double q2 = q_est_[2];
    double q3 = q_est_[3];

    double gx = gyro.x;
    double gy = gyro.y;
    double gz = gyro.z;

    dq[0] = -0.5 * (q1 * gx + q2 * gy + q3 * gz);
    dq[1] =  0.5 * (q0 * gx + q2 * gz - q3 * gy);
    dq[2] =  0.5 * (q0 * gy - q1 * gz + q3 * gx);
    dq[3] =  0.5 * (q0 * gz + q1 * gy - q2 * gx);

    for (int i = 0; i < 4; ++i) {
        q_est_[i] += dq[i] * dt;
    }
    normalize_quaternion(q_est_);

    glm::vec3 accel = state.sensor.accel_mps2;
    float accel_norm = glm::length(accel);
    if (accel_norm > 1e-3f) {
        glm::vec3 accel_unit = accel / accel_norm;

        double q[4] = {q_est_[0], q_est_[1], q_est_[2], q_est_[3]};
        double dcm[3][3];
        quaternion_to_dcm(q, dcm);
        glm::vec3 gravity_est(
            static_cast<float>(dcm[0][2]),
            static_cast<float>(dcm[1][2]),
            static_cast<float>(dcm[2][2])
        );

        glm::vec3 error = glm::cross(gravity_est, -accel_unit);

        glm::vec3 correction = kp_ * error;
        bias_ += static_cast<float>(ki_ * dt) * error;

        gyro += correction;

        gx = gyro.x;
        gy = gyro.y;
        gz = gyro.z;

        dq[0] = -0.5 * (q1 * gx + q2 * gy + q3 * gz);
        dq[1] =  0.5 * (q0 * gx + q2 * gz - q3 * gy);
        dq[2] =  0.5 * (q0 * gy - q1 * gz + q3 * gx);
        dq[3] =  0.5 * (q0 * gz + q1 * gy - q2 * gx);

        for (int i = 0; i < 4; ++i) {
            q_est_[i] += dq[i] * dt;
        }
        normalize_quaternion(q_est_);
    }

    state.estimator.quaternion = q_est_;
    quaternion_to_euler(q_est_.data(), &state.estimator.euler.roll, &state.estimator.euler.pitch, &state.estimator.euler.yaw);
    state.estimator.euler.order = EULER_ZYX;
}
