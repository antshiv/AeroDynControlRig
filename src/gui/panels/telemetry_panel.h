/**
 * @file telemetry_panel.h
 * @brief Flight state telemetry display (attitude, rates, time)
 */

#ifndef TELEMETRY_PANEL_H
#define TELEMETRY_PANEL_H

#include "gui/panel.h"

/**
 * @class TelemetryPanel
 * @brief UI panel displaying vehicle flight state
 *
 * Shows real-time telemetry including:
 * - Attitude (quaternion and Euler angles)
 * - Angular rates (deg/s)
 * - Simulation time and frame delta
 * - Additional flight parameters
 *
 * Data is read from SimulationState and displayed using ImGui widgets.
 */
class TelemetryPanel : public Panel {
public:
    const char* name() const override { return "Flight Telemetry"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // TELEMETRY_PANEL_H
