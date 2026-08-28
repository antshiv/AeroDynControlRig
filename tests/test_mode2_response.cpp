#include <array>
#include <cmath>
#include <cstdio>

#include "core/simulation_state.h"
#include "modules/flight_control.h"
#include "modules/quadcopter_dynamics.h"

namespace {
constexpr double kStepS = 0.02;

struct Rig {
    SimulationState state;
    FlightControlModule controller;
    QuadcopterDynamicsModule plant;

    Rig()
    {
        state.joystick.connected = true;
        state.joystick.standardized_mapping = true;
        controller.initialize(state);
        plant.initialize(state);
        state.pilot.enabled = true;
        state.mission.phase = SimulationState::FlightPhase::Flying;
    }

    void run(double seconds, const std::array<float, 4>& sticks)
    {
        state.joystick.axes[0] = sticks[0];
        state.joystick.axes[1] = sticks[1];
        state.joystick.axes[2] = sticks[2];
        state.joystick.axes[3] = sticks[3];
        const int steps = static_cast<int>(std::lround(seconds / kStepS));
        for (int step = 0; step < steps; ++step) {
            controller.update(kStepS, state);
            plant.update(kStepS, state);
        }
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

void report(const char* name, const SimulationState& state)
{
    std::printf(
        "%s: position=[%+.4f %+.4f %+.4f] velocity=[%+.4f %+.4f %+.4f] "
        "euler_deg=[%+.3f %+.3f %+.3f]\n",
        name,
        state.physics.position.x, state.physics.position.y, state.physics.position.z,
        state.physics.velocity.x, state.physics.velocity.y, state.physics.velocity.z,
        state.euler.roll * 180.0 / 3.14159265358979323846,
        state.euler.pitch * 180.0 / 3.14159265358979323846,
        state.euler.yaw * 180.0 / 3.14159265358979323846);
}
}

int main()
{
    {
        Rig rig;
        rig.run(4.0, {0.0f, 0.0f, 0.0f, 0.0f});
        report("hover", rig.state);
        expect("hover altitude remains bounded", std::abs(rig.state.physics.position.z) < 1.0e-5);
        expect("hover attitude remains level", std::abs(rig.state.euler.roll) < 1.0e-5 &&
                                                std::abs(rig.state.euler.pitch) < 1.0e-5);
    }
    {
        Rig rig;
        rig.run(1.0, {0.0f, -0.7f, 0.0f, 0.0f});
        report("climb", rig.state);
        expect("left stick up climbs", rig.state.physics.position.z < -0.05);
    }
    {
        Rig rig;
        rig.run(1.0, {0.0f, 0.0f, 0.0f, -0.6f});
        report("forward", rig.state);
        expect("right stick up pitches nose down", rig.state.euler.pitch < -0.01);
        expect("right stick up accelerates north", rig.state.physics.velocity.x > 0.01);
        rig.run(2.0, {0.0f, 0.0f, 0.0f, 0.0f});
        report("forward release", rig.state);
        expect("pitch returns toward level", std::abs(rig.state.euler.pitch) < 0.10);
        expect("training assist brakes forward velocity",
               std::abs(rig.state.physics.velocity.x) < 0.10);
    }
    {
        Rig rig;
        rig.run(1.0, {0.0f, 0.0f, 0.6f, 0.0f});
        report("right", rig.state);
        expect("right stick right rolls right", rig.state.euler.roll > 0.01);
        expect("right stick right accelerates east", rig.state.physics.velocity.y > 0.01);
    }
    {
        Rig rig;
        rig.run(1.0, {0.6f, 0.0f, 0.0f, 0.0f});
        report("yaw", rig.state);
        expect("left stick right yaws right", rig.state.euler.yaw > 0.01);
        expect("yaw remains approximately in place",
               std::hypot(rig.state.physics.position.x,
                          rig.state.physics.position.y) < 0.01);
    }

    if (failures != 0) {
        std::fprintf(stderr, "%d Mode 2 response check(s) failed\n", failures);
        return 1;
    }
    std::puts("Mode 2 response contract: all checks passed");
    return 0;
}
