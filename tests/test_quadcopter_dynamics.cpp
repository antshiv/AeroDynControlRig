#include "core/simulation_state.h"
#include "modules/quadcopter_dynamics.h"

#include <cmath>
#include <cstdio>

namespace {

int failures = 0;

void expectNear(const char* name, double actual, double expected, double tolerance)
{
    if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance) {
        std::fprintf(stderr,
                     "FAIL %s: actual=%.12g expected=%.12g tolerance=%.3g\n",
                     name, actual, expected, tolerance);
        ++failures;
    }
}

void expectTrue(const char* name, bool condition)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL %s\n", name);
        ++failures;
    }
}

}  // namespace

int main()
{
    SimulationState state;
    QuadcopterDynamicsModule plant;
    plant.initialize(state);

    // Identity body attitude in NED maps to right/east, up/-down, back/-north.
    expectNear("render row0 col1", state.model_matrix[1][0], 1.0, 1e-7);
    expectNear("render row1 col2", state.model_matrix[2][1], -1.0, 1e-7);
    expectNear("render row2 col0", state.model_matrix[0][2], -1.0, 1e-7);

    for (int i = 0; i < 400; ++i) {
        plant.update(0.0025, state);
    }

    expectTrue("checked integration remains valid", state.physics.integration_valid);
    expectTrue("all hover steps accepted", state.physics.accepted_steps == 400U);
    expectTrue("no hover steps rejected", state.physics.rejected_steps == 0U);
    expectNear("hover north", state.physics.position.x, 0.0, 1e-9);
    expectNear("hover east", state.physics.position.y, 0.0, 1e-9);
    expectNear("hover down", state.physics.position.z, 0.0, 1e-8);
    expectNear("hover vertical velocity", state.physics.velocity.z, 0.0, 1e-8);

    const glm::dvec3 position_before_rejection = state.physics.position;
    plant.update(0.5, state);
    expectTrue("oversized frame step is rejected", !state.physics.integration_valid);
    expectTrue("rejection pauses simulation", state.control.paused);
    expectTrue("one step rejected", state.physics.rejected_steps == 1U);
    expectNear("rejected north unchanged", state.physics.position.x,
               position_before_rejection.x, 0.0);
    expectNear("rejected east unchanged", state.physics.position.y,
               position_before_rejection.y, 0.0);
    expectNear("rejected down unchanged", state.physics.position.z,
               position_before_rejection.z, 0.0);

    if (failures != 0) {
        std::fprintf(stderr, "%d AeroDyn plant check(s) failed\n", failures);
        return 1;
    }

    std::puts("AeroDyn checked visual plant: all tests passed");
    return 0;
}
