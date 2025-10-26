#include "modules/rotor_telemetry.h"

#include <cmath>
#include <limits>

#include "core/simulation_state.h"

void RotorTelemetryModule::initialize(SimulationState& state) {
    // Initialize history buffers
    for (int i = 0; i < 4; ++i) {
        state.rotor_history.window_seconds = 60.0;
        state.rotor_history.sample_interval = 0.1; // 10 Hz sampling
        state.rotor_history.last_sample_time = -std::numeric_limits<double>::infinity();
    }
}

void RotorTelemetryModule::update(double dt, SimulationState& state) {
    // Capture rotor telemetry to history buffers (data comes from QuadcopterDynamicsModule)
    if (state.time_seconds - state.rotor_history.last_sample_time >= state.rotor_history.sample_interval) {
        // Sample each rotor
        for (int i = 0; i < 4; ++i) {
            SimulationState::RotorSample sample;
            sample.timestamp = state.time_seconds;
            sample.rpm = state.rotor.rpm[i];
            sample.thrust = state.rotor.thrust_newton[i];
            sample.power = state.rotor.total_power_watt / 4.0; // Divide total by 4 for now
            sample.temperature = 25.0 + (sample.power * 0.1); // Simple thermal model
            sample.voltage = state.power.bus_voltage;
            sample.current = (sample.power > 0) ? (sample.power / sample.voltage) : 0.0;

            // Add to appropriate rotor history
            switch (i) {
                case 0: state.rotor_history.rotor1_samples.push_back(sample); break;
                case 1: state.rotor_history.rotor2_samples.push_back(sample); break;
                case 2: state.rotor_history.rotor3_samples.push_back(sample); break;
                case 3: state.rotor_history.rotor4_samples.push_back(sample); break;
            }
        }

        state.rotor_history.last_sample_time = state.time_seconds;

        // Prune old samples for all rotors
        auto prune_samples = [&](std::deque<SimulationState::RotorSample>& samples) {
            while (!samples.empty()) {
                double age = state.time_seconds - samples.front().timestamp;
                if (age > state.rotor_history.window_seconds) {
                    samples.pop_front();
                } else {
                    break;
                }
            }
        };

        prune_samples(state.rotor_history.rotor1_samples);
        prune_samples(state.rotor_history.rotor2_samples);
        prune_samples(state.rotor_history.rotor3_samples);
        prune_samples(state.rotor_history.rotor4_samples);
    }

    // Update power consumption metrics
    state.power.bus_current = state.rotor.total_power_watt / state.power.bus_voltage;
    state.power.energy_joule += state.rotor.total_power_watt * dt;
}
