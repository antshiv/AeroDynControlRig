#ifndef AERODYN_JOYSTICK_PANEL_H
#define AERODYN_JOYSTICK_PANEL_H

#include "gui/panel.h"

class JoystickPanel final : public Panel {
public:
    const char* name() const override { return "F310 Flight Controls"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif
