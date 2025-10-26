/**
 * @file plot_widget.h
 * @brief ImPlot wrapper widgets for consistent telemetry plotting
 */

#ifndef GUI_WIDGETS_PLOT_WIDGET_H
#define GUI_WIDGETS_PLOT_WIDGET_H

#include <implot.h>
#include <imgui.h>
#include <vector>
#include <deque>

namespace ui {

/**
 * @brief Plot configuration options
 */
struct PlotConfig {
    const char* title = "Plot";           ///< Plot title
    const char* x_label = "Time (s)";     ///< X-axis label
    const char* y_label = "Value";        ///< Y-axis label
    ImVec2 size = ImVec2(-1, 200);        ///< Plot size (-1 = fill available width)
    double x_min = 0.0;                   ///< X-axis minimum (auto if NaN)
    double x_max = 60.0;                  ///< X-axis maximum (auto if NaN)
    double y_min = -180.0;                ///< Y-axis minimum (auto if NaN)
    double y_max = 180.0;                 ///< Y-axis maximum (auto if NaN)
    bool auto_fit = true;                 ///< Auto-fit axes to data
    bool show_legend = true;              ///< Show legend
    bool show_grid = true;                ///< Show grid lines
};

/**
 * @brief Plot a single time-series line
 *
 * @tparam T Sample type (must have 'timestamp' member)
 * @param label Line label for legend
 * @param samples Data samples (oldest first)
 * @param value_getter Function to extract Y value from sample: [](const T& s) -> double
 * @param color Optional line color (nullptr = auto)
 */
template<typename T, typename ValueGetter>
void PlotLine(const char* label, const std::deque<T>& samples, ValueGetter value_getter, const ImVec4* color = nullptr) {
    if (samples.empty()) return;

    // Extract data into temporary buffers
    std::vector<double> x_data, y_data;
    x_data.reserve(samples.size());
    y_data.reserve(samples.size());

    for (const auto& sample : samples) {
        x_data.push_back(sample.timestamp);
        y_data.push_back(value_getter(sample));
    }

    if (color) {
        ImPlot::PushStyleColor(ImPlotCol_Line, *color);
    }

    // Increase line thickness for better visibility (default is 1.0)
    ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.5f);

    ImPlot::PlotLine(label, x_data.data(), y_data.data(), x_data.size());

    ImPlot::PopStyleVar();

    if (color) {
        ImPlot::PopStyleColor();
    }
}

/**
 * @brief Plot multiple lines with automatic colors
 */
template<typename T>
void PlotAttitudeAngles(const std::deque<T>& samples) {
    if (samples.empty()) return;

    // Define colors as static constants
    static const ImVec4 red(1.0f, 0.3f, 0.3f, 1.0f);
    static const ImVec4 green(0.3f, 1.0f, 0.3f, 1.0f);
    static const ImVec4 blue(0.3f, 0.3f, 1.0f, 1.0f);

    // Roll (red)
    PlotLine("Roll", samples, [](const T& s) { return s.roll * 57.2958; }, &red);

    // Pitch (green)
    PlotLine("Pitch", samples, [](const T& s) { return s.pitch * 57.2958; }, &green);

    // Yaw (blue)
    PlotLine("Yaw", samples, [](const T& s) { return s.yaw * 57.2958; }, &blue);
}

/**
 * @brief Begin a plot with standard configuration
 * @return true if plot is visible and should render
 */
inline bool BeginPlot(const PlotConfig& config) {
    ImPlotFlags flags = ImPlotFlags_None;
    if (!config.show_legend) flags |= ImPlotFlags_NoLegend;

    ImPlotAxisFlags x_flags = ImPlotAxisFlags_None;
    ImPlotAxisFlags y_flags = ImPlotAxisFlags_None;

    if (!config.auto_fit) {
        x_flags |= ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax;
        y_flags |= ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax;
    }

    if (ImPlot::BeginPlot(config.title, config.size, flags)) {
        ImPlot::SetupAxis(ImAxis_X1, config.x_label, x_flags);
        ImPlot::SetupAxis(ImAxis_Y1, config.y_label, y_flags);

        if (!config.auto_fit) {
            ImPlot::SetupAxisLimits(ImAxis_X1, config.x_min, config.x_max, ImPlotCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, config.y_min, config.y_max, ImPlotCond_Always);
        }

        return true;
    }

    return false;
}

/**
 * @brief End plot (must be called if BeginPlot returned true)
 */
inline void EndPlot() {
    ImPlot::EndPlot();
}

/**
 * @brief Complete plot widget with frame
 */
template<typename T, typename ValueGetter>
void TimeSeriesPlot(const char* label, const std::deque<T>& samples, ValueGetter value_getter, const PlotConfig& config) {
    if (BeginPlot(config)) {
        PlotLine(label, samples, value_getter);
        EndPlot();
    }
}

} // namespace ui

#endif // GUI_WIDGETS_PLOT_WIDGET_H
