#include "gui/panels/telemetry_panel.h"

#include "core/simulation_state.h"
#include "render/camera.h"
#include "attitude/attitude_utils.h"

#include "imgui.h"

void TelemetryPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;
    if (ImGui::Begin(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::BeginTable("telemetry_table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Sim time (s)");
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", state.time_seconds);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Body rates (deg/s)");
            ImGui::TableNextColumn();
            ImGui::Text("Roll %.1f  Pitch %.1f  Yaw %.1f",
                        state.angular_rate_deg_per_sec.x,
                        state.angular_rate_deg_per_sec.y,
                        state.angular_rate_deg_per_sec.z);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Orientation (deg)");
            ImGui::TableNextColumn();
            ImGui::Text("Roll %.1f  Pitch %.1f  Yaw %.1f",
                        rad2deg(state.euler.roll),
                        rad2deg(state.euler.pitch),
                        rad2deg(state.euler.yaw));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Quaternion");
            ImGui::TableNextColumn();
            ImGui::Text("[%.4f, %.4f, %.4f, %.4f]",
                        state.quaternion[0],
                        state.quaternion[1],
                        state.quaternion[2],
                        state.quaternion[3]);

            ImGui::EndTable();
        }
    }
    ImGui::End();
}
