#include "gui/panels/rotor_analysis_panel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>

#include "core/simulation_state.h"
#include "gui/style.h"
#include "gui/widgets/card.h"
#include "gui/widgets/chip.h"
#include "gui/widgets/plot_widget.h"
#include "render/camera.h"

#include "imgui.h"
#include "implot.h"

const std::deque<SimulationState::RotorSample>& RotorAnalysisPanel::getSamples(const SimulationState& state) const {
    switch (selected_rotor_) {
        case 0: return state.rotor_history.rotor1_samples;
        case 1: return state.rotor_history.rotor2_samples;
        case 2: return state.rotor_history.rotor3_samples;
        case 3: return state.rotor_history.rotor4_samples;
        default: return state.rotor_history.rotor1_samples;
    }
}

void RotorAnalysisPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;

    ui::CardOptions options;
    options.min_size = ImVec2(640.0f, 480.0f);

    if (!ui::BeginCard(name(), options, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        ui::EndCard();
        return;
    }

    const ui::Palette& palette = ui::Colors();
    ui::CardHeader("Rotor Performance", "Motor Telemetry");

    // === ROTOR SELECTOR SIDEBAR ===
    drawRotorSelector(state);

    ImGui::SameLine();
    ImGui::BeginGroup();

    // === TIME WINDOW CONTROLS ===
    ImGui::Text("Time Window:");
    ImGui::SameLine();
    if (ImGui::Button("10s##rotor")) time_window_ = 10.0f;
    ImGui::SameLine();
    if (ImGui::Button("30s##rotor")) time_window_ = 30.0f;
    ImGui::SameLine();
    if (ImGui::Button("60s##rotor")) time_window_ = 60.0f;
    ImGui::SameLine();
    ImGui::SliderFloat("##rotor_window", &time_window_, 5.0f, 120.0f, "%.0fs");

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20.0f, 0.0f));
    ImGui::SameLine();
    if (ImGui::Button("Export CSV")) {
        show_export_modal_ = true;
    }

    ImGui::Separator();
    ImGui::Spacing();

    // === ROTOR STATUS CHIPS ===
    const auto& samples = getSamples(state);
    if (!samples.empty()) {
        const auto& latest = samples.back();

        char rpm_str[32], thrust_str[32], power_str[32], temp_str[32];
        std::snprintf(rpm_str, sizeof(rpm_str), "%.0f RPM", latest.rpm);
        std::snprintf(thrust_str, sizeof(thrust_str), "%.2f N", latest.thrust);
        std::snprintf(power_str, sizeof(power_str), "%.1f W", latest.power);
        std::snprintf(temp_str, sizeof(temp_str), "%.1f °C", latest.temperature);

        ui::ValueChip("RPM", rpm_str, ui::ChipConfig{120.0f});
        ImGui::SameLine();
        ui::ValueChip("Thrust", thrust_str, ui::ChipConfig{120.0f});
        ImGui::SameLine();
        ui::ValueChip("Power", power_str, ui::ChipConfig{120.0f});
        ImGui::SameLine();

        ui::ChipConfig temp_config;
        temp_config.min_width = 120.0f;
        temp_config.variant = latest.temperature > 80.0f ? ui::ChipVariant::Negative : ui::ChipVariant::Neutral;
        ui::ValueChip("Temp", temp_str, temp_config);
    } else {
        ImGui::TextDisabled("No rotor data available yet...");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // === PLOTS ===
    drawThrustPlot(state);
    ImGui::Spacing();
    drawRPMPlot(state);
    ImGui::Spacing();
    drawPowerPlot(state);
    ImGui::Spacing();
    drawTemperaturePlot(state);
    ImGui::Spacing();

    ImGui::Separator();
    drawDataTable(state);

    ImGui::EndGroup();

    // === EXPORT MODAL ===
    if (show_export_modal_) {
        ImGui::OpenPopup("Export Rotor Data");
    }

    if (ImGui::BeginPopupModal("Export Rotor Data", &show_export_modal_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Export rotor telemetry to CSV file?");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Motor %d", selected_rotor_ + 1);
        ImGui::Text("Sample count: %zu", getSamples(state).size());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Export", ImVec2(120, 0))) {
            exportToCSV(state);
            show_export_modal_ = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show_export_modal_ = false;
        }
        ImGui::EndPopup();
    }

    ui::EndCard();
}

