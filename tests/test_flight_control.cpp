#include <cassert>
#include <cmath>
#include <iostream>

#include "core/simulation_state.h"
#include "modules/flight_control.h"
#include "modules/quadcopter_dynamics.h"

int main()
{
    SimulationState state;
    state.joystick.connected = true;
    state.pilot.enabled = true;

    FlightControlModule controller;
    controller.initialize(state);
    assert(state.pilot.controller_valid);
    QuadcopterDynamicsModule plant;
    plant.initialize(state);

    controller.update(0.02, state);
    assert(state.pilot.accepted_updates == 0u);
    controller.update(0.02, state);
    plant.update(0.04, state);
    assert(state.pilot.accepted_updates == 1u);
    for (double omega : state.motor_commands.omega_rad_s) {
        assert(std::isfinite(omega));
        assert(omega > 0.0);
        assert(omega <= state.rotor_config.maximum_speed_rad_s ||
               state.rotor_config.maximum_speed_rad_s == 0.0);
    }

    state.joystick.axes[2] = 0.5f;
    for (int step = 0; step < 10; ++step) {
        controller.update(0.02, state);
        plant.update(0.02, state);
    }
    assert(state.pilot.accepted_updates == 6u);
    assert(std::abs(state.pilot.target_roll_rad) > 0.0);
    assert(state.motor_commands.omega_rad_s[0] !=
           state.motor_commands.omega_rad_s[1]);
    assert(std::abs(state.euler.roll) > 1e-5);
    assert(state.physics.integration_valid);

    state.pilot.roll_trim_rad = 2.0 * 3.14159265358979323846 / 180.0;
    state.pilot.pitch_trim_rad = -3.0 * 3.14159265358979323846 / 180.0;
    state.pilot.command_scale = 0.5;
    state.joystick.axes[2] = 0.0f;
    state.joystick.axes[3] = 0.0f;
    state.joystick.standardized_mapping = true;
    state.joystick.axes[4] = 1.0f;
    state.joystick.axes[5] = -1.0f;
    controller.update(0.04, state);
    assert(std::abs(state.pilot.target_roll_rad - state.pilot.roll_trim_rad) < 1e-12);
    assert(std::abs(state.pilot.target_pitch_rad - state.pilot.pitch_trim_rad) < 1e-12);
    assert(state.pilot.collective_thrust_n > -state.physics.mass * 9.81);

    controller.update(0.10, state);
    assert(!state.pilot.enabled);
    assert(state.pilot.rejected_updates == 1u);

    std::cout << "PASS: joystick command -> PID -> mixer contract\n";
    return 0;
}
