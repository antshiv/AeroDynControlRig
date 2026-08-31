#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "core/simulation_state.h"
#include "modules/flight_control.h"
#include "modules/mission_control.h"
#include "modules/nrf_hil.h"
#include "modules/quadcopter_dynamics.h"

namespace {
constexpr double kStepS = 0.01;

const char* phaseName(SimulationState::FlightPhase phase)
{
    switch (phase) {
    case SimulationState::FlightPhase::Grounded: return "grounded";
    case SimulationState::FlightPhase::TakingOff: return "taking_off";
    case SimulationState::FlightPhase::Flying: return "flying";
    case SimulationState::FlightPhase::Landing: return "landing";
    case SimulationState::FlightPhase::EmergencyStopped: return "emergency_stopped";
    }
    return "unknown";
}

bool motorsStopped(const SimulationState& state)
{
    return std::all_of(state.motor_commands.omega_rad_s.begin(),
                       state.motor_commands.omega_rad_s.end(),
                       [](double speed) { return speed == 0.0; });
}

struct MissionRig {
    SimulationState state;
    MissionControlModule mission;
    FlightControlModule host_guidance;
    NrfHilModule hil;
    QuadcopterDynamicsModule plant;
    std::ofstream& evidence;
    int run;
    std::string stage{"grounded"};

    MissionRig(const std::string& device, std::ofstream& output, int run_index)
        : evidence(output), run(run_index)
    {
        setenv("ASR_FC_HIL_DEVICE", device.c_str(), 1);
        state.joystick.connected = true;
        state.joystick.standardized_mapping = true;
        state.control.paused = false;
        mission.initialize(state);
        host_guidance.initialize(state);
        hil.initialize(state);
        plant.initialize(state);
        state.execution.selected_mode =
            SimulationState::ExecutionMode::Nrf5340Hil;
    }

    bool ready() const
    {
        return state.execution.nrf_hil_available &&
               state.aircraft_contract.loaded &&
               state.pilot.controller_valid;
    }

    void record()
    {
        evidence << run << ',' << stage << ','
                 << state.execution.nrf_hil_session_id << ','
                 << state.execution.nrf_hil_sequence << ','
                 << std::fixed << std::setprecision(6)
                 << state.time_seconds << ',' << state.last_dt << ','
                 << state.execution.nrf_hil_device_timestamp_us << ','
                 << state.execution.nrf_hil_execution_us << ','
                 << state.execution.nrf_hil_round_trip_us << ','
                 << static_cast<unsigned>(state.execution.nrf_hil_flight_state) << ','
                 << state.execution.nrf_hil_fault_flags << ','
                 << phaseName(state.mission.phase) << ','
                 << state.physics.position.x << ','
                 << state.physics.position.y << ','
                 << state.physics.position.z << ','
                 << state.physics.velocity.x << ','
                 << state.physics.velocity.y << ','
                 << state.physics.velocity.z << ','
                 << state.euler.roll << ','
                 << state.euler.pitch << ','
                 << state.euler.yaw;
        for (double speed : state.motor_commands.omega_rad_s) {
            evidence << ',' << speed;
        }
        evidence << '\n';
    }

    bool step()
    {
        state.last_dt = kStepS;
        state.time_seconds += kStepS;
        mission.update(kStepS, state);
        host_guidance.update(kStepS, state);
        hil.update(kStepS, state);
        if (!state.control.paused) {
            plant.update(kStepS, state);
        }
        record();
        const bool accepted = !state.control.paused &&
            state.execution.nrf_hil_failures == 0u &&
            state.execution.nrf_hil_fault_flags == 0u;
        if (!accepted) {
            std::cerr << "run " << run << " stage " << stage
                      << " failed at sequence "
                      << state.execution.nrf_hil_sequence << ": "
                      << state.execution.nrf_hil_status << '\n';
        }
        return accepted;
    }

