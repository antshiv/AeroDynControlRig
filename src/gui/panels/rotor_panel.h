/**
 * @file rotor_panel.h
 * @brief Rotor performance telemetry (RPM, thrust, torque)
 */

#ifndef ROTOR_PANEL_H
#define ROTOR_PANEL_H

#include <array>

#include "gui/panel.h"

/**
 * @class RotorPanel
 * @brief UI panel displaying rotor performance metrics
 *
 * Shows for each rotor:
 * - RPM (revolutions per minute)
 * - Thrust (Newtons)
 * - Torque (Newton-meters)
 * - Total thrust and power consumption
 *
 * Visualizes data from RotorTelemetryModule using gauges, bar charts,
 * or numeric readouts.
 */
class RotorPanel : public Panel {
public:
    const char* name() const override { return "Rotor Dynamics"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // ROTOR_PANEL_H
