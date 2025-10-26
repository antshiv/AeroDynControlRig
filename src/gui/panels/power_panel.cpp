#include "gui/panels/power_panel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "core/simulation_state.h"
#include "render/camera.h"
#include "gui/style.h"
#include "gui/widgets/card.h"
#include "gui/widgets/chip.h"

#include "imgui.h"

namespace {
constexpr float kChartHeight = 160.0f;
}

void PowerPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;

    power_history_.emplace_back(static_cast<float>(state.rotor.total_power_watt));
    while (power_history_.size() > kMaxSamples) {
        power_history_.pop_front();
    }

    ui::CardOptions options;
    options.min_size = ImVec2(320.0f, 360.0f);

    if (!ui::BeginCard(name(), options, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        ui::EndCard();
        return;
    }

    const ui::Palette& palette = ui::Colors();
    const ui::FontSet& fonts = ui::Fonts();

    float latest_power = power_history_.empty() ? 0.0f : power_history_.back();
    float earliest_power = power_history_.empty() ? latest_power : power_history_.front();
    float delta_percent = 0.0f;
    if (earliest_power > 1.0f) {
        delta_percent = ((latest_power - earliest_power) / earliest_power) * 100.0f;
    }

    ui::CardHeader("Power Consumption", "Last 5 Minutes");

    // Use large metrics font for primary value
    if (fonts.metrics) {
        ImGui::PushFont(fonts.metrics);
    }
    ImGui::Text("%.0f W", latest_power);
    if (fonts.metrics) {
        ImGui::PopFont();
    }

    ImGui::SameLine(0.0f, 18.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, delta_percent >= 0.0f ? palette.success : palette.danger);
    if (fonts.icon) {
        const char* arrow = delta_percent >= 0.0f ? u8"\ue5ce" : u8"\ue5cf";
        ImGui::Text("%s %+0.1f%%", arrow, delta_percent);
    } else {
        ImGui::Text("%+0.1f%%", delta_percent);
    }
    ImGui::PopStyleColor();

    char voltage_label[32];
    std::snprintf(voltage_label, sizeof(voltage_label), "%.1f V", state.power.bus_voltage);
    char current_label[32];
    std::snprintf(current_label, sizeof(current_label), "%.2f A", state.power.bus_current);
    char energy_label[32];
    std::snprintf(energy_label, sizeof(energy_label), "%.1f J", state.power.energy_joule);

    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    ui::ValueChip("Bus Voltage", voltage_label, ui::ChipConfig{130.0f});
    ImGui::SameLine(0.0f, 8.0f);
    ui::ValueChip("Bus Current", current_label, ui::ChipConfig{130.0f});
    ImGui::SameLine(0.0f, 8.0f);
    ui::ValueChip("Energy", energy_label, ui::ChipConfig{130.0f});

    ImGui::Dummy(ImVec2(0.0f, 12.0f));

    ImVec2 chart_pos = ImGui::GetCursorScreenPos();
    ImVec2 chart_size = ImVec2(std::max(220.0f, ImGui::GetContentRegionAvail().x), kChartHeight);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRectFilled(chart_pos,
                             chart_pos + chart_size,
                             ImGui::ColorConvertFloat4ToU32(ImVec4(palette.card_header.x,
                                                                   palette.card_header.y,
                                                                   palette.card_header.z,
                                                                   0.45f)),
                             16.0f);

    if (power_history_.size() >= 2) {
        std::vector<float> samples(power_history_.begin(), power_history_.end());
        float min_power = *std::min_element(samples.begin(), samples.end());
        float max_power = *std::max_element(samples.begin(), samples.end());
        if (std::abs(max_power - min_power) < 1e-3f) {
            max_power = min_power + 1.0f;
        }

        const float range = max_power - min_power;
        const std::size_t count = samples.size();
        const float step = count > 1 ? chart_size.x / static_cast<float>(count - 1) : chart_size.x;

        std::vector<ImVec2> line_points;
        line_points.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            float normalized = (samples[i] - min_power) / range;
            float x = chart_pos.x + step * static_cast<float>(i);
            float y = chart_pos.y + chart_size.y - normalized * chart_size.y;
            line_points.emplace_back(x, y);
        }

        std::vector<ImVec2> fill_points = line_points;
        fill_points.emplace_back(chart_pos.x + chart_size.x, chart_pos.y + chart_size.y);
        fill_points.emplace_back(chart_pos.x, chart_pos.y + chart_size.y);

        ImU32 fill_color = ImGui::ColorConvertFloat4ToU32(ImVec4(palette.accent_base.x,
                                                                 palette.accent_base.y,
                                                                 palette.accent_base.z,
                                                                 0.18f));
        draw_list->AddConvexPolyFilled(fill_points.data(), static_cast<int>(fill_points.size()), fill_color);

        ImU32 line_color = ImGui::ColorConvertFloat4ToU32(palette.accent_base);
        draw_list->AddPolyline(line_points.data(),
                               static_cast<int>(line_points.size()),
                               line_color,
                               false,
                               2.5f);

        ImVec2 last_point = line_points.back();
        draw_list->AddCircleFilled(last_point, 4.0f, line_color);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
        ImGui::SetCursorScreenPos(chart_pos + ImVec2(16.0f, 16.0f));
        ImGui::TextUnformatted("Collecting samples...");
        ImGui::PopStyleColor();
    }

    ImGui::Dummy(chart_size);

    ui::EndCard();
}
