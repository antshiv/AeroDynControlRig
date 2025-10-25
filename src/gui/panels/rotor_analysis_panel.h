/**
 * @file rotor_analysis_panel.h
 * @brief Panel for analyzing individual rotor performance metrics
 */

#ifndef GUI_PANELS_ROTOR_ANALYSIS_PANEL_H
#define GUI_PANELS_ROTOR_ANALYSIS_PANEL_H

#include <deque>
#include "gui/panel.h"
#include "core/simulation_state.h"

/**
 * @brief Rotor dynamics analysis panel with per-motor telemetry
 *
 * Displays real-time and historical performance data for each of the 4 motors:
 * - Thrust output over time
 * - RPM (revolutions per minute)
 * - Power consumption
 * - Temperature monitoring
 * - Raw telemetry data table
 * - CSV export functionality
 */
class RotorAnalysisPanel : public Panel {
public:
    RotorAnalysisPanel() = default;
    ~RotorAnalysisPanel() override = default;

    void draw(SimulationState& state, Camera& camera) override;
    const char* name() const override { return "Rotor Analysis"; }

private:
    int selected_rotor_ = 0;  ///< Currently selected rotor (0-3)
    float time_window_ = 30.0f;  ///< Plot time window in seconds
    bool show_export_modal_ = false;  ///< CSV export dialog visibility

    /**
     * @brief Get samples deque for selected rotor
     */
    const std::deque<SimulationState::RotorSample>& getSamples(const SimulationState& state) const;

    /**
     * @brief Draw sidebar with rotor selection tabs
     */
    void drawRotorSelector(SimulationState& state);

    /**
     * @brief Draw thrust plot for selected rotor
     */
    void drawThrustPlot(const SimulationState& state);

    /**
     * @brief Draw RPM plot for selected rotor
     */
    void drawRPMPlot(const SimulationState& state);

    /**
     * @brief Draw power consumption plot
     */
    void drawPowerPlot(const SimulationState& state);

    /**
     * @brief Draw temperature monitoring plot
     */
    void drawTemperaturePlot(const SimulationState& state);

    /**
     * @brief Draw raw data table
     */
    void drawDataTable(const SimulationState& state);

    /**
     * @brief Export rotor data to CSV file
     */
    void exportToCSV(const SimulationState& state);
};

#endif // GUI_PANELS_ROTOR_ANALYSIS_PANEL_H
