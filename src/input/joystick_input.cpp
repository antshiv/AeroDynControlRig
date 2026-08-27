#include "input/joystick_input.h"

#include <algorithm>
#include <cmath>

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
        state.pilot.enabled = false;
        state.pilot.status = "Joystick disconnected; closed-loop pilot control disabled.";
        joystick.status = "Connect a gamepad to enable pilot input.";
        previous_buttons_.fill(false);
        was_connected_ = false;
        return;
    }

    const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
    joystick.name = name != nullptr ? name : "Unknown joystick";
    GLFWgamepadstate gamepad{};
    joystick.standardized_mapping =
        glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad) == GLFW_TRUE;
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
            "All axes report -1. Press the F310 MODE button and verify the sticks recenter.";
    } else if (!joystick.standardized_mapping) {
        joystick.status = "Using raw DirectInput fallback mapping.";
    } else {
        joystick.status = "F310 ready. Center the sticks, then press A to enable SIL control.";
    }

    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_A]) {
        if (state.pilot.enabled) {
            state.pilot.enabled = false;
            state.pilot.reset_controller = true;
            state.pilot.status = "Closed-loop pilot control disabled by A button.";
        } else if (sticksCentered(joystick) && !joystick.analog_mode_warning) {
            state.pilot.enabled = true;
            state.pilot.reset_controller = true;
            state.pilot.target_yaw_rad = state.euler.yaw;
            state.pilot.status = "Closed-loop pilot control enabled for simulation only.";
        } else {
            state.pilot.status = "Enable rejected: center all sticks and clear the MODE warning.";
        }
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_B]) {
        state.control.paused = !state.control.paused;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_X]) {
        state.pilot.target_roll_rad = 0.0;
        state.pilot.target_pitch_rad = 0.0;
        state.pilot.target_yaw_rad = state.euler.yaw;
        state.pilot.reset_controller = true;
        state.pilot.status = "Attitude target levelled; current heading retained.";
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_Y]) {
        joystick.show_guide = !joystick.show_guide;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]) {
        state.pilot.roll_trim_rad =
            std::max(-kMaximumTrimRad, state.pilot.roll_trim_rad - kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]) {
        state.pilot.roll_trim_rad =
            std::min(kMaximumTrimRad, state.pilot.roll_trim_rad + kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_UP]) {
        state.pilot.pitch_trim_rad =
            std::min(kMaximumTrimRad, state.pilot.pitch_trim_rad + kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]) {
        state.pilot.pitch_trim_rad =
            std::max(-kMaximumTrimRad, state.pilot.pitch_trim_rad - kTrimStepRad);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]) {
        state.pilot.command_scale =
            std::max(kMinimumCommandScale, state.pilot.command_scale - kCommandScaleStep);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]) {
        state.pilot.command_scale =
            std::min(1.0, state.pilot.command_scale + kCommandScaleStep);
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_LEFT_THUMB]) {
        state.pilot.roll_trim_rad = 0.0;
        state.pilot.pitch_trim_rad = 0.0;
        state.pilot.target_roll_rad = 0.0;
        state.pilot.target_pitch_rad = 0.0;
        state.pilot.reset_controller = true;
        state.pilot.status = "Pilot trims cleared and attitude target levelled.";
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB]) {
        state.control.camera_fit_requested = true;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_START]) {
        state.control.reset_requested = true;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_GUIDE]) {
        joystick.show_guide = !joystick.show_guide;
    }
    if (joystick.pressed[GLFW_GAMEPAD_BUTTON_BACK]) {
        state.pilot.enabled = false;
        state.pilot.reset_controller = true;
        state.pilot.status = "Pilot control disabled and controller state cleared.";
    }
}
