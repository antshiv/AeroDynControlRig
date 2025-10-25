/**
 * @file complementary_estimator.h
 * @brief Complementary filter for attitude estimation from IMU data
 */

#ifndef COMPLEMENTARY_ESTIMATOR_H
#define COMPLEMENTARY_ESTIMATOR_H

#include <array>
#include <glm/glm.hpp>

#include "core/module.h"

/**
 * @class ComplementaryEstimatorModule
 * @brief Estimates attitude using a complementary filter fusing gyro and accel
 *
 * This module implements a quaternion-based complementary filter with gyro
 * bias estimation. The filter combines:
 * - **Gyroscope**: High-frequency attitude updates (drift-prone)
 * - **Accelerometer**: Low-frequency corrections assuming gravity-only environment
 *
 * Algorithm:
 * 1. Predict quaternion using bias-corrected gyro measurements
 * 2. Compute error quaternion from accelerometer-derived attitude
 * 3. Apply proportional-integral (PI) correction to quaternion and bias
 *
 * Tuning parameters:
 * - kp: Proportional gain (attitude correction speed)
 * - ki: Integral gain (bias estimation speed)
 *
 * @see SensorSimulatorModule
 */
class ComplementaryEstimatorModule : public Module {
public:
    /**
     * @brief Initialize estimator to identity quaternion with zero bias
     * @param state Reference to simulation state
     */
    void initialize(SimulationState& state) override;

    /**
     * @brief Update attitude estimate using gyro and accel measurements
     *
     * Reads SimulationState::sensor data and writes to SimulationState::estimator.
     *
     * @param dt Time step (seconds)
     * @param state Reference to simulation state (reads sensor.gyro/accel,
     *              writes estimator.quaternion and estimator.euler)
     */
    void update(double dt, SimulationState& state) override;

    /**
     * @brief Tune the complementary filter gains
     * @param kp Proportional gain (higher = faster attitude correction)
     * @param ki Integral gain (higher = faster bias estimation)
     */
    void setGains(float kp, float ki) {
        kp_ = kp;
        ki_ = ki;
    }

private:
    std::array<double, 4> q_est_{1.0, 0.0, 0.0, 0.0}; ///< Estimated attitude quaternion [w, x, y, z]
    glm::vec3 bias_{0.0f};                            ///< Estimated gyroscope bias (rad/s)
    float kp_{2.0f};                                  ///< Proportional gain
    float ki_{0.05f};                                 ///< Integral gain
};

#endif // COMPLEMENTARY_ESTIMATOR_H
