#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

#include <poll.h>
#include <pty.h>
#include <unistd.h>

#include "hil/nrf_hil_transport.h"
#include "asr_fc/protocol/hil_link.h"

namespace {
enum class ReplyMode { Valid, WrongIdentity, Silent };

struct PseudoTerminal {
    int master{-1};
    int slave{-1};
    std::string path;

    PseudoTerminal()
    {
        char name[128]{};
        assert(openpty(&master, &slave, name, nullptr, nullptr) == 0);
        path = name;
    }

    ~PseudoTerminal()
    {
        if (master >= 0) ::close(master);
        if (slave >= 0) ::close(slave);
    }
};

asr_fc_hil_sensor_guidance_t makeRequest()
{
    asr_fc_hil_sensor_guidance_t request{};
    request.session_id = 0x41535231u;
    request.host_timestamp_us = 10000u;
    request.sensor_timestamp_us = 10000u;
    request.quaternion[0] = 1.0f;
    request.guidance_quaternion[0] = 1.0f;
    request.sensor_accuracy = 3u;
    request.collective_thrust_n = 9.81f;
    return request;
}

bool readRequest(int descriptor, std::uint32_t& sequence,
                 asr_fc_hil_sensor_guidance_t& request)
{
    asr_fc_hil_parser_t parser{};
    asr_fc_hil_parser_init(&parser);
    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(200);
    while (std::chrono::steady_clock::now() < deadline) {
        pollfd wait{descriptor, POLLIN, 0};
        if (poll(&wait, 1, 20) <= 0) continue;
        std::uint8_t input[32]{};
        const ssize_t count = ::read(descriptor, input, sizeof(input));
        if (count <= 0) continue;
        for (ssize_t index = 0; index < count; ++index) {
            std::uint8_t frame[ASR_FC_HIL_MAX_FRAME_SIZE]{};
            std::size_t frame_size = 0;
            bool ready = false;
            if (asr_fc_hil_parser_push(&parser, input[index], frame,
                    sizeof(frame), &frame_size, &ready) != ASR_FC_HIL_OK) {
                return false;
            }
            if (ready) {
                return asr_fc_hil_decode_sensor_guidance(
                    frame, frame_size, &sequence, &request) == ASR_FC_HIL_OK;
            }
        }
    }
    return false;
}

void emulateEndpoint(int descriptor, ReplyMode mode)
{
    std::uint32_t sequence = 0;
    asr_fc_hil_sensor_guidance_t request{};
    if (!readRequest(descriptor, sequence, request) || mode == ReplyMode::Silent) {
        return;
    }

    asr_fc_hil_flight_output_t response{};
    response.session_id = request.session_id;
    response.acknowledged_sequence = mode == ReplyMode::WrongIdentity
        ? sequence + 1u : sequence;
    response.device_timestamp_us = 12000u;
    response.execution_time_us = 73u;
    response.flight_state = request.arm_requested ? 1u : 0u;
    response.collective_thrust_n = request.collective_thrust_n;
    response.observed_quaternion[0] = 1.0f;
    for (std::size_t index = 0; index < 4; ++index) {
        response.motor_q15[index] = 12000u + static_cast<std::uint16_t>(index);
        response.motor_speed_rad_s[index] = 400.0f + static_cast<float>(index);
    }

    std::uint8_t encoded[ASR_FC_HIL_MAX_FRAME_SIZE]{};
    std::size_t encoded_size = 0;
    assert(asr_fc_hil_encode_flight_output(
        sequence, &response, encoded, sizeof(encoded), &encoded_size) ==
        ASR_FC_HIL_OK);
    const std::size_t split = encoded_size / 2u;
    assert(::write(descriptor, encoded, split) == static_cast<ssize_t>(split));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    assert(::write(descriptor, encoded + split, encoded_size - split) ==
        static_cast<ssize_t>(encoded_size - split));
}

void testExchange(ReplyMode mode, bool expected_success,
                  const std::string& expected_error)
{
    PseudoTerminal terminal;
    NrfHilTransport transport;
    std::string error;
    assert(transport.openDevice(terminal.path, error));
    ::close(terminal.slave);
    terminal.slave = -1;

    std::thread endpoint(emulateEndpoint, terminal.master, mode);
    asr_fc_hil_flight_output_t response{};
    const bool success = transport.exchange(makeRequest(), response, 50u, error);
    endpoint.join();
    assert(success == expected_success);
    if (expected_success) {
        assert(response.session_id == 0x41535231u);
        assert(response.acknowledged_sequence == 1u);
        assert(response.execution_time_us == 73u);
        assert(response.motor_q15[3] == 12003u);
    } else {
        assert(error.find(expected_error) != std::string::npos);
    }
}
}

int main()
{
    testExchange(ReplyMode::Valid, true, "");
    testExchange(ReplyMode::WrongIdentity, false, "identity mismatch");
    testExchange(ReplyMode::Silent, false, "timed out");

    NrfHilTransport unopened;
    asr_fc_hil_flight_output_t response{};
    std::string error;
    assert(!unopened.exchange(makeRequest(), response, 20u, error));
    assert(error.find("not open") != std::string::npos);

    std::cout << "AeroDyn nRF HIL serial transport gates passed\n";
    return 0;
}
