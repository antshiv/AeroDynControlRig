#include "gui/panels/joystick_panel.h"

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>
#include "core/simulation_state.h"
#include "imgui.h"
#include "render/camera.h"

namespace {
bool sticksCentered(const SimulationState::JoystickState& joystick)
{
    return std::all_of(
        joystick.axes.begin(), joystick.axes.begin() + 4,
        [](float value) { return std::abs(value) <= 0.25f; });
}

ImU32 buttonColor(bool active)
{
    return ImGui::GetColorU32(active ? ImVec4(1.0f, 0.55f, 0.12f, 1.0f)
                                     : ImVec4(0.20f, 0.25f, 0.31f, 1.0f));
}

void drawButton(ImDrawList* draw, ImVec2 center, const char* label, bool active)
{
    draw->AddCircleFilled(center, 13.0f, buttonColor(active));
    draw->AddCircle(center, 13.0f, IM_COL32(220, 230, 240, 255), 0, 1.5f);
    const ImVec2 text = ImGui::CalcTextSize(label);
    draw->AddText({center.x - text.x * 0.5f, center.y - text.y * 0.5f},
                  IM_COL32(245, 248, 250, 255), label);
}

void drawStick(ImDrawList* draw, ImVec2 center, float x, float y, bool active)
{
    draw->AddCircleFilled(center, 30.0f, IM_COL32(25, 31, 39, 255));
    draw->AddCircle(center, 30.0f, IM_COL32(115, 130, 145, 255), 0, 1.5f);
    const ImVec2 knob{center.x + x * 16.0f, center.y + y * 16.0f};
    draw->AddCircleFilled(knob, 11.0f, buttonColor(active));
}

void drawPill(ImDrawList* draw, ImVec2 center, ImVec2 half_size,
              const char* label, bool active)
{
    draw->AddRectFilled({center.x - half_size.x, center.y - half_size.y},
                        {center.x + half_size.x, center.y + half_size.y},
                        buttonColor(active), 5.0f);
    draw->AddRect({center.x - half_size.x, center.y - half_size.y},
                  {center.x + half_size.x, center.y + half_size.y},
                  IM_COL32(190, 203, 215, 255), 5.0f, 0, 1.0f);
    const ImVec2 text = ImGui::CalcTextSize(label);
    draw->AddText({center.x - text.x * 0.5f, center.y - text.y * 0.5f},
                  IM_COL32(245, 248, 250, 255), label);
}

void drawDpad(ImDrawList* draw, ImVec2 center,
              const SimulationState::JoystickState& joystick)
{
    constexpr float arm = 17.0f;
    constexpr float half = 8.0f;
    draw->AddRectFilled({center.x - half, center.y - arm - half},
                        {center.x + half, center.y + arm + half},
                        IM_COL32(34, 42, 51, 255), 3.0f);
    draw->AddRectFilled({center.x - arm - half, center.y - half},
                        {center.x + arm + half, center.y + half},
                        IM_COL32(34, 42, 51, 255), 3.0f);
    const auto active = [&](int button) { return joystick.buttons[button]; };
    if (active(GLFW_GAMEPAD_BUTTON_DPAD_UP))
        draw->AddRectFilled({center.x - half, center.y - arm - half},
                            {center.x + half, center.y - half}, buttonColor(true), 3.0f);
    if (active(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT))
        draw->AddRectFilled({center.x + half, center.y - half},
                            {center.x + arm + half, center.y + half}, buttonColor(true), 3.0f);
    if (active(GLFW_GAMEPAD_BUTTON_DPAD_DOWN))
        draw->AddRectFilled({center.x - half, center.y + half},
                            {center.x + half, center.y + arm + half}, buttonColor(true), 3.0f);
    if (active(GLFW_GAMEPAD_BUTTON_DPAD_LEFT))
        draw->AddRectFilled({center.x - arm - half, center.y - half},
                            {center.x - half, center.y + half}, buttonColor(true), 3.0f);
}
}

