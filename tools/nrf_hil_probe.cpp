#include <cstdint>
#include <iostream>
#include <string>

#include "hil/nrf_hil_transport.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: aerodyn_nrf_hil_probe <serial-device>\n";
        return 2;
    }

    NrfHilTransport transport;
    std::string error;
    if (!transport.openDevice(argv[1], error)) {
        std::cerr << error << '\n';
        return 1;
    }

    asr_fc_hil_sensor_guidance_t request{};
    request.session_id = 0x50524f42u;
    request.host_timestamp_us = 10000u;
    request.sensor_timestamp_us = 10000u;
    request.sensor_accuracy = 3u;
    request.quaternion[0] = 1.0f;
    request.guidance_quaternion[0] = 1.0f;

    asr_fc_hil_flight_output_t response{};
    if (!transport.exchange(request, response, 100u, error)) {
        std::cerr << error << '\n';
        return 1;
    }

    std::cout << "step=" << static_cast<unsigned>(response.step_result)
              << " state=" << static_cast<unsigned>(response.flight_state)
              << " faults=0x" << std::hex << response.fault_flags << std::dec
              << " execution_us=" << response.execution_time_us << '\n';
    std::cout << "motor_q15=";
    for (std::size_t index = 0; index < 4; ++index) {
        if (index != 0) std::cout << ',';
        std::cout << response.motor_q15[index];
    }
    std::cout << " motor_rad_s=";
    for (std::size_t index = 0; index < 4; ++index) {
        if (index != 0) std::cout << ',';
        std::cout << response.motor_speed_rad_s[index];
    }
    std::cout << '\n';
    return response.step_result == 0u && response.fault_flags == 0u ? 0 : 1;
}
