#include "modules/quadcopter_dynamics.h"

#include <algorithm>
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
    if (!loadConfiguration(state)) {
        state.physics.integration_valid = false;
        state.control.paused = true;
        return;
    }

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
    double hover_thrust = computeHoverThrust(vehicle_config_.mass, vehicle_config_.gravity,
                                             vehicle_config_.rotor_count);

    for (std::size_t i = 0; i < vehicle_config_.rotor_count; ++i) {
        const double hover_omega =
            std::sqrt(hover_thrust / vehicle_config_.rotors[i].thrust_coeff);
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

bool QuadcopterDynamicsModule::loadConfiguration(SimulationState& state) {
    std::string error;
    if (!loadAircraftSpec(ASR_DEFAULT_AIRCRAFT_SPEC, aircraft_spec_, error) ||
        !loadControllerCoefficientBundle(ASR_DEFAULT_COEFFICIENT_BUNDLE,
                                         aircraft_spec_, coefficient_bundle_,
                                         error)) {
        state.aircraft_contract.error = error;
        state.aircraft_contract.loaded = false;
        return false;
    }
    if (aircraft_spec_.rotors.size() != 4u) {
        state.aircraft_contract.error =
            "the current AeroDyn renderer and telemetry path requires four rotors";
        state.aircraft_contract.loaded = false;
        return false;
    }

    std::memset(&vehicle_config_, 0, sizeof(vehicle_config_));
    vehicle_config_.rotor_count = aircraft_spec_.rotors.size();
    vehicle_config_.mass = aircraft_spec_.mass_kg;
    vehicle_config_.gravity = aircraft_spec_.gravity_m_s2;
    for (std::size_t row = 0; row < 3u; ++row) {
        for (std::size_t column = 0; column < 3u; ++column) {
            vehicle_config_.inertia[row][column] =
                aircraft_spec_.inertia[row][column];
            vehicle_config_.inertia_inv[row][column] =
                aircraft_spec_.inertia_inverse[row][column];
        }
    }
    for (std::size_t index = 0; index < aircraft_spec_.rotors.size(); ++index) {
        const AircraftRotorSpec& source = aircraft_spec_.rotors[index];
        dm_rotor_config_t& target = vehicle_config_.rotors[index];
        std::memcpy(target.position_body, source.position_body_m.data(),
                    sizeof(target.position_body));
        std::memcpy(target.axis_body, source.thrust_axis_body.data(),
                    sizeof(target.axis_body));
        target.direction = source.spin_direction;
        target.thrust_coeff = source.thrust_coefficient;
        target.torque_coeff = source.torque_coefficient;
    }
    if (dm_vehicle_config_validate(&vehicle_config_) != DM_OK) {
        state.aircraft_contract.error =
            "dynamic_models rejected the loaded aircraft specification";
        state.aircraft_contract.loaded = false;
        return false;
    }

    state.vehicle_config.mass = aircraft_spec_.mass_kg;
    state.vehicle_config.gravity = aircraft_spec_.gravity_m_s2;
    state.vehicle_config.Ixx = aircraft_spec_.inertia[0][0];
    state.vehicle_config.Iyy = aircraft_spec_.inertia[1][1];
    state.vehicle_config.Izz = aircraft_spec_.inertia[2][2];
    state.rotor_config.thrust_coefficient =
        aircraft_spec_.rotors.front().thrust_coefficient;
    state.rotor_config.torque_coefficient =
        aircraft_spec_.rotors.front().torque_coefficient;
    double arm_length_sum = 0.0;
    double minimum_speed_limit = aircraft_spec_.rotors.front().maximum_speed_rad_s;
    for (const AircraftRotorSpec& rotor : aircraft_spec_.rotors) {
        arm_length_sum += std::sqrt(
            rotor.position_body_m[0] * rotor.position_body_m[0] +
            rotor.position_body_m[1] * rotor.position_body_m[1] +
            rotor.position_body_m[2] * rotor.position_body_m[2]);
        minimum_speed_limit =
            std::min(minimum_speed_limit, rotor.maximum_speed_rad_s);
    }
    state.rotor_config.arm_length_m =
        arm_length_sum / static_cast<double>(aircraft_spec_.rotors.size());
    state.rotor_config.maximum_speed_rad_s = minimum_speed_limit;
    state.aircraft_contract.aircraft_id = aircraft_spec_.aircraft_id;
    state.aircraft_contract.aircraft_revision = aircraft_spec_.revision;
    state.aircraft_contract.coefficient_bundle_id =
        coefficient_bundle_.bundle_id;
    state.aircraft_contract.physical_flight_approved =
        coefficient_bundle_.physical_flight_approved;
    state.aircraft_contract.loaded = true;
    state.aircraft_contract.error.clear();
    return true;
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
    state.physics.motor_command_saturated = false;
    for (size_t i = 0; i < vehicle_config_.rotor_count; ++i) {
        const double command = state.motor_commands.omega_rad_s[i];
        if (!std::isfinite(command)) {
            state.physics.last_result = static_cast<int>(DM_INVALID_ARGUMENT);
            state.physics.integration_valid = false;
            ++state.physics.rejected_steps;
            state.control.paused = true;
            return;
        }
        rotor_omega[i] = std::clamp(
            command, 0.0, aircraft_spec_.rotors[i].maximum_speed_rad_s);
        state.physics.motor_command_saturated |= rotor_omega[i] != command;
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
