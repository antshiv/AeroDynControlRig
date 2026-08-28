#include <cmath>
#include <cstdio>

#include "core/simulation_state.h"
#include "modules/flight_control.h"
#include "modules/mission_control.h"
#include "modules/quadcopter_dynamics.h"

namespace {
constexpr double kStepS = 0.02;

struct Rig {
    SimulationState state;
    MissionControlModule mission;
    FlightControlModule controller;
    QuadcopterDynamicsModule plant;

    Rig()
    {
        state.joystick.connected = true;
        state.joystick.standardized_mapping = true;
        mission.initialize(state);
        controller.initialize(state);
        plant.initialize(state);
    }

    void step()
    {
        mission.update(kStepS, state);
        controller.update(kStepS, state);
        plant.update(kStepS, state);
    }

    bool runUntil(SimulationState::FlightPhase phase, double timeout_s)
    {
        const int steps = static_cast<int>(std::lround(timeout_s / kStepS));
        for (int index = 0; index < steps; ++index) {
            step();
            if (state.mission.phase == phase) {
                return true;
            }
        }
        return false;
    }
};

int failures = 0;

void expect(const char* name, bool condition)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}
}

int main()
{
    {
        Rig rig;
        expect("reset starts grounded",
               rig.state.mission.phase == SimulationState::FlightPhase::Grounded);
        expect("reset motors are stopped", rig.state.motor_commands.omega_rad_s[0] == 0.0);

        rig.state.pilot.enabled = true;
        rig.state.mission.takeoff_requested = true;
        expect("takeoff reaches flight phase",
               rig.runUntil(SimulationState::FlightPhase::Flying, 15.0));
        expect("takeoff reaches declared altitude",
               std::abs(-rig.state.physics.position.z -
                        rig.state.mission.takeoff_altitude_m) < 0.15);
        expect("takeoff settles vertical speed",
               std::abs(rig.state.physics.velocity.z) < 0.15);

        rig.state.joystick.axes[3] = -0.5f;
        for (int index = 0; index < 50; ++index) {
            rig.step();
        }
        rig.state.joystick.axes[3] = 0.0f;
        expect("joystick flight advances north", rig.state.physics.position.x > 0.02);

        rig.state.mission.landing_requested = true;
        expect("landing returns to grounded phase",
               rig.runUntil(SimulationState::FlightPhase::Grounded, 15.0));
        expect("landing reaches ground plane", rig.state.physics.position.z == 0.0);
        expect("landing stops vertical motion", rig.state.physics.velocity.z == 0.0);
        expect("landing stops motors", rig.state.motor_commands.omega_rad_s[0] == 0.0);
        expect("landing disables pilot", !rig.state.pilot.enabled);
        expect("mission records all four transitions",
               rig.state.mission.transition_count == 4u);
    }
    {
        Rig rig;
        rig.state.pilot.enabled = true;
        rig.state.mission.phase = SimulationState::FlightPhase::EmergencyStopped;
        rig.state.motor_commands.omega_rad_s.fill(400.0);
        rig.step();
        expect("emergency stop disables pilot", !rig.state.pilot.enabled);
        expect("emergency stop clears motors",
               rig.state.motor_commands.omega_rad_s[0] == 0.0);
    }

    if (failures != 0) {
        std::fprintf(stderr, "%d mission sequence check(s) failed\n", failures);
        return 1;
    }
    std::puts("Takeoff -> flight -> landing contract: all checks passed");
    return 0;
}
