#include "modules/mission_control.h"

#include <algorithm>
#include <cmath>

#include "core/simulation_state.h"

namespace {
constexpr double kTakeoffAltitudeGain = 0.8;
constexpr double kMaximumClimbSpeedMps = 0.7;
constexpr double kMaximumDescentSpeedMps = 0.5;
constexpr double kMinimumLandingSpeedMps = 0.12;

void stopVirtualMotors(SimulationState& state)
{
    state.motor_commands.omega_rad_s.fill(0.0);
    state.motor_commands.throttle_0_1.fill(0.0);
    state.pilot.desired_velocity_ned = {0.0, 0.0, 0.0};
}

void transition(SimulationState& state, SimulationState::FlightPhase phase,
                const char* status)
{
    state.mission.phase = phase;
    state.mission.status = status;
    ++state.mission.transition_count;
}
}

void MissionControlModule::initialize(SimulationState& state)
{
    state.mission = SimulationState::MissionState{};
    stopVirtualMotors(state);
}

void MissionControlModule::update(double dt, SimulationState& state)
{
    auto& mission = state.mission;
    mission.vertical_velocity_override = false;
    mission.vertical_velocity_down_mps = 0.0;

    if (!std::isfinite(dt) || dt <= 0.0) {
        transition(state, SimulationState::FlightPhase::EmergencyStopped,
                   "Mission stopped: invalid simulation timing.");
        state.pilot.enabled = false;
        stopVirtualMotors(state);
        return;
    }

    const double altitude_m = std::max(0.0, -state.physics.position.z);
    const double vertical_speed_mps = state.physics.velocity.z;

    if (mission.phase == SimulationState::FlightPhase::EmergencyStopped) {
        state.pilot.enabled = false;
        stopVirtualMotors(state);
        return;
    }

    if (mission.landing_requested) {
        mission.landing_requested = false;
        mission.takeoff_requested = false;
        if (mission.phase == SimulationState::FlightPhase::TakingOff ||
            mission.phase == SimulationState::FlightPhase::Flying) {
            transition(state, SimulationState::FlightPhase::Landing,
                       "Landing: descending under mission control.");
        }
    }

    switch (mission.phase) {
    case SimulationState::FlightPhase::Grounded:
        stopVirtualMotors(state);
        if (mission.takeoff_requested && state.pilot.enabled) {
            mission.takeoff_requested = false;
            transition(state, SimulationState::FlightPhase::TakingOff,
                       "Takeoff: climbing to the declared hover altitude.");
        }
        break;

    case SimulationState::FlightPhase::TakingOff: {
        mission.vertical_velocity_override = true;
        const double altitude_error = mission.takeoff_altitude_m - altitude_m;
        mission.vertical_velocity_down_mps = -std::clamp(
            kTakeoffAltitudeGain * altitude_error,
            -kMaximumDescentSpeedMps, kMaximumClimbSpeedMps);
        if (std::abs(altitude_error) <= mission.altitude_tolerance_m &&
            std::abs(vertical_speed_mps) <= mission.vertical_speed_tolerance_mps) {
            mission.vertical_velocity_down_mps = 0.0;
            transition(state, SimulationState::FlightPhase::Flying,
                       "Hover reached; joystick velocity control active.");
        }
        break;
    }

    case SimulationState::FlightPhase::Flying:
        mission.takeoff_requested = false;
        break;

    case SimulationState::FlightPhase::Landing:
        mission.vertical_velocity_override = true;
        mission.vertical_velocity_down_mps = std::clamp(
            kTakeoffAltitudeGain * altitude_m,
            kMinimumLandingSpeedMps, kMaximumDescentSpeedMps);
        if (altitude_m <= 0.01 &&
            std::abs(vertical_speed_mps) <= mission.vertical_speed_tolerance_mps) {
            transition(state, SimulationState::FlightPhase::Grounded,
                       "Landing complete; aircraft grounded and motors stopped.");
            state.pilot.enabled = false;
            state.pilot.reset_controller = true;
            stopVirtualMotors(state);
        }
        break;

    case SimulationState::FlightPhase::EmergencyStopped:
        break;
    }
}
