#ifndef SENSOR_PANEL_H
#define SENSOR_PANEL_H

#include "gui/panel.h"

class SensorPanel : public Panel {
public:
    const char* name() const override { return "Sensor Suite"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // SENSOR_PANEL_H
