/**
 * @file dynamics_panel.h
 * @brief First-order dynamics configuration and plotting
 */

#ifndef DYNAMICS_PANEL_H
#define DYNAMICS_PANEL_H

#include <deque>
#include <utility>

#include "gui/panel.h"

/**
 * @class DynamicsPanel
 * @brief UI panel for configuring and visualizing first-order dynamics
 *
 * Provides:
 * - Configuration controls (gain, time constant, input mode)
 * - Real-time plotting of input/output time series
 * - History buffer with configurable sample count
 *
 * Useful for:
 * - Tuning system parameters
 * - Visualizing step/sinusoidal responses
 * - Debugging dynamics module behavior
 */
class DynamicsPanel : public Panel {
public:
    const char* name() const override { return "Dynamics"; }
    void draw(SimulationState& state, Camera& camera) override;

private:
    static constexpr std::size_t kMaxSamples = 512; ///< Max samples in history buffer
    std::deque<float> output_history_;              ///< Output time series
    std::deque<float> input_history_;               ///< Input time series
    double last_recorded_time_{0.0};                ///< Last sample timestamp

    /**
     * @brief Add a sample to the history buffers
     * @param time Current simulation time
     * @param input Current input value
     * @param output Current output value
     */
    void appendSample(double time, double input, double output);
};

#endif // DYNAMICS_PANEL_H
