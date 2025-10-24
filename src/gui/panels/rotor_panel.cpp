#include "gui/panels/rotor_panel.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "core/simulation_state.h"
#include "render/camera.h"
#include "gui/style.h"
#include "gui/widgets/card.h"
#include "gui/widgets/chip.h"

#include "imgui.h"

namespace {
constexpr float kChartHeight = 140.0f;
constexpr float kColumnWidth = 80.0f;
constexpr float kBarWidth = 20.0f;
constexpr float kBarGap = 8.0f;
constexpr float kColumnSpacing = 26.0f;
}

void RotorPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;

    ui::CardOptions options;
    options.min_size = ImVec2(320.0f, 360.0f);
    options.allow_scrollbar = false;

    if (!ui::BeginCard(name(), options, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        ui::EndCard();
        return;
    }

    const ui::Palette& palette = ui::Colors();
    const ui::FontSet& fonts = ui::Fonts();

    float rpm_sum = 0.0f;
    for (double rpm : state.rotor.rpm) {
        rpm_sum += static_cast<float>(rpm);
    }
    float avg_rpm = state.rotor.rpm.empty() ? 0.0f : rpm_sum / static_cast<float>(state.rotor.rpm.size());

    static float previous_avg = 0.0f;
    float delta_percent = 0.0f;
    if (previous_avg > 1.0f) {
        delta_percent = ((avg_rpm - previous_avg) / previous_avg) * 100.0f;
    }
    previous_avg = avg_rpm;

    char badge_label[32];
    std::snprintf(badge_label, sizeof(badge_label), "Real-time %+0.1f%%", delta_percent);
    const ImVec4& badge_color = delta_percent >= 0.0f ? palette.success : palette.danger;
    ui::CardHeader("Rotor Dynamics", badge_label, &badge_color);

    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
    ImGui::TextUnformatted("Rotor RPM");
    ImGui::PopStyleColor();

    if (fonts.heading) {
        ImGui::PushFont(fonts.heading);
    }
    ImGui::Text("%.0f RPM", avg_rpm);
    if (fonts.heading) {
        ImGui::PopFont();
    }

    char thrust_value[32];
    std::snprintf(thrust_value, sizeof(thrust_value), "%.1f N", state.rotor.total_thrust_newton);
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ui::ValueChip("Total Thrust", thrust_value, ui::ChipConfig{140.0f});
    ImGui::SameLine(0.0f, 10.0f);
    char power_value[32];
    std::snprintf(power_value, sizeof(power_value), "%.0f W", state.rotor.total_power_watt);
    ui::ValueChip("Total Power", power_value, ui::ChipConfig{140.0f});

    ImGui::Dummy(ImVec2(0.0f, 12.0f));

    ImVec2 chart_origin = ImGui::GetCursorScreenPos();
    std::size_t rotor_count = state.rotor.rpm.size();
    float chart_width = rotor_count == 0
                            ? 0.0f
                            : rotor_count * kColumnWidth + (rotor_count - 1) * kColumnSpacing;
    float chart_draw_width = std::max(chart_width, 1.0f);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec4 grid_color = ImVec4(palette.card_border.x, palette.card_border.y, palette.card_border.z, 0.35f);
    const int grid_lines = 4;
    for (int i = 0; i <= grid_lines; ++i) {
        float y = chart_origin.y + kChartHeight - (kChartHeight / grid_lines) * static_cast<float>(i);
        draw_list->AddLine(ImVec2(chart_origin.x, y),
                           ImVec2(chart_origin.x + chart_draw_width, y),
                           ImGui::ColorConvertFloat4ToU32(grid_color));
    }

    for (std::size_t i = 0; i < rotor_count; ++i) {
        float column_offset = chart_origin.x + static_cast<float>(i) * (kColumnWidth + kColumnSpacing);
        float rpm = static_cast<float>(state.rotor.rpm[i]);
        float thrust = static_cast<float>(state.rotor.thrust_newton[i]);

        float rpm_norm = std::clamp(rpm / 2200.0f, 0.0f, 1.0f);
        float thrust_norm = std::clamp(thrust / 8.0f, 0.0f, 1.0f);

        float rpm_height = kChartHeight * rpm_norm;
        float thrust_height = kChartHeight * thrust_norm;

        ImVec2 rpm_min(column_offset,
                       chart_origin.y + kChartHeight - rpm_height);
        ImVec2 rpm_max(column_offset + kBarWidth,
                       chart_origin.y + kChartHeight);

        ImVec2 thrust_min(column_offset + kBarWidth + kBarGap,
                          chart_origin.y + kChartHeight - thrust_height);
        ImVec2 thrust_max(column_offset + 2 * kBarWidth + kBarGap,
                          chart_origin.y + kChartHeight);

        draw_list->AddRectFilled(rpm_min, rpm_max, ImGui::ColorConvertFloat4ToU32(palette.accent_base), 8.0f);
        draw_list->AddRectFilled(thrust_min, thrust_max, ImGui::ColorConvertFloat4ToU32(palette.success), 8.0f);

        std::string label = "R" + std::to_string(i + 1);
        ImVec2 label_pos = ImVec2(column_offset, chart_origin.y + kChartHeight + 10.0f);
        draw_list->AddText(label_pos, ImGui::ColorConvertFloat4ToU32(palette.text_muted), label.c_str());

        char rpm_label[32];
        std::snprintf(rpm_label, sizeof(rpm_label), "%.0f", rpm);
        ImVec2 rpm_label_pos = ImVec2(column_offset,
                                      rpm_min.y - 22.0f);
        draw_list->AddText(rpm_label_pos,
                           ImGui::ColorConvertFloat4ToU32(palette.text_primary),
                           rpm_label);
    }

    ImGui::Dummy(ImVec2(chart_draw_width, kChartHeight + 40.0f));

    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
    ImGui::Text("Ct %.2e   Cq %.2e   Arm %.2f m",
                state.rotor_config.thrust_coefficient,
                state.rotor_config.torque_coefficient,
                state.rotor_config.arm_length_m);
    ImGui::PopStyleColor();

    ui::EndCard();
}
