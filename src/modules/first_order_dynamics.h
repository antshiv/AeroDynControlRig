/**
 * @file first_order_dynamics.h
 * @brief First-order linear system dynamics for testing and validation
 */

#ifndef FIRST_ORDER_DYNAMICS_H
#define FIRST_ORDER_DYNAMICS_H

#include "core/module.h"

/**
 * @class FirstOrderDynamicsModule
 * @brief Simulates a simple first-order linear time-invariant (LTI) system
 *
 * This module provides a test platform for control algorithm development.
 * It implements the transfer function:
 *
 *   G(s) = K / (τs + 1)
 *
 * Where:
 * - K = gain (output/input ratio at steady state)
 * - τ = time constant (seconds to reach 63.2% of final value)
 *
 * **State-space form**:
 *   dx/dt = (1/τ) * (K*u - x)
 *   y = x
 *
 * Input modes:
 * - Constant: Fixed target value
 * - Sinusoidal: Frequency-swept input for Bode analysis
 *
 * Useful for:
 * - Testing PID controllers
 * - Verifying numerical integration
 * - Demonstrating basic control concepts
 */
class FirstOrderDynamicsModule : public Module {
public:
    /**
     * @brief Initialize internal state to zero
     * @param state Reference to simulation state
     */
    void initialize(SimulationState& state) override;

    /**
     * @brief Update first-order dynamics state
     *
     * Integrates the differential equation using Euler method.
     * Reads config from SimulationState::dynamics_config and writes
     * output to SimulationState::dynamics_state.
     *
     * @param dt Time step (seconds)
     * @param state Reference to simulation state (reads dynamics_config,
     *              writes dynamics_state)
     */
    void update(double dt, SimulationState& state) override;

private:
    double internal_state_{0.0}; ///< Current system state (output value)
    double time_constant_{1.0};  ///< System time constant τ (seconds)
    double gain_{1.0};           ///< System gain K (dimensionless)
};

#endif // FIRST_ORDER_DYNAMICS_H
