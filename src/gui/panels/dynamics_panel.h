#ifndef DYNAMICS_PANEL_H
#define DYNAMICS_PANEL_H

#include <deque>
#include <utility>

#include "gui/panel.h"

class DynamicsPanel : public Panel {
public:
    const char* name() const override { return "Dynamics"; }
    void draw(SimulationState& state, Camera& camera) override;

private:
    static constexpr std::size_t kMaxSamples = 512;
    std::deque<float> output_history_;
    std::deque<float> input_history_;
    double last_recorded_time_{0.0};

    void appendSample(double time, double input, double output);
};

#endif // DYNAMICS_PANEL_H
