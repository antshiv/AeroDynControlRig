#ifndef TELEMETRY_PANEL_H
#define TELEMETRY_PANEL_H

#include "gui/panel.h"

class TelemetryPanel : public Panel {
public:
    const char* name() const override { return "Flight Telemetry"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // TELEMETRY_PANEL_H
