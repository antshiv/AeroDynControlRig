#ifndef ESTIMATOR_PANEL_H
#define ESTIMATOR_PANEL_H

#include "gui/panel.h"

class EstimatorPanel : public Panel {
public:
    const char* name() const override { return "Estimator"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // ESTIMATOR_PANEL_H