void RotorAnalysisPanel::drawRotorSelector(SimulationState& state) {
    (void)state;

    const ui::Palette& palette = ui::Colors();
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
    ImGui::Text("Select Motor");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Tab-style buttons for rotor selection
    const char* rotor_labels[] = {"Rotor 1", "Rotor 2", "Rotor 3", "Rotor 4"};
    const ImVec4 colors[] = {
        ImVec4(1.0f, 0.3f, 0.3f, 1.0f),  // Red
        ImVec4(0.3f, 1.0f, 0.3f, 1.0f),  // Green
        ImVec4(0.3f, 0.3f, 1.0f, 1.0f),  // Blue
        ImVec4(1.0f, 0.8f, 0.3f, 1.0f)   // Yellow
    };

    for (int i = 0; i < 4; ++i) {
        if (selected_rotor_ == i) {
            ImGui::PushStyleColor(ImGuiCol_Button, colors[i]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors[i]);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors[i]);
        } else {
            ImVec4 dim_color = colors[i];
            dim_color.w = 0.3f;
            ImGui::PushStyleColor(ImGuiCol_Button, dim_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors[i]);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors[i]);
        }

        if (ImGui::Button(rotor_labels[i], ImVec2(100.0f, 40.0f))) {
            selected_rotor_ = i;
        }

        ImGui::PopStyleColor(3);
        ImGui::Spacing();
    }

    ImGui::EndGroup();
}

void RotorAnalysisPanel::drawThrustPlot(const SimulationState& state) {
    const auto& samples = getSamples(state);

    if (samples.empty()) {
        ImGui::TextDisabled("No thrust data available");
        return;
    }

    ui::PlotConfig config;
    config.title = "Thrust vs Time";
    config.y_label = "Thrust (N)";
    config.size = ImVec2(-1, 180);
    config.y_min = 0.0;
    config.y_max = 10.0;
    config.auto_fit = false;

    if (!samples.empty()) {
        config.x_min = samples.back().timestamp - time_window_;
        config.x_max = samples.back().timestamp;
    }

    if (ui::BeginPlot(config)) {
        // Cyan color for thrust
        static const ImVec4 cyan(0.2f, 0.8f, 0.9f, 1.0f);
        ui::PlotLine("Thrust", samples,
                     [](const SimulationState::RotorSample& s) { return static_cast<double>(s.thrust); },
                     &cyan);
        ui::EndPlot();
    }
}

void RotorAnalysisPanel::drawRPMPlot(const SimulationState& state) {
    const auto& samples = getSamples(state);

    if (samples.empty()) {
        ImGui::TextDisabled("No RPM data available");
        return;
    }

    ui::PlotConfig config;
    config.title = "RPM vs Time";
    config.y_label = "RPM";
    config.size = ImVec2(-1, 180);
    config.y_min = 0.0;
    config.y_max = 10000.0;
    config.auto_fit = false;

    if (!samples.empty()) {
        config.x_min = samples.back().timestamp - time_window_;
        config.x_max = samples.back().timestamp;
    }

    if (ui::BeginPlot(config)) {
        // Orange/yellow color for RPM
        static const ImVec4 orange(1.0f, 0.7f, 0.2f, 1.0f);
        ui::PlotLine("RPM", samples,
                     [](const SimulationState::RotorSample& s) { return static_cast<double>(s.rpm); },
                     &orange);
        ui::EndPlot();
    }
}

void RotorAnalysisPanel::drawPowerPlot(const SimulationState& state) {
    const auto& samples = getSamples(state);

    if (samples.empty()) {
        ImGui::TextDisabled("No power data available");
        return;
    }

    ui::PlotConfig config;
    config.title = "Power vs Time";
    config.y_label = "Power (W)";
    config.size = ImVec2(-1, 180);
    config.y_min = 0.0;
    config.y_max = 500.0;
    config.auto_fit = false;

    if (!samples.empty()) {
        config.x_min = samples.back().timestamp - time_window_;
        config.x_max = samples.back().timestamp;
    }

    if (ui::BeginPlot(config)) {
        // Cyan color for power
        static const ImVec4 cyan(0.2f, 0.8f, 0.9f, 1.0f);
        ui::PlotLine("Power", samples,
                     [](const SimulationState::RotorSample& s) { return static_cast<double>(s.power); },
                     &cyan);
        ui::EndPlot();
    }
}

