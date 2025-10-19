#ifndef QUATERNION_DEMO_H
#define QUATERNION_DEMO_H

#include "core/module.h"

class QuaternionDemoModule : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;
};

#endif // QUATERNION_DEMO_H
