#include "modules/quadcopter_dynamics.h"

#include <cmath>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "attitude/euler.h"
#include "attitude/quaternion.h"
#include "attitude/attitude_utils.h"
#include "core/simulation_state.h"

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMaxPhysicsStepS = 0.0025;
constexpr double kMaxFrameStepS = 0.25;

glm::mat4 renderFromNed()
{
    // NED (north, east, down) -> renderer (right/east, up, back/-north).
    glm::mat4 transform(1.0f);
    transform[0][0] = 0.0f;
    transform[1][0] = 1.0f;
    transform[2][0] = 0.0f;
    transform[0][1] = 0.0f;
    transform[1][1] = 0.0f;
    transform[2][1] = -1.0f;
    transform[0][2] = -1.0f;
    transform[1][2] = 0.0f;
    transform[2][2] = 0.0f;
    return transform;
}

/**
 * @brief Compute hover throttle for a quadcopter
 * @param mass Vehicle mass (kg)
 * @param gravity Gravitational acceleration (m/s²)
 * @param num_rotors Number of rotors
 * @return Hover thrust per rotor (N)
 */
double computeHoverThrust(double mass, double gravity, int num_rotors) {
    return (mass * gravity) / num_rotors;
}

/**
 * @brief Convert throttle [0, 1] to rotor angular velocity (rad/s)
 * @param throttle Throttle command [0, 1]
 * @param thrust_coeff Thrust coefficient
 * @return Angular velocity (rad/s)
 */
double throttleToOmega(double throttle, double thrust_coeff) {
    if (throttle <= 0.0 || thrust_coeff <= 0.0) return 0.0;
    // T = k_t * omega^2  =>  omega = sqrt(T / k_t)
    // Assume max throttle = 1.0 corresponds to 4x hover thrust
    double max_thrust = 4.0 * thrust_coeff * (500.0 * 500.0); // ~500 rad/s max
    double thrust = throttle * max_thrust;
    return std::sqrt(thrust / thrust_coeff);
}

}  // namespace

void QuadcopterDynamicsModule::initialize(SimulationState& state) {
    // Initialize vehicle configuration from simulation state
    vehicle_config_.rotor_count = 4;
    vehicle_config_.mass = state.vehicle_config.mass;
    vehicle_config_.gravity = state.vehicle_config.gravity;

    // Inertia tensor (diagonal, assuming symmetry)
    vehicle_config_.inertia[0][0] = state.vehicle_config.Ixx;
    vehicle_config_.inertia[0][1] = 0.0;
    vehicle_config_.inertia[0][2] = 0.0;

    vehicle_config_.inertia[1][0] = 0.0;
    vehicle_config_.inertia[1][1] = state.vehicle_config.Iyy;
    vehicle_config_.inertia[1][2] = 0.0;

    vehicle_config_.inertia[2][0] = 0.0;
    vehicle_config_.inertia[2][1] = 0.0;
    vehicle_config_.inertia[2][2] = state.vehicle_config.Izz;

    // Inertia inverse (for diagonal matrix, just invert diagonal elements)
    vehicle_config_.inertia_inv[0][0] = 1.0 / state.vehicle_config.Ixx;
    vehicle_config_.inertia_inv[0][1] = 0.0;
    vehicle_config_.inertia_inv[0][2] = 0.0;

    vehicle_config_.inertia_inv[1][0] = 0.0;
    vehicle_config_.inertia_inv[1][1] = 1.0 / state.vehicle_config.Iyy;
    vehicle_config_.inertia_inv[1][2] = 0.0;

    vehicle_config_.inertia_inv[2][0] = 0.0;
    vehicle_config_.inertia_inv[2][1] = 0.0;
    vehicle_config_.inertia_inv[2][2] = 1.0 / state.vehicle_config.Izz;

    // Setup rotor configuration (X-frame quadcopter)
    setupRotorConfiguration();

    // Initialize physics state
    std::memset(&physics_state_, 0, sizeof(physics_state_));

    // Start at origin, hovering
    physics_state_.position[0] = 0.0;
    physics_state_.position[1] = 0.0;
    physics_state_.position[2] = 0.0;

    physics_state_.velocity[0] = 0.0;
    physics_state_.velocity[1] = 0.0;
    physics_state_.velocity[2] = 0.0;

    // Identity quaternion (no rotation)
    physics_state_.quaternion[0] = 1.0;  // w
    physics_state_.quaternion[1] = 0.0;  // x
    physics_state_.quaternion[2] = 0.0;  // y
    physics_state_.quaternion[3] = 0.0;  // z

    physics_state_.angular_rate[0] = 0.0;
    physics_state_.angular_rate[1] = 0.0;
    physics_state_.angular_rate[2] = 0.0;

    // Setup vehicle model
    vehicle_model_.config = &vehicle_config_;
    vehicle_model_.state = physics_state_;

    // Initialize motor commands to hover
    double hover_thrust = computeHoverThrust(vehicle_config_.mass, vehicle_config_.gravity, 4);
    double hover_omega = std::sqrt(hover_thrust / state.rotor_config.thrust_coefficient);

    for (int i = 0; i < 4; ++i) {
        state.motor_commands.omega_rad_s[i] = hover_omega;
        state.motor_commands.throttle_0_1[i] = 0.5;  // 50% throttle for hover
    }

    // Copy initial state to simulation
    copyStateToSim(physics_state_, state);
    state.physics.integration_valid = true;
    state.physics.last_result = static_cast<int>(DM_OK);
    state.physics.accepted_steps = 0;
    state.physics.rejected_steps = 0;
}

