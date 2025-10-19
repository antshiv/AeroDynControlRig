#ifndef MODULE_H
#define MODULE_H

class SimulationState;

class Module {
public:
    virtual ~Module() = default;
    virtual void initialize(SimulationState& state) {}
    virtual void update(double dt, SimulationState& state) = 0;
};

#endif // MODULE_H
