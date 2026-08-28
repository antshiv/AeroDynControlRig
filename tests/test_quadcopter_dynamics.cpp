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

    expectNear("published plant mass", state.physics.mass, 1.2, 1e-12);
    expectNear("published inertia xx", state.physics.inertia[0][0], 0.02, 1e-12);
    expectNear("published inertia yy", state.physics.inertia[1][1], 0.02, 1e-12);
    expectNear("published inertia zz", state.physics.inertia[2][2], 0.04, 1e-12);

    const auto& model = state.mathematical_model;
    const auto a = [&](std::size_t row, std::size_t column) {
        return model.A[row * model.kStateCount + column];
    };
    const auto b = [&](std::size_t row, std::size_t column) {
        return model.B[row * model.kInputCount + column];
    };
    expectTrue("hover linearization is valid", model.valid);
    expectNear("model mass", model.mass_kg, 1.2, 1e-12);
    expectNear("north position integrator", a(0u, 3u), 1.0, 0.0);
    expectNear("north pitch coupling", a(3u, 7u), -9.80665, 1e-12);
    expectNear("down thrust input", b(5u, 0u), -1.0 / 1.2, 1e-12);
    expectNear("roll torque input", b(9u, 1u), 1.0 / 0.02, 1e-12);
    expectTrue("hover rotor speed is finite", std::isfinite(model.hover_omega_rad_s));
    expectTrue("hover rotor speed is positive", model.hover_omega_rad_s > 0.0);

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