void QuadcopterDynamicsModule::setupRotorConfiguration() {
    // X-frame quadcopter configuration (45° from body axes)
    // Front-right, front-left, back-left, back-right
    // Rotor 0: Front-right (+X, +Y), CW
    // Rotor 1: Front-left  (+X, -Y), CCW
    // Rotor 2: Back-left   (-X, -Y), CW
    // Rotor 3: Back-right  (-X, +Y), CCW

    const double arm_length = 0.225;  // 225mm arms (450mm wheelbase)
    const double diag = arm_length / std::sqrt(2.0);

    // Rotor 0: Front-right
    vehicle_config_.rotors[0].position_body[0] = diag;
    vehicle_config_.rotors[0].position_body[1] = diag;
    vehicle_config_.rotors[0].position_body[2] = 0.0;
    vehicle_config_.rotors[0].axis_body[0] = 0.0;
    vehicle_config_.rotors[0].axis_body[1] = 0.0;
    vehicle_config_.rotors[0].axis_body[2] = -1.0;
    vehicle_config_.rotors[0].direction = 1.0;  // CW
    vehicle_config_.rotors[0].thrust_coeff = 1.2e-6;
    vehicle_config_.rotors[0].torque_coeff = 2.5e-8;

    // Rotor 1: Front-left
    vehicle_config_.rotors[1].position_body[0] = diag;
    vehicle_config_.rotors[1].position_body[1] = -diag;
    vehicle_config_.rotors[1].position_body[2] = 0.0;
    vehicle_config_.rotors[1].axis_body[0] = 0.0;
    vehicle_config_.rotors[1].axis_body[1] = 0.0;
    vehicle_config_.rotors[1].axis_body[2] = -1.0;
    vehicle_config_.rotors[1].direction = -1.0;  // CCW
    vehicle_config_.rotors[1].thrust_coeff = 1.2e-6;
    vehicle_config_.rotors[1].torque_coeff = 2.5e-8;

    // Rotor 2: Back-left
    vehicle_config_.rotors[2].position_body[0] = -diag;
    vehicle_config_.rotors[2].position_body[1] = -diag;
    vehicle_config_.rotors[2].position_body[2] = 0.0;
    vehicle_config_.rotors[2].axis_body[0] = 0.0;
    vehicle_config_.rotors[2].axis_body[1] = 0.0;
    vehicle_config_.rotors[2].axis_body[2] = -1.0;
    vehicle_config_.rotors[2].direction = 1.0;  // CW
    vehicle_config_.rotors[2].thrust_coeff = 1.2e-6;
    vehicle_config_.rotors[2].torque_coeff = 2.5e-8;

    // Rotor 3: Back-right
    vehicle_config_.rotors[3].position_body[0] = -diag;
    vehicle_config_.rotors[3].position_body[1] = diag;
    vehicle_config_.rotors[3].position_body[2] = 0.0;
    vehicle_config_.rotors[3].axis_body[0] = 0.0;
    vehicle_config_.rotors[3].axis_body[1] = 0.0;
    vehicle_config_.rotors[3].axis_body[2] = -1.0;
    vehicle_config_.rotors[3].direction = -1.0;  // CCW
    vehicle_config_.rotors[3].thrust_coeff = 1.2e-6;
    vehicle_config_.rotors[3].torque_coeff = 2.5e-8;
}

void QuadcopterDynamicsModule::update(double dt, SimulationState& state) {
    if (!std::isfinite(dt) || dt <= 0.0 || dt > kMaxFrameStepS) {
        state.physics.last_result = static_cast<int>(DM_INVALID_ARGUMENT);
        state.physics.integration_valid = false;
        ++state.physics.rejected_steps;
        state.control.paused = true;
        return;
    }

    // Copy simulation state to physics state
    copyStateFromSim(state, physics_state_);

    // Prepare motor speeds array
    double rotor_omega[DM_MAX_ROTORS] = {0};
    for (size_t i = 0; i < 4; ++i) {
        rotor_omega[i] = state.motor_commands.omega_rad_s[i];
    }

    const int substep_count = static_cast<int>(std::ceil(dt / kMaxPhysicsStepS));
    const double substep_dt = dt / static_cast<double>(substep_count);
    vehicle_model_.state = physics_state_;
    for (int substep = 0; substep < substep_count; ++substep) {
        const dm_result_t result =
            dm_vehicle_step_rk4_checked(&vehicle_model_, rotor_omega, substep_dt);
        state.physics.last_result = static_cast<int>(result);
        state.physics.integration_valid = result == DM_OK;
        if (result != DM_OK) {
            ++state.physics.rejected_steps;
            state.control.paused = true;
            return;
        }
        ++state.physics.accepted_steps;
    }

    physics_state_ = vehicle_model_.state;

    // Copy physics state back to simulation
    copyStateToSim(physics_state_, state);

    // Update rotor telemetry
    updateRotorTelemetry(state);
}

