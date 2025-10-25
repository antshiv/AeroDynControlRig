/**
 * @file power_panel.h
 * @brief Electrical power consumption monitoring
 */

#ifndef POWER_PANEL_H
#define POWER_PANEL_H

#include <deque>

#include "gui/panel.h"

/**
 * @class PowerPanel
 * @brief UI panel monitoring electrical power consumption
 *
 * Displays:
 * - Bus voltage (V)
 * - Bus current (A)
 * - Instantaneous power (W)
 * - Cumulative energy (J or Wh)
 * - Time-series plot of power consumption
 *
 * Useful for analyzing flight endurance and optimizing control strategies
 * for energy efficiency.
 */
class PowerPanel : public Panel {
public:
    const char* name() const override { return "Power Monitor"; }
    void draw(SimulationState& state, Camera& camera) override;

private:
    static constexpr std::size_t kMaxSamples = 512; ///< Max samples in power history
    std::deque<float> power_history_;               ///< Power consumption time series (W)
};

#endif // POWER_PANEL_H
