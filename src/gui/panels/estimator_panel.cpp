#include "gui/panels/estimator_panel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>

#include "attitude/attitude_utils.h"
#include "core/simulation_state.h"
#include "gui/style.h"
#include "gui/widgets/card.h"
#include "gui/widgets/chip.h"
#include "gui/widgets/plot_widget.h"
#include "render/camera.h"

#include "imgui.h"
#include "implot.h"

namespace {
std::string FormatEuler(double roll, double pitch, double yaw) {
    char buffer[96];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "%.1f deg, %.1f deg, %.1f deg",
                  rad2deg(roll),
                  rad2deg(pitch),
                  rad2deg(yaw));
    return std::string(buffer);
}

std::string FormatEulerDelta(double roll, double pitch, double yaw) {
    char buffer[96];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "d %.2f deg, %.2f deg, %.2f deg",
                  rad2deg(roll),
                  rad2deg(pitch),
                  rad2deg(yaw));
    return std::string(buffer);
}

std::string FormatQuaternion(const std::array<double, 4>& q) {
    char buffer[128];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "[%.3f, %.3f, %.3f, %.3f]",
                  q[0],
                  q[1],
                  q[2],
                  q[3]);
    return std::string(buffer);
}
}  // namespace

void EstimatorPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;

    ui::CardOptions options;
    options.min_size = ImVec2(320.0f, 320.0f);

    if (!ui::BeginCard(name(), options, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        ui::EndCard();
        return;
    }

    const ui::Palette& palette = ui::Colors();
    ui::CardHeader("State Estimation", "Kalman Filter");

    const auto& true_euler = state.euler;
    const auto& est_euler = state.estimator.euler;

    std::string true_orientation = FormatEuler(true_euler.roll, true_euler.pitch, true_euler.yaw);
    std::string est_orientation = FormatEuler(est_euler.roll, est_euler.pitch, est_euler.yaw);
    double err_roll = est_euler.roll - true_euler.roll;
    double err_pitch = est_euler.pitch - true_euler.pitch;
    double err_yaw = est_euler.yaw - true_euler.yaw;
    std::string error_orientation = FormatEulerDelta(err_roll, err_pitch, err_yaw);
    double max_error_deg = std::max({std::abs(rad2deg(err_roll)),
                                     std::abs(rad2deg(err_pitch)),
                                     std::abs(rad2deg(err_yaw))});
    std::string estimator_quat = FormatQuaternion(state.estimator.quaternion);

    ui::ValueChip("True Orientation", true_orientation.c_str(), ui::ChipConfig{220.0f});
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ui::ValueChip("Estimated Orientation", est_orientation.c_str(), ui::ChipConfig{220.0f});
    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    ui::ChipConfig error_config;
    error_config.min_width = 220.0f;
    error_config.variant = max_error_deg < 1.0 ? ui::ChipVariant::Positive : ui::ChipVariant::Negative;
    ui::ValueChip("Orientation Error", error_orientation.c_str(), error_config);

    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ui::ValueChip("Estimator Quaternion", estimator_quat.c_str(), ui::ChipConfig{240.0f});

    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
    ImGui::Text("Last dt %.5f s", state.last_dt);
    ImGui::PopStyleColor();

    ImVec2 callout_origin = ImGui::GetCursorScreenPos();
    const float callout_radius = 28.0f;
    callout_origin.x += ImGui::GetContentRegionAvail().x - callout_radius * 2.0f - 4.0f;
    callout_origin.y += 4.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCircleFilled(callout_origin + ImVec2(callout_radius, callout_radius),
                               callout_radius,
                               ImGui::ColorConvertFloat4ToU32(palette.accent_base));
    const ui::FontSet& fonts = ui::Fonts();
    if (fonts.icon) {
        draw_list->AddText(fonts.icon,
                           fonts.icon->FontSize,
                           callout_origin + ImVec2(callout_radius - 12.0f, callout_radius - 12.0f),
                           ImGui::ColorConvertFloat4ToU32(ImVec4(0.976f, 0.992f, 1.0f, 1.0f)),
                           u8"\ue946");
    } else {
        draw_list->AddText(callout_origin + ImVec2(callout_radius - 12.0f, callout_radius - 8.0f),
                           ImGui::ColorConvertFloat4ToU32(ImVec4(0.976f, 0.992f, 1.0f, 1.0f)),
                           "N");
    }
    ImGui::Dummy(ImVec2(callout_radius * 2.0f, callout_radius * 2.0f));

    // === ATTITUDE HISTORY PLOT ===
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Attitude History");

    // Time window controls
    float window_sec = static_cast<float>(state.attitude_history.window_seconds);
    ImGui::Text("Time Window:");
    ImGui::SameLine();
    if (ImGui::Button("10s")) state.attitude_history.window_seconds = 10.0;
    ImGui::SameLine();
    if (ImGui::Button("30s")) state.attitude_history.window_seconds = 30.0;
    ImGui::SameLine();
    if (ImGui::Button("60s")) state.attitude_history.window_seconds = 60.0;
    ImGui::SameLine();
    if (ImGui::SliderFloat("##window", &window_sec, 5.0f, 120.0f, "%.0fs")) {
        state.attitude_history.window_seconds = window_sec;
    }

    // Plot controls
    static bool pause_updates = false;
    ImGui::Checkbox("Pause Updates", &pause_updates);
    ImGui::SameLine();
    if (ImGui::Button("Clear History")) {
        state.attitude_history.samples.clear();
        state.attitude_history.last_sample_time = -std::numeric_limits<double>::infinity();
    }

    if (!state.attitude_history.samples.empty()) {
        ui::PlotConfig plot_config;
        plot_config.title = "Roll/Pitch/Yaw (deg)";
        plot_config.y_label = "Angle (deg)";
        plot_config.size = ImVec2(-1, 250);
        plot_config.y_min = -180.0;
        plot_config.y_max = 180.0;
        plot_config.auto_fit = false;

        // Auto-adjust X axis to show the time window
        if (!state.attitude_history.samples.empty()) {
            plot_config.x_min = state.attitude_history.samples.back().timestamp - state.attitude_history.window_seconds;
            plot_config.x_max = state.attitude_history.samples.back().timestamp;
        }

        if (ui::BeginPlot(plot_config)) {
            ui::PlotAttitudeAngles(state.attitude_history.samples);
            ui::EndPlot();
        }
    } else {
        ImGui::TextDisabled("No attitude history data yet...");
        ImGui::TextDisabled("Press M to toggle rotation mode and start moving the drone");
    }

    // === ANGULAR RATE PLOT ===
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Angular Rates (Body Frame)");

    if (!state.attitude_history.samples.empty()) {
        ui::PlotConfig rate_config;
        rate_config.title = "Angular Rates (deg/s)";
        rate_config.y_label = "Rate (deg/s)";
        rate_config.size = ImVec2(-1, 200);
        rate_config.y_min = -360.0;
        rate_config.y_max = 360.0;
        rate_config.auto_fit = false;

        // Auto-adjust X axis
        if (!state.attitude_history.samples.empty()) {
            rate_config.x_min = state.attitude_history.samples.back().timestamp - state.attitude_history.window_seconds;
            rate_config.x_max = state.attitude_history.samples.back().timestamp;
        }

        if (ui::BeginPlot(rate_config)) {
            // Define colors for angular rates
            static const ImVec4 red(1.0f, 0.3f, 0.3f, 1.0f);
            static const ImVec4 green(0.3f, 1.0f, 0.3f, 1.0f);
            static const ImVec4 blue(0.3f, 0.3f, 1.0f, 1.0f);

            // Plot angular rates (convert rad/s to deg/s)
            ui::PlotLine("Roll Rate", state.attitude_history.samples,
                         [](const SimulationState::AttitudeSample& s) { return s.angular_rate.x * 57.2958; }, &red);
            ui::PlotLine("Pitch Rate", state.attitude_history.samples,
                         [](const SimulationState::AttitudeSample& s) { return s.angular_rate.y * 57.2958; }, &green);
            ui::PlotLine("Yaw Rate", state.attitude_history.samples,
                         [](const SimulationState::AttitudeSample& s) { return s.angular_rate.z * 57.2958; }, &blue);

            ui::EndPlot();
        }
    }

    ui::EndCard();
}
