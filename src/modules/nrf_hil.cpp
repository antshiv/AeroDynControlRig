#include "modules/nrf_hil.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>

#include "attitude/euler.h"
#include "core/simulation_state.h"

namespace {
constexpr double kControlPeriodS = 0.01;
constexpr std::uint32_t kResponseTimeoutMs = 30;
constexpr double kPi = 3.14159265358979323846;

std::uint32_t newSessionId()
{
    using clock = std::chrono::steady_clock;
    const auto ticks = static_cast<std::uint64_t>(
        clock::now().time_since_epoch().count());
    const std::uint32_t session = static_cast<std::uint32_t>(
        ticks ^ (ticks >> 32u) ^ 0x41535231u);
    return session == 0u ? 1u : session;
}
}

void NrfHilModule::initialize(SimulationState& state)
{
    session_id_ = newSessionId();
    const char* configured = std::getenv("ASR_FC_HIL_DEVICE");
    if (configured == nullptr || configured[0] == '\0') {
        return;
    }
    std::string error;
    state.execution.nrf_hil_device = configured;
    state.execution.nrf_hil_available = transport_.openDevice(configured, error);
    state.execution.nrf_hil_status = state.execution.nrf_hil_available
        ? "Motor-disabled nRF5340 HIL endpoint ready."
        : error;
}

void NrfHilModule::clearMotorAuthority(SimulationState& state)
{
    state.motor_commands.omega_rad_s.fill(0.0);
    state.motor_commands.throttle_0_1.fill(0.0);
}

void NrfHilModule::failClosed(SimulationState& state, const std::string& reason)
{
    clearMotorAuthority(state);
    response_valid_ = false;
    state.pilot.enabled = false;
    state.pilot.reset_controller = true;
    state.control.paused = true;
    state.execution.nrf_hil_status = reason;
    ++state.execution.nrf_hil_failures;
}

bool NrfHilModule::responseWithinContract(
    const asr_fc_hil_flight_output_t& response,
    const SimulationState& state) const
{
    const double speed_limit = state.rotor_config.maximum_speed_rad_s;
    if (!std::isfinite(speed_limit) || speed_limit <= 0.0) {
        return false;
    }
    for (const float speed : response.motor_speed_rad_s) {
        if (!std::isfinite(speed) || speed < 0.0 || speed > speed_limit) {
            return false;
        }
    }
    return true;
}

void NrfHilModule::update(double dt, SimulationState& state)
{
    const bool selected = state.execution.selected_mode ==
        SimulationState::ExecutionMode::Nrf5340Hil;
    if (!selected) {
        if (hil_selected_) {
            clearMotorAuthority(state);
            state.pilot.reset_controller = true;
            state.execution.active_path =
                "Joystick -> PID -> mixer -> RK4 plant";
            state.execution.nrf_hil_status =
                "nRF5340 HIL endpoint ready; desktop SIL selected.";
            accumulator_s_ = 0.0;
            response_valid_ = false;
            hil_selected_ = false;
        }
        return;
    }
    if (!hil_selected_) {
        clearMotorAuthority(state);
        state.execution.active_path =
            "Waiting for checked nRF5340 controller output";
        state.execution.nrf_hil_status =
            "HIL selected; virtual motors remain stopped until the first valid response.";
        accumulator_s_ = 0.0;
        response_valid_ = false;
        hil_selected_ = true;
    }
    if (!transport_.available() || !std::isfinite(dt) || dt <= 0.0) {
        failClosed(state, "nRF5340 HIL unavailable or timing invalid.");
        return;
    }
    accumulator_s_ += dt;
    if (accumulator_s_ < kControlPeriodS) {
        if (!response_valid_) {
            clearMotorAuthority(state);
        }
        return;
    }
    accumulator_s_ -= kControlPeriodS;

    asr_fc_hil_sensor_guidance_t request{};
    request.session_id = session_id_;
    request.host_timestamp_us = static_cast<std::uint64_t>(
        std::max(0.0, state.time_seconds) * 1.0e6);
    request.sensor_timestamp_us = request.host_timestamp_us;
    request.arm_requested = state.pilot.enabled &&
        state.mission.phase != SimulationState::FlightPhase::Grounded &&
        state.mission.phase != SimulationState::FlightPhase::EmergencyStopped;
    request.sensor_accuracy = 3;
    for (std::size_t index = 0; index < 4; ++index) {
        request.quaternion[index] = static_cast<float>(state.quaternion[index]);
    }
    for (std::size_t index = 0; index < 3; ++index) {
        request.angular_rate[index] = static_cast<float>(
            state.angular_rate_deg_per_sec[index] * kPi / 180.0);
        request.linear_acceleration[index] = static_cast<float>(
            state.physics.acceleration[index]);
    }
    EulerAngles guidance{state.pilot.target_roll_rad,
                         state.pilot.target_pitch_rad,
                         state.pilot.target_yaw_rad, EULER_ZYX};
    double guidance_quaternion[4]{};
    euler_to_quaternion(&guidance, guidance_quaternion);
    for (std::size_t index = 0; index < 4; ++index) {
        request.guidance_quaternion[index] =
            static_cast<float>(guidance_quaternion[index]);
    }
    request.collective_thrust_n =
        static_cast<float>(state.pilot.collective_thrust_n);

    asr_fc_hil_flight_output_t response{};
    std::string error;
    ++state.execution.nrf_hil_requests;
    if (!transport_.exchange(request, response, kResponseTimeoutMs, error)) {
        failClosed(state, error.empty() ? "nRF5340 rejected the HIL step." : error);
        return;
    }
    if (response.step_result != 0 || response.fault_flags != 0) {
        failClosed(state, "nRF5340 reported a flight-core fault.");
        return;
    }
    if (!responseWithinContract(response, state)) {
        failClosed(state, "nRF5340 returned motor output outside the aircraft contract.");
        return;
    }
    response_valid_ = true;
    state.execution.nrf_hil_execution_us = response.execution_time_us;
    state.execution.nrf_hil_fault_flags = response.fault_flags;
    state.execution.nrf_hil_status = "nRF5340 controller -> virtual motors -> RK4 plant.";
    state.execution.active_path = state.execution.nrf_hil_status;
    for (std::size_t index = 0; index < 4; ++index) {
        state.motor_commands.omega_rad_s[index] = response.motor_speed_rad_s[index];
        state.motor_commands.throttle_0_1[index] =
            static_cast<double>(response.motor_q15[index]) / 32767.0;
    }
}
