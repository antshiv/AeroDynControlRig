#include "gui/panels/joystick_panel.h"

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>
#include "core/simulation_state.h"
#include "imgui.h"
#include "render/camera.h"

namespace {
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
    ImGui::TextColored(joystick.connected ? ImVec4(0.25f, 0.9f, 0.55f, 1.0f)
                                           : ImVec4(1.0f, 0.35f, 0.30f, 1.0f),
                       "%s", joystick.connected ? "CONNECTED" : "DISCONNECTED");
    ImGui::SameLine();
    ImGui::TextUnformatted(joystick.name.c_str());
    ImGui::TextWrapped("%s", joystick.status.c_str());
    ImGui::TextColored(state.pilot.enabled ? ImVec4(1.0f, 0.58f, 0.16f, 1.0f)
                                           : ImVec4(0.55f, 0.62f, 0.70f, 1.0f),
                       "SIL CONTROL: %s", state.pilot.enabled ? "ACTIVE" : "OFF");

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
    drawButton(draw, {face.x, face.y + 31.0f}, "A", joystick.buttons[0]);
    drawButton(draw, {face.x + 31.0f, face.y}, "B", joystick.buttons[1]);
    drawButton(draw, {face.x - 31.0f, face.y}, "X", joystick.buttons[2]);
    drawButton(draw, {face.x, face.y - 31.0f}, "Y", joystick.buttons[3]);

    drawPill(draw, {origin.x + width * 0.43f, origin.y + 110.0f}, {22.0f, 8.0f},
             "BACK", joystick.buttons[GLFW_GAMEPAD_BUTTON_BACK]);
    drawPill(draw, {origin.x + width * 0.50f, origin.y + 95.0f}, {18.0f, 8.0f},
             "MODE", joystick.buttons[GLFW_GAMEPAD_BUTTON_GUIDE]);
    drawPill(draw, {origin.x + width * 0.57f, origin.y + 110.0f}, {22.0f, 8.0f},
             "START", joystick.buttons[GLFW_GAMEPAD_BUTTON_START]);

    draw->AddText({origin.x + 20.0f, origin.y + 10.0f},
                  IM_COL32(230, 236, 242, 255), "LOGITECH F310 / MODE 2");
    draw->AddText({left.x - 48.0f, origin.y + 240.0f},
                  IM_COL32(155, 170, 184, 255), "THROTTLE / YAW");
    draw->AddText({right.x - 35.0f, origin.y + 240.0f},
                  IM_COL32(155, 170, 184, 255), "PITCH / ROLL");
    ImGui::Dummy({width, height});

    if (joystick.show_guide) {
        ImGui::SeparatorText("Button guide");
        ImGui::BulletText("A: enable/disable SIL control (sticks must be centered)");
        ImGui::BulletText("B: pause/resume simulation");
        ImGui::BulletText("X: level roll/pitch and retain current heading");
        ImGui::BulletText("Y: show/hide this guide");
        ImGui::BulletText("BACK: disable control and clear PID state");
        ImGui::BulletText("D-pad: one-degree roll/pitch trim");
        ImGui::BulletText("LB/RB: decrease/increase command authority");
        ImGui::BulletText("LT/RT: decrease/increase collective thrust");
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
    ImGui::Text("Trim  roll %+.1f deg  pitch %+.1f deg",
                state.pilot.roll_trim_rad * 180.0 / 3.14159265358979323846,
                state.pilot.pitch_trim_rad * 180.0 / 3.14159265358979323846);
    ImGui::TextWrapped("%s", state.pilot.status.c_str());
    ImGui::End();
}