void RotorAnalysisPanel::drawTemperaturePlot(const SimulationState& state) {
    const auto& samples = getSamples(state);

    if (samples.empty()) {
        ImGui::TextDisabled("No temperature data available");
        return;
    }

    ui::PlotConfig config;
    config.title = "Temperature vs Time";
    config.y_label = "Temperature (°C)";
    config.size = ImVec2(-1, 180);
    config.y_min = 0.0;
    config.y_max = 100.0;
    config.auto_fit = false;

    if (!samples.empty()) {
        config.x_min = samples.back().timestamp - time_window_;
        config.x_max = samples.back().timestamp;
    }

    if (ui::BeginPlot(config)) {
        // Orange color for temperature
        static const ImVec4 orange(1.0f, 0.5f, 0.2f, 1.0f);
        ui::PlotLine("Temperature", samples,
                     [](const SimulationState::RotorSample& s) { return static_cast<double>(s.temperature); },
                     &orange);
        ui::EndPlot();
    }
}

void RotorAnalysisPanel::drawDataTable(const SimulationState& state) {
    const auto& samples = getSamples(state);
    const ui::Palette& palette = ui::Colors();

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Raw Telemetry Data");

    if (samples.empty()) {
        ImGui::TextDisabled("No data captured yet");
        return;
    }

    // Enhanced table with better padding and sizing
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(14.0f, 10.0f));
    if (ImGui::BeginTable("RotorData", 6,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchSame,
                          ImVec2(0, 200))) {
        ImGui::TableSetupColumn("Timestamp");
        ImGui::TableSetupColumn("RPM");
        ImGui::TableSetupColumn("Thrust (N)");
        ImGui::TableSetupColumn("Power (W)");
        ImGui::TableSetupColumn("Temp (°C)");
        ImGui::TableSetupColumn("Voltage (V)");

        // Enhanced header with better contrast
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, palette.card_header);
        ImGui::PushStyleColor(ImGuiCol_Text, palette.text_primary);
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor(2);

        // Show last 50 samples
        size_t start_idx = samples.size() > 50 ? samples.size() - 50 : 0;
        for (size_t i = start_idx; i < samples.size(); ++i) {
            const auto& sample = samples[i];
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%.3f", sample.timestamp);

            ImGui::TableNextColumn();
            ImGui::Text("%.0f", sample.rpm);

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", sample.thrust);

            ImGui::TableNextColumn();
            ImGui::Text("%.1f", sample.power);

            // Color-code temperature based on value
            ImGui::TableNextColumn();
            const ui::Palette& palette = ui::Colors();
            ImVec4 temp_color = palette.text_primary;
            if (sample.temperature > 80.0f) {
                temp_color = palette.danger;  // Red for high temp
            } else if (sample.temperature > 60.0f) {
                temp_color = palette.warning; // Yellow for warm
            } else {
                temp_color = palette.success; // Green for good
            }
            ImGui::TextColored(temp_color, "%.1f", sample.temperature);

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", sample.voltage);
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar(); // Pop CellPadding
}

void RotorAnalysisPanel::exportToCSV(const SimulationState& state) {
    const auto& samples = getSamples(state);

    if (samples.empty()) {
        return;
    }

    char filename[128];
    std::snprintf(filename, sizeof(filename), "rotor_%d_telemetry.csv", selected_rotor_ + 1);

    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }

    // Write CSV header
    file << "Timestamp,RPM,Thrust_N,Power_W,Temperature_C,Voltage_V,Current_A\n";

    // Write data rows
    for (const auto& sample : samples) {
        file << sample.timestamp << ","
             << sample.rpm << ","
             << sample.thrust << ","
             << sample.power << ","
             << sample.temperature << ","
             << sample.voltage << ","
             << sample.current << "\n";
    }

    file.close();
}