    bool runUntil(SimulationState::FlightPhase wanted, int maximum_steps)
    {
        for (int index = 0; index < maximum_steps; ++index) {
            if (!step()) return false;
            if (state.mission.phase == wanted) return true;
        }
        return false;
    }
};

bool runMission(const std::string& device, std::ofstream& evidence, int run)
{
    MissionRig rig(device, evidence, run);
    if (!rig.ready()) {
        std::cerr << "run " << run << ": HIL rig unavailable: "
                  << rig.state.execution.nrf_hil_status << '\n';
        return false;
    }

    for (int index = 0; index < 3; ++index) {
        if (!rig.step()) return false;
    }
    if (rig.state.mission.phase != SimulationState::FlightPhase::Grounded ||
        !motorsStopped(rig.state)) {
        std::cerr << "run " << run << ": grounded gate failed\n";
        return false;
    }

    rig.stage = "takeoff";
    rig.state.pilot.enabled = true;
    rig.state.mission.takeoff_requested = true;
    if (!rig.runUntil(SimulationState::FlightPhase::Flying, 2000)) {
        std::cerr << "run " << run << ": takeoff/hover gate failed: "
                  << rig.state.execution.nrf_hil_status << '\n';
        return false;
    }
    const double hover_altitude = -rig.state.physics.position.z;
    if (std::abs(hover_altitude - rig.state.mission.takeoff_altitude_m) > 0.15 ||
        std::abs(rig.state.physics.velocity.z) > 0.15) {
        std::cerr << "run " << run << ": hover did not settle\n";
        return false;
    }

    rig.stage = "joystick_flight";
    const double north_before = rig.state.physics.position.x;
    const double yaw_before = rig.state.euler.yaw;
    rig.state.joystick.axes[0] = 0.18f;
    rig.state.joystick.axes[3] = -0.40f;
    for (int index = 0; index < 120; ++index) {
        if (!rig.step()) return false;
    }
    rig.state.joystick.axes[0] = 0.0f;
    rig.state.joystick.axes[3] = 0.0f;
    for (int index = 0; index < 80; ++index) {
        if (!rig.step()) return false;
    }
    if (rig.state.physics.position.x <= north_before + 0.02 ||
        std::abs(rig.state.euler.yaw - yaw_before) <= 0.005) {
        std::cerr << "run " << run << ": translation/yaw gate failed\n";
        return false;
    }

    rig.stage = "landing";
    rig.state.mission.landing_requested = true;
    if (!rig.runUntil(SimulationState::FlightPhase::Grounded, 2000)) {
        std::cerr << "run " << run << ": landing gate failed\n";
        return false;
    }
    rig.stage = "grounded_complete";
    if (!rig.step() || !motorsStopped(rig.state) || rig.state.pilot.enabled ||
        rig.state.physics.position.z != 0.0) {
        std::cerr << "run " << run << ": final grounded gate failed\n";
        return false;
    }
    std::cout << "run " << run << " passed: "
              << rig.state.execution.nrf_hil_sequence << " checked MCU steps, "
              << rig.state.physics.accepted_steps << " RK4 substeps\n";
    return true;
}
}  // namespace

int main(int argc, char** argv)
{
    if (argc < 3 || argc > 4) {
        std::cerr << "usage: aerodyn_nrf_hil_mission <serial-device> <evidence.csv> [runs]\n";
        return 2;
    }
    const int runs = argc == 4 ? std::atoi(argv[3]) : 3;
    if (runs <= 0) {
        std::cerr << "runs must be positive\n";
        return 2;
    }

    const std::filesystem::path output_path(argv[2]);
    if (output_path.has_parent_path()) {
        std::filesystem::create_directories(output_path.parent_path());
    }
    std::ofstream evidence(output_path);
    if (!evidence) {
        std::cerr << "cannot create evidence file: " << output_path << '\n';
        return 1;
    }
    evidence << "run,stage,session_id,sequence,sim_time_s,sim_dt_s,device_timestamp_us,"
                "mcu_execution_us,round_trip_us,flight_state,fault_flags,phase,"
                "north_m,east_m,down_m,north_mps,east_mps,down_mps,"
                "roll_rad,pitch_rad,yaw_rad,"
                "motor_1_rad_s,motor_2_rad_s,motor_3_rad_s,motor_4_rad_s\n";

    for (int run = 1; run <= runs; ++run) {
        if (!runMission(argv[1], evidence, run)) return 1;
    }
    std::cout << "HIL mission evidence written to " << output_path << '\n';
    return 0;
}
