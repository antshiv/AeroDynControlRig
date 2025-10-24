#ifndef ROTOR_PANEL_H
#define ROTOR_PANEL_H

#include <array>

#include "gui/panel.h"

class RotorPanel : public Panel {
public:
    const char* name() const override { return "Rotor Dynamics"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // ROTOR_PANEL_H
