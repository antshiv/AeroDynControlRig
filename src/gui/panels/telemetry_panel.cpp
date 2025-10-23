#include "gui/panels/telemetry_panel.h"

#include "core/simulation_state.h"
#include "render/camera.h"
#include "attitude/attitude_utils.h"

#include "imgui.h"

void TelemetryPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;
    if (ImGui::Begin(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Orientation Data:");
        ImGui::Text("Roll: %.1f", rad2deg(state.euler.roll));
        ImGui::Text("Pitch: %.1f", rad2deg(state.euler.pitch));
        ImGui::Text("Yaw: %.1f", rad2deg(state.euler.yaw));

        ImGui::Separator();
        ImGui::Text("Quaternion:");
        ImGui::Text("w: %.4f", state.quaternion[0]);
        ImGui::Text("x: %.4f", state.quaternion[1]);
        ImGui::Text("y: %.4f", state.quaternion[2]);
        ImGui::Text("z: %.4f", state.quaternion[3]);
    }
    ImGui::End();
}
