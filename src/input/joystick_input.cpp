#include "input/joystick_input.h"

#include <algorithm>
#include <cmath>
#include <string>

#include <GLFW/glfw3.h>

#include "core/simulation_state.h"

namespace {
constexpr float kEnableDeadzone = 0.25f;
constexpr double kTrimStepRad = 3.14159265358979323846 / 180.0;
constexpr double kMaximumTrimRad = 15.0 * 3.14159265358979323846 / 180.0;
constexpr double kMinimumCommandScale = 0.25;
constexpr double kCommandScaleStep = 0.10;

bool sticksCentered(const SimulationState::JoystickState& joystick)
{
    return std::all_of(joystick.axes.begin(), joystick.axes.begin() + 4,
                       [](float value) { return std::abs(value) <= kEnableDeadzone; });
}

void stopDesktopSil(SimulationState& state, const char* status,
                    bool latch_emergency)
{
    if (latch_emergency &&
        state.mission.phase != SimulationState::FlightPhase::EmergencyStopped) {
        state.mission.phase = SimulationState::FlightPhase::EmergencyStopped;
        state.mission.status = status;
        ++state.mission.transition_count;
    }
    state.pilot.enabled = false;
    state.pilot.reset_controller = true;
    state.control.paused = true;
    state.motor_commands.omega_rad_s.fill(0.0);
    state.motor_commands.throttle_0_1.fill(0.0);
    state.pilot.status = status;
}

void recordAction(SimulationState& state, const char* action)
{
    ++state.joystick.action_sequence;
    state.joystick.last_action = action;
}

bool isLogitechDualAction(const char* name)
{
    return name != nullptr &&
           std::string(name).find("Logitech Dual Action") != std::string::npos;
}

bool installLogitechDualActionMapping(int joystick_id)
{
    const char* guid = glfwGetJoystickGUID(joystick_id);
    if (guid == nullptr) {
        return false;
    }
    const std::string mapping =
        std::string(guid) +
        ",Logitech Dual Action,"
        "a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,"
        "leftshoulder:b4,rightshoulder:b5,lefttrigger:b6,righttrigger:b7,"
        "leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,"
        "dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,";
    return glfwUpdateGamepadMappings(mapping.c_str()) == GLFW_TRUE;
}
}

