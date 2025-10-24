#include "modules/rotor_telemetry.h"

#include <cmath>

#include "core/simulation_state.h"

void RotorTelemetryModule::initialize(SimulationState& state) {
    base_rpm_ = 1500.0;
    phase_ = 0.0;

    state.rotor.total_power_watt = 0.0;
    state.rotor.total_thrust_newton = 0.0;
}

void RotorTelemetryModule::update(double dt, SimulationState& state) {
    (void)dt;

    // Simple synthetic model: base RPM plus slight phase offset per rotor
    constexpr double two_pi = 6.28318530718;
    phase_ += dt * 0.5; // slow oscillation

    double total_thrust = 0.0;
    double total_power = 0.0;

    for (std::size_t i = 0; i < state.rotor.rpm.size(); ++i) {
        double phase_offset = phase_ + static_cast<double>(i) * two_pi / 4.0;
        double rpm = base_rpm_ + 50.0 * std::sin(phase_offset);

        state.rotor.rpm[i] = rpm;

        // Momentum-theory-inspired placeholders
        double omega = rpm * two_pi / 60.0;
        double thrust = state.rotor_config.thrust_coefficient * omega * omega;
        double torque = state.rotor_config.torque_coefficient * omega * omega;
        double power = torque * omega;

        state.rotor.thrust_newton[i] = thrust;
        state.rotor.torque_newton_metre[i] = torque;

        total_thrust += thrust;
        total_power += power;
    }

    state.rotor.total_thrust_newton = total_thrust;
    state.rotor.total_power_watt = total_power;

    state.power.bus_current = total_power / state.power.bus_voltage;
    state.power.energy_joule += total_power * dt;
}
