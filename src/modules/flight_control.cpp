#include "modules/flight_control.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "attitude/euler.h"
#include "core/simulation_state.h"

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMaximumTiltRad = 25.0 * kPi / 180.0;
constexpr double kMaximumYawRateRadS = 60.0 * kPi / 180.0;
constexpr double kCollectiveRange = 0.40;
constexpr float kStickDeadzone = 0.08f;

double applyDeadzone(float value)
{
    const double magnitude = std::abs(static_cast<double>(value));
    if (magnitude <= kStickDeadzone) {
        return 0.0;
    }
    const double scaled = (magnitude - kStickDeadzone) / (1.0 - kStickDeadzone);
    return std::copysign(std::min(1.0, scaled), value);
}
}

void FlightControlModule::initialize(SimulationState& state)
{
    initialized_ = false;
    std::string error;
    if (!loadAircraftSpec(ASR_DEFAULT_AIRCRAFT_SPEC, aircraft_, error) ||
        !loadControllerCoefficientBundle(ASR_DEFAULT_COEFFICIENT_BUNDLE,
                                         aircraft_, coefficients_, error)) {
        state.pilot.controller_valid = false;
        state.pilot.status = error;
        return;
    }

    cs_pid_gains_t gains[3]{};
    for (std::size_t index = 0; index < 3; ++index) {
        const auto& source = coefficients_.attitude_pid[index];
        gains[index] = {source.kp, source.ki, source.kd,
                        source.integrator_limit, source.output_limit};
    }
    if (!cs_attitude_pid_init_checked(&controller_, gains, coefficients_.rate_weight)) {
        state.pilot.status = "controlSystems rejected the PID coefficient bundle.";
        state.pilot.controller_valid = false;
        return;
    }

    cs_rotor_config_t rotors[CS_MAX_ROTORS]{};
    for (std::size_t index = 0; index < aircraft_.rotors.size(); ++index) {
        const auto& source = aircraft_.rotors[index];
        std::copy(source.position_body_m.begin(), source.position_body_m.end(),
                  rotors[index].position);
        std::copy(source.thrust_axis_body.begin(), source.thrust_axis_body.end(),
                  rotors[index].axis);
        rotors[index].direction = source.spin_direction;
        rotors[index].thrust_coeff = source.thrust_coefficient;
        rotors[index].torque_coeff = source.torque_coefficient;
    }
    if (cs_mixer_init(&mixer_, rotors, aircraft_.rotors.size()) != 0) {
        state.pilot.status = "controlSystems rejected the geometry-derived mixer.";
        state.pilot.controller_valid = false;
        return;
    }

    state.pilot.target_yaw_rad = state.euler.yaw;
    state.pilot.controller_valid = true;
    state.pilot.status = "Controller and geometry-derived mixer ready (simulation only).";
    initialized_ = true;
    control_accumulator_s_ = 0.0;
}

void FlightControlModule::update(double dt, SimulationState& state)
{
    if (!initialized_) {
        ++state.pilot.rejected_updates;
        return;
    }
    if (state.pilot.reset_controller) {
        cs_attitude_pid_reset(&controller_);
        control_accumulator_s_ = 0.0;
        state.pilot.reset_controller = false;
    }
    if (!state.pilot.enabled) {
        return;
    }
    if (!state.joystick.connected || !std::isfinite(dt) || dt <= 0.0 ||
        dt > coefficients_.maximum_period_s) {
        state.pilot.enabled = false;
        state.pilot.reset_controller = true;
        state.pilot.status = "Pilot control failed closed: input or controller timing invalid.";
        ++state.pilot.rejected_updates;
        return;
    }

    control_accumulator_s_ += dt;
    if (control_accumulator_s_ < coefficients_.nominal_period_s) {
        return;
    }
    const double control_dt = control_accumulator_s_;
    control_accumulator_s_ = 0.0;
    if (control_dt < coefficients_.minimum_period_s ||
        control_dt > coefficients_.maximum_period_s) {
        state.pilot.enabled = false;
        state.pilot.reset_controller = true;
        state.pilot.status = "Pilot control failed closed: accumulated timing is out of contract.";
        ++state.pilot.rejected_updates;
        return;
    }

    const double yaw_input = applyDeadzone(state.joystick.axes[0]);
    const double collective_input = -applyDeadzone(state.joystick.axes[1]);
    const double roll_input = applyDeadzone(state.joystick.axes[2]);
    const double pitch_input = -applyDeadzone(state.joystick.axes[3]);
    state.pilot.target_roll_rad = roll_input * kMaximumTiltRad;
    state.pilot.target_pitch_rad = pitch_input * kMaximumTiltRad;
    state.pilot.target_yaw_rad += yaw_input * kMaximumYawRateRadS * control_dt;

    EulerAngles target_euler{state.pilot.target_roll_rad,
                             state.pilot.target_pitch_rad,
                             state.pilot.target_yaw_rad, EULER_ZYX};
    cs_attitude_setpoint_t setpoint{};
    euler_to_quaternion(&target_euler, setpoint.quaternion);
    setpoint.angular_rate[2] = yaw_input * kMaximumYawRateRadS;

    cs_state_t current{};
    std::copy(state.quaternion.begin(), state.quaternion.end(), current.quaternion);
    for (std::size_t index = 0; index < 3; ++index) {
        current.position[index] = state.physics.position[index];
        current.velocity[index] = state.physics.velocity[index];
        current.angular_rate[index] = state.angular_rate_deg_per_sec[index] * kPi / 180.0;
    }

    cs_actuator_command_t command{};
    if (!cs_attitude_pid_update_checked(&controller_, &setpoint, &current,
                                        control_dt, &command)) {
        state.pilot.enabled = false;
        state.pilot.status = "controlSystems rejected the attitude update.";
        ++state.pilot.rejected_updates;
        return;
    }
    // The aircraft contract uses FRD and thrust axes along body -Z.
    command.collective_thrust =
        -aircraft_.mass_kg * aircraft_.gravity_m_s2 *
        std::clamp(1.0 + collective_input * kCollectiveRange, 0.6, 1.4);
    state.pilot.collective_thrust_n = command.collective_thrust;
    state.pilot.requested_torque_nm = {
        command.body_torque[0], command.body_torque[1], command.body_torque[2]};

    double omega[CS_MAX_ROTORS]{};
    if (cs_mixer_mix(&mixer_, &command, omega) != 0) {
        state.pilot.enabled = false;
        state.pilot.status = "Geometry-derived mixer rejected the requested wrench.";
        ++state.pilot.rejected_updates;
        return;
    }
    for (std::size_t index = 0; index < aircraft_.rotors.size(); ++index) {
        state.motor_commands.omega_rad_s[index] = omega[index];
        state.motor_commands.throttle_0_1[index] =
            std::clamp(omega[index] / aircraft_.rotors[index].maximum_speed_rad_s,
                       0.0, 1.0);
    }
    ++state.pilot.accepted_updates;
    state.pilot.status = "Joystick -> PID -> geometry mixer -> RK4 plant active.";
}
