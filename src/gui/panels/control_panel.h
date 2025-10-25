/**
 * @file control_panel.h
 * @brief Simulation playback controls (pause, timestep, speed)
 */

#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include "gui/panel.h"

/**
 * @class ControlPanel
 * @brief UI panel for simulation playback control
 *
 * Provides interactive controls for:
 * - Play/pause simulation
 * - Toggle between real-time and fixed timestep modes
 * - Adjust simulation speed multiplier
 * - Configure fixed timestep duration
 * - Switch between legacy and dashboard UI layouts
 */
class ControlPanel : public Panel {
public:
    const char* name() const override { return "Control Panel"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // CONTROL_PANEL_H
