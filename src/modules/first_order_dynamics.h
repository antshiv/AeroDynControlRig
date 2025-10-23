#ifndef FIRST_ORDER_DYNAMICS_H
#define FIRST_ORDER_DYNAMICS_H

#include "core/module.h"

class FirstOrderDynamicsModule : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;

private:
    double internal_state_{0.0};
    double time_constant_{1.0};
    double gain_{1.0};
};

#endif // FIRST_ORDER_DYNAMICS_H
