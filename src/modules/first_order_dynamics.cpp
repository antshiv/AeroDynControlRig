#include "modules/first_order_dynamics.h"

#include <cmath>
#include <algorithm>

#include "core/simulation_state.h"

namespace {
constexpr double kMinTimeConstant = 1e-3;
}

void FirstOrderDynamicsModule::initialize(SimulationState& state) {
    internal_state_ = 0.0;
    state.dynamics_state.input = state.dynamics_config.input_target;
    state.dynamics_state.output = 0.0;
    state.dynamics_state.internal_state = 0.0;
}

void FirstOrderDynamicsModule::update(double dt, SimulationState& state) {
    if (dt <= 0.0) {
        return;
    }

    time_constant_ = std::max(state.dynamics_config.time_constant, kMinTimeConstant);
    gain_ = state.dynamics_config.gain;

    double command = state.dynamics_config.input_target;
    if (state.dynamics_config.use_sine) {
        command = gain_ * std::sin(2.0 * M_PI * state.dynamics_config.sine_frequency_hz * state.time_seconds);
    }

    state.dynamics_state.input = command;

    double derivative = (-internal_state_ + gain_ * command) / time_constant_;
    internal_state_ += derivative * dt;

    state.dynamics_state.internal_state = internal_state_;
    state.dynamics_state.output = internal_state_;
}