void JoystickInput::poll(SimulationState& state)
{
    auto& joystick = state.joystick;
    joystick.pressed.fill(false);
    joystick.connected = glfwJoystickPresent(GLFW_JOYSTICK_1) == GLFW_TRUE;
    if (!joystick.connected) {
        joystick.name = "No joystick";
        joystick.standardized_mapping = false;
        joystick.analog_mode_warning = false;
        joystick.axes = {0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f};
        joystick.buttons.fill(false);
        stopDesktopSil(
            state,
            "Joystick disconnected; SIL paused and virtual motors cleared.",
            state.pilot.enabled);
        joystick.status = "Connect a gamepad to enable pilot input.";
        previous_buttons_.fill(false);
        was_connected_ = false;
        mapping_attempted_ = false;
        return;
    }

    const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
    joystick.name = name != nullptr ? name : "Unknown joystick";
    if (!mapping_attempted_) {
        joystick.known_mapping_installed =
            isLogitechDualAction(name) &&
            installLogitechDualActionMapping(GLFW_JOYSTICK_1);
        mapping_attempted_ = true;
    }
    joystick.profile = isLogitechDualAction(name)
                           ? "Logitech Dual Action / Mode 2"
                           : "Generic gamepad / Mode 2";
    GLFWgamepadstate gamepad{};
    joystick.standardized_mapping =
        glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad) == GLFW_TRUE;
    if (isLogitechDualAction(name) && joystick.standardized_mapping) {
        joystick.known_mapping_installed = true;
    }
    if (joystick.standardized_mapping) {
        joystick.axes = {
            gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
            gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_Y],
            gamepad.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
            gamepad.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y],
            gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER],
            gamepad.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER],
        };
        for (std::size_t index = 0; index < joystick.buttons.size(); ++index) {
            joystick.buttons[index] = gamepad.buttons[index] == GLFW_PRESS;
        }
    } else {
        int axis_count = 0;
        int button_count = 0;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axis_count);
        const unsigned char* buttons =
            glfwGetJoystickButtons(GLFW_JOYSTICK_1, &button_count);
        joystick.axes = {0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f};
        for (std::size_t index = 0; index < 4; ++index) {
            joystick.axes[index] =
                axes != nullptr && static_cast<int>(index) < axis_count ? axes[index] : 0.0f;
        }
        for (std::size_t index = 0; index < joystick.buttons.size(); ++index) {
            joystick.buttons[index] =
                buttons != nullptr && static_cast<int>(index) < button_count &&
                buttons[index] == GLFW_PRESS;
        }
    }

    if (!was_connected_) {
        previous_buttons_ = joystick.buttons;
        was_connected_ = true;
    }
    for (std::size_t index = 0; index < joystick.buttons.size(); ++index) {
        joystick.pressed[index] = joystick.buttons[index] && !previous_buttons_[index];
    }
    previous_buttons_ = joystick.buttons;

    joystick.analog_mode_warning = std::all_of(
        joystick.axes.begin(), joystick.axes.begin() + 4,
        [](float value) { return value < -0.95f; });
    if (joystick.analog_mode_warning) {
        joystick.status =
            "All axes report -1. Press the controller MODE button and verify the sticks recenter.";
    } else if (!joystick.standardized_mapping) {
        joystick.status = "Using raw DirectInput fallback mapping.";
    } else if (joystick.known_mapping_installed) {
        joystick.status =
            "Dual Action mapping loaded. Center the sticks, then press physical button 2 (A).";
    } else {
        joystick.status = "Gamepad ready. Center the sticks, then press A to enable SIL control.";
    }

    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_A]) {
        recordAction(state, "2 / A: run or resume SIL");
        if (state.pilot.enabled) {
            state.control.paused = false;
            state.pilot.status = "Desktop SIL already active; simulation running.";
        } else if (sticksCentered(joystick) && !joystick.analog_mode_warning) {
            state.pilot.enabled = true;
            state.pilot.reset_controller = true;
            state.pilot.target_yaw_rad = state.euler.yaw;
            state.control.paused = false;
            state.pilot.status =
                "Desktop SIL active: joystick -> PID -> mixer -> RK4 plant.";
        } else {
            state.pilot.status = "Enable rejected: center all sticks and clear the MODE warning.";
        }
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_B]) {
        recordAction(state, "3 / B: pause SIL");
        state.control.paused = true;
        state.pilot.status = "Desktop SIL paused; controller remains armed in simulation.";
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_X]) {
        recordAction(state, "1 / X: level attitude target");
        state.pilot.target_roll_rad = 0.0;
        state.pilot.target_pitch_rad = 0.0;
        state.pilot.target_yaw_rad = state.euler.yaw;
        state.pilot.reset_controller = true;
        state.pilot.status = "Attitude target levelled; current heading retained.";
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_Y]) {
        recordAction(state, "4 / Y: recenter aircraft view");
        state.control.camera_fit_requested = true;
        state.pilot.status = "Camera recenter requested.";
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]) {
        recordAction(state, "D-pad left: decrease roll trim");
        state.pilot.roll_trim_rad =
            std::max(-kMaximumTrimRad, state.pilot.roll_trim_rad - kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]) {
        recordAction(state, "D-pad right: increase roll trim");
        state.pilot.roll_trim_rad =
            std::min(kMaximumTrimRad, state.pilot.roll_trim_rad + kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_UP]) {
        recordAction(state, "D-pad up: increase pitch trim");
        state.pilot.pitch_trim_rad =
            std::min(kMaximumTrimRad, state.pilot.pitch_trim_rad + kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]) {
        recordAction(state, "D-pad down: decrease pitch trim");
        state.pilot.pitch_trim_rad =
            std::max(-kMaximumTrimRad, state.pilot.pitch_trim_rad - kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]) {
        recordAction(state, "Left shoulder: decrease command authority");
        state.pilot.command_scale =
            std::max(kMinimumCommandScale, state.pilot.command_scale - kCommandScaleStep);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]) {
        recordAction(state, "Right shoulder: increase command authority");
        state.pilot.command_scale =
            std::min(1.0, state.pilot.command_scale + kCommandScaleStep);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_LEFT_THUMB]) {
        recordAction(state, "Left-stick click: clear trim and level");
        state.pilot.roll_trim_rad = 0.0;
        state.pilot.pitch_trim_rad = 0.0;
        state.pilot.target_roll_rad = 0.0;
        state.pilot.target_pitch_rad = 0.0;
        state.pilot.reset_controller = true;
        state.pilot.status = "Pilot trims cleared and attitude target levelled.";
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB]) {
        recordAction(state, "Right-stick click: recenter aircraft view");
        state.control.camera_fit_requested = true;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_START]) {
        recordAction(state, "Start: reset aircraft and telemetry");
        state.control.reset_requested = true;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_GUIDE]) {
        recordAction(state, "Guide: toggle controller help");
        joystick.show_guide = !joystick.show_guide;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_BACK]) {
        recordAction(state, "Back: emergency stop");
        stopDesktopSil(
            state,
            "Emergency stop: SIL paused, PID reset, and virtual motors cleared.",
            true);
    }
}
