#ifndef ROTOR_TELEMETRY_H
#define ROTOR_TELEMETRY_H

#include "core/module.h"

class RotorTelemetryModule : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;

private:
    double base_rpm_{1500.0};
    double phase_{0.0};
};

#endif // ROTOR_TELEMETRY_H
