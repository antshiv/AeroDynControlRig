#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include "gui/panel.h"

class ControlPanel : public Panel {
public:
    const char* name() const override { return "Control Panel"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // CONTROL_PANEL_H
