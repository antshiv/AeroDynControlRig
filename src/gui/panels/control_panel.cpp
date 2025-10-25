#include "gui/panels/control_panel.h"

#include <algorithm>
#include <limits>
#include <glm/gtc/type_ptr.hpp>

#include "core/simulation_state.h"
#include "render/camera.h"

#include "imgui.h"

void ControlPanel::draw(SimulationState& state, Camera& camera) {
    if (!ImGui::Begin(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    // === HELP / KEYBOARD CONTROLS ===
    static bool show_help_modal = false;

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Keyboard Controls");
    if (ImGui::Button("Show Help / Controls (?)")) {
        show_help_modal = true;
    }

    // Quick reference (always visible)
    const char* mode_text = state.control.manual_rotation_mode ? "MANUAL (discrete)" : "AUTOMATIC (continuous)";
    ImGui::Text("Mode: %s  [Press M to toggle]", mode_text);

    if (!state.control.manual_rotation_mode) {
        ImGui::Text("Hold Q/E/Arrows/I/K/J/L to spin  |  Space: Stop");
    } else {
        ImGui::Text("W/A/S/D/Q/E: 5\xC2\xB0 steps  |  Shift+key: 1\xC2\xB0");
    }

    ImGui::Separator();

    // Help Modal Window
    if (show_help_modal) {
        ImGui::OpenPopup("Keyboard Controls & Help");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Keyboard Controls & Help", &show_help_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "AeroDynControlRig - Keyboard Controls");
        ImGui::Separator();

        // Mode Toggle
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ROTATION MODE TOGGLE");
        ImGui::BulletText("Press M to switch between AUTOMATIC and MANUAL modes");
        ImGui::Spacing();

        // Automatic Mode
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "AUTOMATIC MODE (Default) - Continuous Angular Rates");
        ImGui::Indent();
        ImGui::Text("Simulates drone flight with angular velocity:");
        ImGui::BulletText("Q / E           - Roll left/right (continuous)");
        ImGui::BulletText("Up / Down / I/K - Pitch up/down (continuous)");
        ImGui::BulletText("Left/Right/J/L  - Yaw left/right (continuous)");
        ImGui::BulletText("Space           - Stop all rotation (zero rates)");
        ImGui::Text("Behavior: 180\xC2\xB0/s\xC2\xB2 acceleration, maintains momentum");
        ImGui::Unindent();
        ImGui::Spacing();

        // Manual Mode
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "MANUAL MODE - Discrete Step Rotation");
        ImGui::Indent();
        ImGui::Text("For testing/debugging quaternions:");
        ImGui::BulletText("W / S   - Pitch up/down (5\xC2\xB0 steps)");
        ImGui::BulletText("A / D   - Roll left/right (5\xC2\xB0 steps)");
        ImGui::BulletText("Q / E   - Yaw left/right (5\xC2\xB0 steps)");
        ImGui::BulletText("R       - Reset to identity quaternion");
        ImGui::Spacing();
        ImGui::Text("Fine control (hold Shift):");
        ImGui::BulletText("Shift + W/A/S/D/Q/E - Same rotations but 1\xC2\xB0 steps");
        ImGui::Text("Behavior: No momentum, rotation only on key press");
        ImGui::Unindent();
        ImGui::Spacing();

        ImGui::Separator();

        // Camera Controls
        ImGui::TextColored(ImVec4(0.8f, 0.5f, 1.0f, 1.0f), "CAMERA CONTROLS");
        ImGui::Indent();
        ImGui::BulletText("Mouse Drag  - Orbit camera around origin");
        ImGui::BulletText("Scroll      - Zoom in/out");
        ImGui::Unindent();
        ImGui::Spacing();

        // Application Controls
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "APPLICATION CONTROLS");
        ImGui::Indent();
        ImGui::BulletText("M       - Toggle rotation mode");
        ImGui::BulletText("ESC     - Exit application");
        ImGui::Unindent();
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::TextDisabled("Tip: Use AUTOMATIC mode to simulate drone flight,");
        ImGui::TextDisabled("     MANUAL mode to test specific quaternion orientations.");

        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            show_help_modal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::Separator();

    // === 3D SCENE ===
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "3D Scene Controls");
    ImGui::SliderFloat("Camera Zoom", &camera.zoom, 0.1f, 100.0f);

    ImGui::Separator();

    glm::vec3 body_rates = glm::vec3(state.angular_rate_deg_per_sec);
    if (ImGui::SliderFloat3("Body Rates (deg/s)", glm::value_ptr(body_rates), -360.0f, 360.0f, "%.1f")) {
        state.angular_rate_deg_per_sec = glm::dvec3(body_rates);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Zero##BodyRates")) {
        state.angular_rate_deg_per_sec = glm::dvec3(0.0, 0.0, 0.0);
    }

    ImGui::Separator();

    bool paused = state.control.paused;
    if (ImGui::Checkbox("Pause Simulation", &paused)) {
        state.control.paused = paused;
    }

    bool use_fixed_dt = state.control.use_fixed_dt;
    if (ImGui::Checkbox("Use Fixed dt", &use_fixed_dt)) {
        state.control.use_fixed_dt = use_fixed_dt;
    }
    if (state.control.use_fixed_dt) {
        double fixed_dt = state.control.fixed_dt;
        if (ImGui::DragScalar("Fixed dt (s)", ImGuiDataType_Double, &fixed_dt, 0.0001, nullptr, nullptr, "%.4f")) {
            fixed_dt = std::clamp(fixed_dt, 1e-5, 0.5);
            state.control.fixed_dt = fixed_dt;
        }
    } else {
        float time_scale = static_cast<float>(state.control.time_scale);
        if (ImGui::SliderFloat("Time Scale", &time_scale, 0.0f, 2.0f, "%.2f")) {
            state.control.time_scale = std::max(0.0f, time_scale);
        }
    }

    ImGui::Separator();
    ImGui::Text("Last dt: %.5f s", state.last_dt);
    ImGui::Text("Sim time: %.2f s", state.time_seconds);
    if (ImGui::Button("Reset Simulation Time")) {
        state.time_seconds = 0.0;
        state.attitude_history.samples.clear();
        state.attitude_history.last_sample_time = -std::numeric_limits<double>::infinity();
    }

    ImGui::Separator();
    if (ImGui::Button("Reset View")) {
        camera.reset();
    }

    ImGui::Separator();
    bool use_legacy_ui = state.control.use_legacy_ui;
    if (ImGui::Checkbox("Show legacy quaternion UI", &use_legacy_ui)) {
        state.control.use_legacy_ui = use_legacy_ui;
    }

    ImGui::Separator();
    float history_window = static_cast<float>(state.attitude_history.window_seconds);
    if (ImGui::SliderFloat("Attitude history window (s)", &history_window, 1.0f, 120.0f, "%.0f")) {
        state.attitude_history.window_seconds = std::clamp(static_cast<double>(history_window), 1.0, 120.0);
    }
    float sample_interval = static_cast<float>(state.attitude_history.sample_interval);
    if (ImGui::SliderFloat("Attitude sample interval (s)", &sample_interval, 0.01f, 0.5f, "%.3f")) {
        state.attitude_history.sample_interval = std::max(0.001, static_cast<double>(sample_interval));
        state.attitude_history.last_sample_time = -std::numeric_limits<double>::infinity();
    }

    ImGui::Separator();
    bool recording = state.attitude_history_video.recording;
    if (ImGui::Checkbox("Record attitude history", &recording)) {
        state.attitude_history_video.recording = recording;
        if (recording) {
            state.attitude_history.last_sample_time = -std::numeric_limits<double>::infinity();
        }
    }

    float playback_speed = static_cast<float>(state.attitude_history_video.playback_speed);
    if (ImGui::SliderFloat("Playback speed", &playback_speed, 0.1f, 4.0f, "%.1fx")) {
        state.attitude_history_video.playback_speed = std::clamp(static_cast<double>(playback_speed), 0.1, 4.0);
    }

    if (ImGui::SliderFloat("Trail length (s)", &state.attitude_history_video.trail_length_seconds, 0.5f, 10.0f, "%.1f")) {
        state.attitude_history_video.trail_length_seconds = std::clamp(state.attitude_history_video.trail_length_seconds, 0.5f, 20.0f);
    }

    if (ImGui::SliderFloat("Trail width (px)", &state.attitude_history_video.trail_width, 1.0f, 8.0f, "%.1f")) {
        state.attitude_history_video.trail_width = std::clamp(state.attitude_history_video.trail_width, 1.0f, 12.0f);
    }

    ImGui::End();
}
