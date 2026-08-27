#ifndef AERODYN_FLIGHT_CONTROL_H
#define AERODYN_FLIGHT_CONTROL_H

#include "config/aircraft_config.h"
#include "core/module.h"

extern "C" {
#include "mixer.h"
#include "pid.h"
}

class FlightControlModule final : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;

private:
    AircraftSpec aircraft_;
    ControllerCoefficientBundle coefficients_;
    cs_attitude_pid_t controller_{};
    cs_mixer_t mixer_{};
    bool initialized_{false};
    double control_accumulator_s_{0.0};
};

#endif
