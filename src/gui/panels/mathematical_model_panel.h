#ifndef AERODYN_MATHEMATICAL_MODEL_PANEL_H
#define AERODYN_MATHEMATICAL_MODEL_PANEL_H

#include "gui/panel.h"

class MathematicalModelPanel final : public Panel {
public:
    const char* name() const override { return "Aircraft Mathematical Model"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif
