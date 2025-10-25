/**
 * @file quaternion_demo.h
 * @brief Quaternion-based attitude dynamics simulation module
 */

#ifndef QUATERNION_DEMO_H
#define QUATERNION_DEMO_H

#include "core/module.h"

/**
 * @class QuaternionDemoModule
 * @brief Simulates rigid body attitude dynamics using quaternion integration
 *
 * This module updates the vehicle's attitude quaternion based on the angular
 * velocity specified in SimulationState. It demonstrates quaternion-based
 * rotational kinematics and converts the quaternion to:
 * - Euler angles (for display)
 * - Rotation matrix (for 3D rendering)
 *
 * The integration uses a simple Euler method with quaternion normalization
 * to prevent numerical drift.
 */
class QuaternionDemoModule : public Module {
public:
    /**
     * @brief Initialize the module (sets initial quaternion to identity)
     * @param state Reference to simulation state
     */
    void initialize(SimulationState& state) override;

    /**
     * @brief Update attitude quaternion based on angular velocity
     *
     * Performs quaternion integration:
     * q_dot = 0.5 * omega * q
     *
     * Where omega is the angular velocity quaternion [0, wx, wy, wz].
     * The result is normalized and converted to Euler angles and rotation matrix.
     *
     * @param dt Time step (seconds)
     * @param state Reference to simulation state (reads angular_rate_deg_per_sec,
     *              writes quaternion, euler, model_matrix)
     */
    void update(double dt, SimulationState& state) override;
};

#endif // QUATERNION_DEMO_H