void JoystickPanel::draw(SimulationState& state, Camera& camera)
{
    (void)camera;
    ImGui::SetNextWindowPos(ImVec2(80.0f, 120.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(520.0f, 650.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(name())) {
        ImGui::End();
        return;
    }
    const auto& joystick = state.joystick;
    const bool dual_action =
        joystick.profile.find("Dual Action") != std::string::npos;
    ImGui::TextColored(joystick.connected ? ImVec4(0.25f, 0.9f, 0.55f, 1.0f)
                                           : ImVec4(1.0f, 0.35f, 0.30f, 1.0f),
                       "%s", joystick.connected ? "CONNECTED" : "DISCONNECTED");
    ImGui::SameLine();
    ImGui::TextUnformatted(joystick.name.c_str());
    ImGui::TextWrapped("%s", joystick.status.c_str());
    ImGui::TextDisabled("Profile: %s", joystick.profile.c_str());
    ImGui::TextWrapped("Last action #%llu: %s",
                       static_cast<unsigned long long>(joystick.action_sequence),
                       joystick.last_action.c_str());
    ImGui::TextColored(state.pilot.enabled ? ImVec4(1.0f, 0.58f, 0.16f, 1.0f)
                                           : ImVec4(0.55f, 0.62f, 0.70f, 1.0f),
                       "SIL CONTROL: %s", state.pilot.enabled ? "ACTIVE" : "OFF");
    ImGui::TextWrapped(
        "DRONE FLIGHT: left stick changes height and heading; right stick moves horizontally.");
    if (ImGui::Checkbox("Training assist: sticks command velocity",
                        &state.pilot.assisted_velocity_mode)) {
        state.pilot.reset_controller = true;
    }
    if (state.pilot.assisted_velocity_mode) {
        ImGui::TextDisabled(
            "Release the sticks to brake horizontal and vertical drift.");
        ImGui::Text("Velocity target N/E/D  %+.2f  %+.2f  %+.2f m/s",
                    state.pilot.desired_velocity_ned.x,
                    state.pilot.desired_velocity_ned.y,
                    state.pilot.desired_velocity_ned.z);
    } else {
        ImGui::TextDisabled(
            "Advanced mode: direct attitude + collective; no automatic braking.");
    }

    ImGui::SeparatorText("Demo execution path");
    bool desktop_sil = state.execution.selected_mode ==
        SimulationState::ExecutionMode::DesktopSil;
    if (ImGui::RadioButton("Desktop SIL", desktop_sil)) {
        state.execution.selected_mode =
            SimulationState::ExecutionMode::DesktopSil;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!state.execution.nrf_hil_available);
    bool nrf_hil = state.execution.selected_mode ==
        SimulationState::ExecutionMode::Nrf5340Hil;
    ImGui::RadioButton("nRF5340 HIL", nrf_hil);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!state.execution.ceva_replay_available);
    bool ceva_replay = state.execution.selected_mode ==
        SimulationState::ExecutionMode::LiveCevaReplay;
    ImGui::RadioButton("CEVA replay", ceva_replay);
    ImGui::EndDisabled();
    ImGui::TextDisabled("Active: %s", state.execution.active_path.c_str());

    const bool centered = sticksCentered(joystick);
    ImGui::SeparatorText("Readiness gates");
    ImGui::TextColored(joystick.connected ? ImVec4(0.25f, 0.9f, 0.55f, 1.0f)
                                          : ImVec4(1.0f, 0.35f, 0.30f, 1.0f),
                       "%s  controller connected", joystick.connected ? "PASS" : "WAIT");
    ImGui::TextColored(centered ? ImVec4(0.25f, 0.9f, 0.55f, 1.0f)
                                : ImVec4(1.0f, 0.70f, 0.20f, 1.0f),
                       "%s  sticks centered", centered ? "PASS" : "WAIT");
    ImGui::TextColored(state.pilot.controller_valid ?
                           ImVec4(0.25f, 0.9f, 0.55f, 1.0f) :
                           ImVec4(1.0f, 0.35f, 0.30f, 1.0f),
                       "%s  controller + mixer contract",
                       state.pilot.controller_valid ? "PASS" : "FAIL");
    ImGui::TextColored(state.physics.integration_valid ?
                           ImVec4(0.25f, 0.9f, 0.55f, 1.0f) :
                           ImVec4(1.0f, 0.35f, 0.30f, 1.0f),
                       "%s  checked RK4 plant",
                       state.physics.integration_valid ? "PASS" : "FAIL");

    if (ImGui::Button("Run / resume SIL")) {
        if (state.pilot.enabled) {
            state.control.paused = false;
            state.pilot.status = "Desktop SIL already active; simulation running.";
        } else if (joystick.connected && centered &&
                   !joystick.analog_mode_warning &&
                   state.pilot.controller_valid &&
                   state.physics.integration_valid) {
            state.pilot.enabled = true;
            state.pilot.reset_controller = true;
            state.pilot.target_yaw_rad = state.euler.yaw;
            state.control.paused = false;
            state.pilot.status =
                "Desktop SIL active: joystick -> PID -> mixer -> RK4 plant.";
        } else {
            state.pilot.status =
                "SIL start rejected: satisfy every readiness gate first.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        state.control.paused = true;
        state.pilot.status = "Desktop SIL paused; controller remains armed in simulation.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Emergency stop")) {
        state.pilot.enabled = false;
        state.pilot.reset_controller = true;
        state.control.paused = true;
        state.motor_commands.omega_rad_s.fill(0.0);
        state.motor_commands.throttle_0_1.fill(0.0);
        state.pilot.status = "Emergency stop: SIL paused and virtual motors cleared.";
    }
    if (ImGui::Button("Reset aircraft")) {
        state.control.reset_requested = true;
    }

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float width = std::max(330.0f, ImGui::GetContentRegionAvail().x);
    const float height = 285.0f;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(origin, {origin.x + width, origin.y + height},
                        IM_COL32(13, 18, 24, 255), 12.0f);
    draw->AddRect({origin.x + 28.0f, origin.y + 24.0f},
                  {origin.x + width - 28.0f, origin.y + 260.0f},
                  IM_COL32(90, 106, 122, 255), 45.0f, 0, 2.0f);

    drawPill(draw, {origin.x + width * 0.25f, origin.y + 50.0f}, {25.0f, 10.0f},
             "LT", joystick.axes[4] > -0.8f);
    drawPill(draw, {origin.x + width * 0.37f, origin.y + 50.0f}, {25.0f, 10.0f},
             "LB", joystick.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]);
    drawPill(draw, {origin.x + width * 0.63f, origin.y + 50.0f}, {25.0f, 10.0f},
             "RB", joystick.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]);
    drawPill(draw, {origin.x + width * 0.75f, origin.y + 50.0f}, {25.0f, 10.0f},
             "RT", joystick.axes[5] > -0.8f);

    const ImVec2 left{origin.x + width * 0.30f, origin.y + 145.0f};
    const ImVec2 right{origin.x + width * 0.63f, origin.y + 174.0f};
    drawStick(draw, left, joystick.axes[0], joystick.axes[1],
              std::abs(joystick.axes[0]) > 0.08f || std::abs(joystick.axes[1]) > 0.08f);
    drawStick(draw, right, joystick.axes[2], joystick.axes[3],
              std::abs(joystick.axes[2]) > 0.08f || std::abs(joystick.axes[3]) > 0.08f);
    if (joystick.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB])
        draw->AddCircle(left, 34.0f, buttonColor(true), 0, 3.0f);
    if (joystick.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB])
        draw->AddCircle(right, 34.0f, buttonColor(true), 0, 3.0f);

    drawDpad(draw, {origin.x + width * 0.18f, origin.y + 205.0f}, joystick);

    const ImVec2 face{origin.x + width * 0.82f, origin.y + 135.0f};
    drawButton(draw, {face.x, face.y + 31.0f}, dual_action ? "2" : "A",
               joystick.buttons[GLFW_GAMEPAD_BUTTON_A]);
    drawButton(draw, {face.x + 31.0f, face.y}, dual_action ? "3" : "B",
               joystick.buttons[GLFW_GAMEPAD_BUTTON_B]);
    drawButton(draw, {face.x - 31.0f, face.y}, dual_action ? "1" : "X",
               joystick.buttons[GLFW_GAMEPAD_BUTTON_X]);
    drawButton(draw, {face.x, face.y - 31.0f}, dual_action ? "4" : "Y",
               joystick.buttons[GLFW_GAMEPAD_BUTTON_Y]);

    drawPill(draw, {origin.x + width * 0.43f, origin.y + 110.0f}, {22.0f, 8.0f},
             "BACK", joystick.buttons[GLFW_GAMEPAD_BUTTON_BACK]);
    drawPill(draw, {origin.x + width * 0.50f, origin.y + 95.0f}, {18.0f, 8.0f},
             "MODE", joystick.buttons[GLFW_GAMEPAD_BUTTON_GUIDE]);
    drawPill(draw, {origin.x + width * 0.57f, origin.y + 110.0f}, {22.0f, 8.0f},
             "START", joystick.buttons[GLFW_GAMEPAD_BUTTON_START]);

    draw->AddText({origin.x + 20.0f, origin.y + 10.0f},
                  IM_COL32(230, 236, 242, 255),
                  dual_action ? "LOGITECH DUAL ACTION / MODE 2" :
                                "GAMEPAD / MODE 2");
    draw->AddText({origin.x + 18.0f, origin.y + 238.0f},
                  IM_COL32(155, 170, 184, 255),
                  "LEFT: UP CLIMB / DOWN DESCEND / SIDE TURN");
    draw->AddText({origin.x + 18.0f, origin.y + 258.0f},
                  IM_COL32(155, 170, 184, 255),
                  "RIGHT: UP FORWARD / DOWN BACK / SIDE STRAFE");
    ImGui::Dummy({width, height});

    if (joystick.show_guide) {
        ImGui::SeparatorText("Button guide");
        ImGui::BulletText("%s: run/resume SIL control (sticks centered)",
                          dual_action ? "2 / A" : "A");
        ImGui::BulletText("%s: pause simulation",
                          dual_action ? "3 / B" : "B");
        ImGui::BulletText("%s: level roll/pitch and retain current heading",
                          dual_action ? "1 / X" : "X");
        ImGui::BulletText("%s: recenter aircraft in the viewport",
                          dual_action ? "4 / Y" : "Y");
        ImGui::BulletText("BACK: disable control and clear PID state");
        ImGui::BulletText("D-pad: one-degree roll/pitch trim");
        ImGui::BulletText("LB/RB: decrease/increase command authority");
        ImGui::BulletText("LT/RT: descend/climb (alternate vertical command)");
        ImGui::BulletText("L3: clear trim  |  R3: fit aircraft in view");
        ImGui::BulletText("START: reset aircraft, controller, and telemetry");
        ImGui::BulletText("MODE: hardware mapping switch; may not emit a button event");
    }
    ImGui::Text("Axes  LX %+.2f  LY %+.2f  RX %+.2f  RY %+.2f",
                joystick.axes[0], joystick.axes[1], joystick.axes[2], joystick.axes[3]);
    ImGui::Text("Triggers  LT %.2f  RT %.2f  |  authority %.0f%%",
                (joystick.axes[4] + 1.0f) * 0.5f,
                (joystick.axes[5] + 1.0f) * 0.5f,
                state.pilot.command_scale * 100.0);
    const double minimum_expo = 0.0;
    const double maximum_expo = 0.75;
    ImGui::SliderScalar("Stick expo", ImGuiDataType_Double,
                        &state.pilot.stick_expo, &minimum_expo, &maximum_expo,
                        "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextDisabled("More expo softens small stick motions without reducing full travel.");
    ImGui::Text("Trim  roll %+.1f deg  pitch %+.1f deg",
                state.pilot.roll_trim_rad * 180.0 / 3.14159265358979323846,
                state.pilot.pitch_trim_rad * 180.0 / 3.14159265358979323846);
    ImGui::TextWrapped("%s", state.pilot.status.c_str());
    ImGui::End();
}
