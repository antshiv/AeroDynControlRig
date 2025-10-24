#ifndef POWER_PANEL_H
#define POWER_PANEL_H

#include <deque>

#include "gui/panel.h"

class PowerPanel : public Panel {
public:
    const char* name() const override { return "Power Monitor"; }
    void draw(SimulationState& state, Camera& camera) override;

private:
    static constexpr std::size_t kMaxSamples = 512;
    std::deque<float> power_history_;
};

#endif // POWER_PANEL_H
