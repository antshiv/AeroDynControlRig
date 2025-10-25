#include "gui/panels/telemetry_panel.h"

#include "core/simulation_state.h"
#include "render/camera.h"
#include "attitude/attitude_utils.h"

#include "imgui.h"
#include <vector>

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

        const auto& history_cfg = state.attitude_history_video;
        const auto& history = state.attitude_history.samples;
        if (history.size() >= 2) {
            ImGui::Separator();
            ImGui::Text("Quaternion history (last %.0f s)", state.attitude_history.window_seconds);
            ImGui::SameLine();
            ImGui::TextDisabled("(%zu samples)", history.size());

            const float plot_width = std::max(220.0f, ImGui::GetContentRegionAvail().x);
            const ImVec2 plot_size(plot_width, 58.0f);
            const ImVec4 quat_colors[4] = {
                ImVec4(0.85f, 0.35f, 0.35f, 1.0f),
                ImVec4(0.30f, 0.67f, 0.93f, 1.0f),
                ImVec4(0.38f, 0.85f, 0.47f, 1.0f),
                ImVec4(0.93f, 0.66f, 0.30f, 1.0f),
            };

            auto plot_quaternion_component = [&](const char* label, int index, const ImVec4& color) {
                std::vector<float> values;
                values.reserve(history.size());
                for (const auto& sample : history) {
                    values.push_back(static_cast<float>(sample.quaternion[index]));
                }
                ImGui::PushStyleColor(ImGuiCol_PlotLines, color);
                ImGui::PlotLines(label,
                                 values.data(),
                                 static_cast<int>(values.size()),
                                 0,
                                 nullptr,
                                 -1.0f,
                                 1.0f,
                                 plot_size);
                ImGui::PopStyleColor();
            };

            plot_quaternion_component("w", 0, quat_colors[0]);
            plot_quaternion_component("x", 1, quat_colors[1]);
            plot_quaternion_component("y", 2, quat_colors[2]);
            plot_quaternion_component("z", 3, quat_colors[3]);

            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::Text("Euler history (deg)");
            const ImVec4 euler_colors[3] = {
                ImVec4(0.95f, 0.55f, 0.65f, 1.0f),
                ImVec4(0.65f, 0.80f, 0.35f, 1.0f),
                ImVec4(0.65f, 0.55f, 0.95f, 1.0f),
            };

            auto plot_euler_component = [&](const char* label, auto getter, const ImVec4& color) {
                std::vector<float> values;
                values.reserve(history.size());
                for (const auto& sample : history) {
                    values.push_back(static_cast<float>(rad2deg(getter(sample))));
                }
                ImGui::PushStyleColor(ImGuiCol_PlotLines, color);
                ImGui::PlotLines(label,
                                 values.data(),
                                 static_cast<int>(values.size()),
                                 0,
                                 nullptr,
                                 -180.0f,
                                 180.0f,
                                 plot_size);
                ImGui::PopStyleColor();
            };

            plot_euler_component("Roll", [](const SimulationState::AttitudeSample& sample) { return sample.roll; }, euler_colors[0]);
            plot_euler_component("Pitch", [](const SimulationState::AttitudeSample& sample) { return sample.pitch; }, euler_colors[1]);
            plot_euler_component("Yaw", [](const SimulationState::AttitudeSample& sample) { return sample.yaw; }, euler_colors[2]);

            ImGui::TextDisabled("Recording %s • Trail %.1fs", history_cfg.recording ? "ON" : "OFF", history_cfg.trail_length_seconds);
        } else {
            ImGui::Separator();
            ImGui::TextDisabled("No attitude history captured yet. Enable recording in Control panel.");
        }
    }
    ImGui::End();
}
