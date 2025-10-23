#ifndef SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_H

#include "core/module.h"

class SensorSimulatorModule : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;

private:
    double gravity_{9.80665};
};

#endif // SENSOR_SIMULATOR_H
