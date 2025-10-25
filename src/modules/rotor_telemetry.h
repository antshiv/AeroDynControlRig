/**
 * @file rotor_telemetry.h
 * @brief Quadcopter rotor performance calculation module
 */

#ifndef ROTOR_TELEMETRY_H
#define ROTOR_TELEMETRY_H

#include "core/module.h"

/**
 * @class RotorTelemetryModule
 * @brief Computes rotor thrust, torque, and power from RPM measurements
 *
 * This module calculates performance metrics for a quadcopter's rotors
 * using first-principles aerodynamics:
 *
 * **Thrust**: F = k_T * ω²
 * **Torque**: τ = k_Q * ω²
 * **Power**: P = τ * ω
 *
 * Where:
 * - ω = angular velocity (rad/s) = RPM * (2π/60)
 * - k_T = thrust coefficient (N/(rad/s)²)
 * - k_Q = torque coefficient (N·m/(rad/s)²)
 *
 * The module currently generates synthetic RPM data (sinusoidal variation).
 * Future enhancements:
 * - Accept commanded RPM from controller module
 * - Add rotor dynamics (lag between command and actual RPM)
 * - Include motor voltage/current modeling
 *
 * @see SimulationState::RotorConfig
 * @see SimulationState::RotorTelemetry
 */
class RotorTelemetryModule : public Module {
public:
    /**
     * @brief Initialize rotor parameters
     * @param state Reference to simulation state
     */
    void initialize(SimulationState& state) override;

    /**
     * @brief Update rotor telemetry based on RPM
     *
     * Calculates per-rotor thrust, torque, and total power consumption.
     * Writes results to SimulationState::rotor.
     *
     * @param dt Time step (seconds)
     * @param state Reference to simulation state (reads rotor_config,
     *              writes rotor telemetry including thrust/torque/power)
     */
    void update(double dt, SimulationState& state) override;

private:
    double base_rpm_{1500.0}; ///< Baseline RPM for synthetic data generation
    double phase_{0.0};       ///< Phase accumulator for sinusoidal RPM variation
};

#endif // ROTOR_TELEMETRY_H
