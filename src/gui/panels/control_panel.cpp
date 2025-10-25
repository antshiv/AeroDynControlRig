#include "gui/panels/control_panel.h"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "core/simulation_state.h"
#include "render/camera.h"

#include "imgui.h"

void ControlPanel::draw(SimulationState& state, Camera& camera) {
    if (!ImGui::Begin(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    ImGui::Text("3D Scene Controls");
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

    ImGui::End();
}
