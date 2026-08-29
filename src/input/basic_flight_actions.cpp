#include "input/basic_flight_actions.h"

#include "core/simulation_state.h"

namespace {
void recordAction(SimulationState& state, const char* action)
{
    ++state.joystick.action_sequence;
    state.joystick.last_action = action;
}
}

void applyBasicFlightAction(BasicFlightAction action, bool input_ready,
                            SimulationState& state)
{
    switch (action) {
    case BasicFlightAction::Takeoff:
        recordAction(state, "A: request takeoff");
        if (state.mission.phase != SimulationState::FlightPhase::Grounded) {
            state.pilot.status = "Takeoff ignored: aircraft is not grounded.";
            return;
        }
        if (!input_ready) {
            state.pilot.status =
                "Takeoff rejected: center the sticks and satisfy all readiness gates.";
            return;
        }
        state.pilot.enabled = true;
        state.pilot.reset_controller = true;
        state.pilot.target_yaw_rad = state.euler.yaw;
        state.mission.takeoff_requested = true;
        state.control.paused = false;
        state.pilot.status = "Takeoff requested; mission controller owns climb rate.";
        return;

    case BasicFlightAction::Land:
        recordAction(state, "B: request landing");
        if (state.mission.phase != SimulationState::FlightPhase::TakingOff &&
            state.mission.phase != SimulationState::FlightPhase::Flying) {
            state.pilot.status = "Landing ignored: aircraft is not airborne.";
            return;
        }
        state.mission.landing_requested = true;
        state.control.paused = false;
        state.pilot.status = "Landing requested; mission controller owns descent rate.";
        return;

    case BasicFlightAction::TogglePause:
        recordAction(state, "X: pause or resume simulation");
        if (!state.pilot.enabled ||
            state.mission.phase == SimulationState::FlightPhase::Grounded ||
            state.mission.phase == SimulationState::FlightPhase::EmergencyStopped) {
            state.pilot.status = "Pause/resume ignored: no active flight.";
            return;
        }
        state.control.paused = !state.control.paused;
        state.pilot.status = state.control.paused
            ? "Flight simulation paused; virtual command state retained."
            : "Flight simulation resumed.";
        return;

    case BasicFlightAction::FitView:
        recordAction(state, "Y: fit aircraft in view");
        state.control.camera_fit_requested = true;
        state.pilot.status = "Camera fit requested.";
        return;

    case BasicFlightAction::Reset:
        recordAction(state, "Start: reset aircraft and telemetry");
        state.control.reset_requested = true;
        return;

    case BasicFlightAction::EmergencyStop:
        recordAction(state, "Back: emergency stop");
        if (state.mission.phase != SimulationState::FlightPhase::EmergencyStopped) {
            state.mission.phase = SimulationState::FlightPhase::EmergencyStopped;
            ++state.mission.transition_count;
        }
        state.mission.status = "Emergency stop latched; reset required.";
        state.pilot.enabled = false;
        state.pilot.reset_controller = true;
        state.control.paused = true;
        state.motor_commands.omega_rad_s.fill(0.0);
        state.motor_commands.throttle_0_1.fill(0.0);
        state.pilot.status =
            "Emergency stop: simulation paused and virtual motors cleared.";
        return;
    }
}
