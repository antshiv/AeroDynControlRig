#ifndef MODULES_MISSION_CONTROL_H
#define MODULES_MISSION_CONTROL_H

#include "core/module.h"

class MissionControlModule : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;
};

#endif