void QuadcopterDynamicsModule::copyStateToSim(const dm_state_t& dm_state, SimulationState& state) {
    // Position and velocity
    state.physics.position = glm::dvec3(dm_state.position[0], dm_state.position[1], dm_state.position[2]);
    state.physics.velocity = glm::dvec3(dm_state.velocity[0], dm_state.velocity[1], dm_state.velocity[2]);

    // Quaternion
    state.quaternion[0] = dm_state.quaternion[0];  // w
    state.quaternion[1] = dm_state.quaternion[1];  // x
    state.quaternion[2] = dm_state.quaternion[2];  // y
    state.quaternion[3] = dm_state.quaternion[3];  // z

    // Angular rates (body frame)
    state.angular_rate_deg_per_sec = glm::dvec3(
        rad2deg(dm_state.angular_rate[0]),
        rad2deg(dm_state.angular_rate[1]),
        rad2deg(dm_state.angular_rate[2])
    );

    // Convert quaternion to Euler angles
    double q[4] = {dm_state.quaternion[0], dm_state.quaternion[1],
                   dm_state.quaternion[2], dm_state.quaternion[3]};
    double roll, pitch, yaw;
    quaternion_to_euler(q, &roll, &pitch, &yaw);

    state.euler.roll = roll;
    state.euler.pitch = pitch;
    state.euler.yaw = yaw;
    state.euler.order = EULER_ZYX;

    // Keep physics in NED and adapt only at the renderer boundary.
    double dcm[3][3];
    quaternion_to_dcm(q, dcm);

    glm::mat4 ned_from_body(1.0f);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            ned_from_body[col][row] = static_cast<float>(dcm[row][col]);
        }
    }
    const glm::mat4 render_from_ned = renderFromNed();
    const glm::vec4 position_ned(static_cast<float>(dm_state.position[0]),
                                 static_cast<float>(dm_state.position[1]),
                                 static_cast<float>(dm_state.position[2]),
                                 1.0f);
    const glm::vec3 position_render = glm::vec3(render_from_ned * position_ned);
    const glm::mat4 translation = glm::translate(
        glm::mat4(1.0f), position_render);
    state.model_matrix = translation * render_from_ned * ned_from_body;
}

void QuadcopterDynamicsModule::copyStateFromSim(const SimulationState& state, dm_state_t& dm_state) {
    // Position and velocity
    dm_state.position[0] = state.physics.position.x;
    dm_state.position[1] = state.physics.position.y;
    dm_state.position[2] = state.physics.position.z;

    dm_state.velocity[0] = state.physics.velocity.x;
    dm_state.velocity[1] = state.physics.velocity.y;
    dm_state.velocity[2] = state.physics.velocity.z;

    // Quaternion
    dm_state.quaternion[0] = state.quaternion[0];
    dm_state.quaternion[1] = state.quaternion[1];
    dm_state.quaternion[2] = state.quaternion[2];
    dm_state.quaternion[3] = state.quaternion[3];

    // Angular rates
    dm_state.angular_rate[0] = deg2rad(state.angular_rate_deg_per_sec.x);
    dm_state.angular_rate[1] = deg2rad(state.angular_rate_deg_per_sec.y);
    dm_state.angular_rate[2] = deg2rad(state.angular_rate_deg_per_sec.z);
}

void QuadcopterDynamicsModule::updateRotorTelemetry(SimulationState& state) {
    // Update rotor telemetry from cached values in vehicle_model
    double total_thrust = 0.0;
    double total_power = 0.0;

    for (size_t i = 0; i < 4; ++i) {
        const auto& rotor = vehicle_model_.rotor_cache[i];
        double omega = rotor.omega;

        // Convert rad/s to RPM
        state.rotor.rpm[i] = omega * 60.0 / (2.0 * kPi);

        // Thrust and torque
        state.rotor.thrust_newton[i] = rotor.thrust;
        state.rotor.torque_newton_metre[i] = rotor.torque;

        total_thrust += rotor.thrust;

        // Power = Torque * Omega
        total_power += rotor.torque * omega;
    }

    state.rotor.total_thrust_newton = total_thrust;
    state.rotor.total_power_watt = total_power;
}
