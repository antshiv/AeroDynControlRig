#ifndef AERODYN_NRF_HIL_MODULE_H
#define AERODYN_NRF_HIL_MODULE_H

#include <cstdint>

#include "core/module.h"
#include "hil/nrf_hil_transport.h"

class NrfHilModule : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;

private:
    void clearMotorAuthority(SimulationState& state);
    void failClosed(SimulationState& state, const std::string& reason);
    bool responseWithinContract(const asr_fc_hil_flight_output_t& response,
                                const SimulationState& state) const;

    NrfHilTransport transport_;
    std::uint32_t session_id_{0};
    double accumulator_s_{0.0};
    bool hil_selected_{false};
    bool response_valid_{false};
};

#endif
