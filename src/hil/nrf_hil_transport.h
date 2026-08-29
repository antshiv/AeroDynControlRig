#ifndef AERODYN_NRF_HIL_TRANSPORT_H
#define AERODYN_NRF_HIL_TRANSPORT_H

#include <cstddef>
#include <cstdint>
#include <string>

extern "C" {
#include "asr_fc/protocol/hil_link.h"
}

class NrfHilTransport {
public:
    NrfHilTransport() = default;
    ~NrfHilTransport();
    NrfHilTransport(const NrfHilTransport&) = delete;
    NrfHilTransport& operator=(const NrfHilTransport&) = delete;

    bool openDevice(const std::string& path, std::string& error);
    void closeDevice();
    bool available() const { return descriptor_ >= 0; }
    const std::string& device() const { return device_; }

    bool exchange(const asr_fc_hil_sensor_guidance_t& request,
                  asr_fc_hil_flight_output_t& response,
                  std::uint32_t timeout_ms,
                  std::string& error);

private:
    bool writeAll(const std::uint8_t* data, std::size_t size,
                  std::uint64_t deadline_ms, std::string& error);

    int descriptor_{-1};
    std::string device_;
    std::uint32_t sequence_{0};
    asr_fc_hil_parser_t parser_{};
};

#endif
