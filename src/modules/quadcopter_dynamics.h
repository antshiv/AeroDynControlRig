/**
 * @file quadcopter_dynamics.h
 * @brief Physics-based quadcopter dynamics simulation module
 *
 * Integrates the dynamic_models library to provide realistic 6-DOF rigid-body
 * dynamics with rotor aerodynamics, gravity, and drag. Replaces the simple
 * QuaternionDemoModule with full Newton-Euler equations of motion.
 */

#ifndef MODULES_QUADCOPTER_DYNAMICS_H
#define MODULES_QUADCOPTER_DYNAMICS_H

#include "core/module.h"
#include "drone/physics_model.h"

/**
 * @class QuadcopterDynamicsModule
 * @brief Physics-based quadcopter simulation using dynamic_models library
 *
 * Features:
 * - 6-DOF rigid-body dynamics (position, velocity, orientation, angular rates)
 * - Newton-Euler equations with quaternion kinematics
 * - Individual rotor thrust/torque modeling
 * - Gravity, drag, and gyroscopic effects
 * - RK4 numerical integration for accuracy
 * - Configurable vehicle parameters (mass, inertia, rotor layout)
 *
 * Usage:
 *   - Call initialize() to set up vehicle configuration
 *   - Call update(dt, state) each frame to propagate physics
 *   - Motor commands from state.motor_commands are used as control inputs
 *   - Physics state is written to state.physics and state.quaternion
 */
class QuadcopterDynamicsModule : public Module {
public:
    QuadcopterDynamicsModule() = default;
    ~QuadcopterDynamicsModule() override = default;

    /**
     * @brief Initialize vehicle configuration and physics model
     *
     * Sets up:
     * - Quadcopter mass and inertia tensor
     * - Rotor positions in body frame (X configuration)
     * - Thrust/torque coefficients
     * - Initial hover state (motors spun up to hover thrust)
     */
    void initialize(SimulationState& state) override;

    /**
     * @brief Update physics simulation by dt seconds
     *
     * Integrates equations of motion using RK4:
     * - Reads motor commands from state.motor_commands
     * - Computes forces/torques from rotors + gravity + drag
     * - Integrates state derivatives (ṗ, v̇, q̇, ω̇)
     * - Updates state.physics, state.quaternion, state.euler
     * - Updates state.rotor telemetry
     *
     * @param dt Time step in seconds
     * @param state Simulation state (read motor_commands, write physics)
     */
    void update(double dt, SimulationState& state) override;

private:
    dm_vehicle_config_t vehicle_config_;    ///< Vehicle physical parameters
    dm_vehicle_model_t vehicle_model_;      ///< Runtime physics model
    dm_state_t physics_state_;              ///< Current vehicle state for dm library

    /**
     * @brief Copy dm_state to SimulationState
     */
    void copyStateToSim(const dm_state_t& dm_state, SimulationState& state);

    /**
     * @brief Copy SimulationState to dm_state
     */
    void copyStateFromSim(const SimulationState& state, dm_state_t& dm_state);

    /**
     * @brief Configure standard X-frame quadcopter rotor layout
     */
    void setupRotorConfiguration();

    /**
     * @brief Update rotor telemetry from physics model
     */
    void updateRotorTelemetry(SimulationState& state);
};

#endif // MODULES_QUADCOPTER_DYNAMICS_H
