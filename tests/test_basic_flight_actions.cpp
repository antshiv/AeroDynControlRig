#include <cassert>
#include <iostream>

#include "core/simulation_state.h"
#include "input/basic_flight_actions.h"

int main()
{
    SimulationState state;
    state.pilot.controller_valid = true;
    state.physics.integration_valid = true;

    applyBasicFlightAction(BasicFlightAction::Takeoff, false, state);
    assert(!state.pilot.enabled);
    assert(!state.mission.takeoff_requested);

    applyBasicFlightAction(BasicFlightAction::Takeoff, true, state);
    assert(state.pilot.enabled);
    assert(state.mission.takeoff_requested);
    assert(!state.control.paused);

    state.mission.takeoff_requested = false;
    state.mission.phase = SimulationState::FlightPhase::Flying;
    applyBasicFlightAction(BasicFlightAction::Takeoff, true, state);
    assert(!state.mission.takeoff_requested);

    applyBasicFlightAction(BasicFlightAction::TogglePause, true, state);
    assert(state.control.paused);
    applyBasicFlightAction(BasicFlightAction::TogglePause, true, state);
    assert(!state.control.paused);

    applyBasicFlightAction(BasicFlightAction::Land, true, state);
    assert(state.mission.landing_requested);

    applyBasicFlightAction(BasicFlightAction::FitView, true, state);
    assert(state.control.camera_fit_requested);
    applyBasicFlightAction(BasicFlightAction::Reset, true, state);
    assert(state.control.reset_requested);

    state.motor_commands.omega_rad_s.fill(500.0);
    state.motor_commands.throttle_0_1.fill(0.5);
    applyBasicFlightAction(BasicFlightAction::EmergencyStop, true, state);
    assert(state.mission.phase == SimulationState::FlightPhase::EmergencyStopped);
    assert(!state.pilot.enabled);
    assert(state.control.paused);
    for (double omega : state.motor_commands.omega_rad_s) assert(omega == 0.0);
    for (double throttle : state.motor_commands.throttle_0_1) assert(throttle == 0.0);

    SimulationState grounded;
    applyBasicFlightAction(BasicFlightAction::Land, true, grounded);
    assert(!grounded.mission.landing_requested);
    applyBasicFlightAction(BasicFlightAction::TogglePause, true, grounded);
    assert(grounded.control.paused);

    std::cout << "PASS: stable basic joystick action contract\n";
    return 0;
}
