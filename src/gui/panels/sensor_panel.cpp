#include "gui/panels/sensor_panel.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <string>

#include "core/simulation_state.h"
#include "gui/style.h"
#include "gui/widgets/card.h"
#include "gui/widgets/chip.h"
#include "render/camera.h"

#include "imgui.h"

namespace {
ui::ChipVariant VariantForValue(float value, float warning_threshold) {
    float magnitude = std::abs(value);
    if (magnitude > warning_threshold) {
        return ui::ChipVariant::Negative;
    }
    if (magnitude < warning_threshold * 0.4f) {
        return ui::ChipVariant::Positive;
    }
    return ui::ChipVariant::Neutral;
}

void VectorRow(const char* label,
               const glm::vec3& vec,
               const char* units,
               float warning_threshold) {
    const ui::Palette& palette = ui::Colors();
    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    const char axes[3] = {'X', 'Y', 'Z'};
    for (int i = 0; i < 3; ++i) {
        float value = (&vec.x)[i];
        char buffer[48];
        std::snprintf(buffer, sizeof(buffer), "%.2f %s", value, units);
        ui::ChipConfig config;
        config.min_width = 120.0f;
        config.variant = VariantForValue(value, warning_threshold);

        char label_buffer[16];
        std::snprintf(label_buffer, sizeof(label_buffer), "%c", axes[i]);
        ui::ValueChip(label_buffer, buffer, config);
        if (i < 2) {
            ImGui::SameLine(0.0f, 10.0f);
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 8.0f));
}

template <typename Array>
void ArrayRow(const char* label,
              const Array& values,
              const char* units,
              float warning_threshold) {
    const ui::Palette& palette = ui::Colors();
    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    for (std::size_t i = 0; i < values.size(); ++i) {
        float value = static_cast<float>(values[i]);
        char buffer[48];
        std::snprintf(buffer, sizeof(buffer), "%.2f %s", value, units);

        ui::ChipConfig config;
        config.min_width = 110.0f;
        config.variant = VariantForValue(value, warning_threshold);

        char label_buffer[16];
        std::snprintf(label_buffer, sizeof(label_buffer), "R%zu", i + 1);
        ui::ValueChip(label_buffer, buffer, config);
        if (i + 1 < values.size()) {
            ImGui::SameLine(0.0f, 8.0f);
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 8.0f));
}
}  // namespace

void SensorPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;

    ui::CardOptions options;
    options.min_size = ImVec2(320.0f, 320.0f);

    if (!ui::BeginCard(name(), options, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        ui::EndCard();
        return;
    }

    const ui::Palette& palette = ui::Colors();
    ui::CardHeader("INS Data", "Sensor Suite");

    VectorRow("Gyroscope", state.sensor.gyro_rad_s, "rad/s", 1.2f);
    VectorRow("Accelerometer", state.sensor.accel_mps2, "m/s^2", 9.0f);
    ArrayRow("Rotor Thrust", state.rotor.thrust_newton, "N", 6.0f);
    ArrayRow("Rotor RPM", state.rotor.rpm, "RPM", 1800.0f);

    ImVec2 callout_origin = ImGui::GetCursorScreenPos();
    float callout_radius = 26.0f;
    callout_origin.x += ImGui::GetContentRegionAvail().x - callout_radius * 2.0f - 4.0f;
    callout_origin.y += 6.0f;

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
                           u8"\ue63e");
    } else {
        draw_list->AddText(callout_origin + ImVec2(callout_radius - 12.0f, callout_radius - 8.0f),
                           ImGui::ColorConvertFloat4ToU32(ImVec4(0.976f, 0.992f, 1.0f, 1.0f)),
                           "RF");
    }
    ImGui::Dummy(ImVec2(callout_radius * 2.0f, callout_radius * 2.0f));

    ui::EndCard();
}
