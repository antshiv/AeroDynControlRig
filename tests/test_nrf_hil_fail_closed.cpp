#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <poll.h>
#include <pty.h>
#include <unistd.h>

extern "C" {
#include "asr_fc/protocol/hil_link.h"
}
#include "core/simulation_state.h"
#include "modules/nrf_hil.h"

namespace {
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

void consumeOneRequestThenDisconnect(PseudoTerminal* terminal)
{
    const int descriptor = terminal->master;
    asr_fc_hil_parser_t parser{};
    asr_fc_hil_parser_init(&parser);
    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(100);
    while (std::chrono::steady_clock::now() < deadline) {
        pollfd wait{descriptor, POLLIN, 0};
        if (poll(&wait, 1, 10) <= 0) continue;
        std::uint8_t input[32]{};
        const ssize_t count = ::read(descriptor, input, sizeof(input));
        if (count <= 0) continue;
        for (ssize_t index = 0; index < count; ++index) {
            std::uint8_t frame[ASR_FC_HIL_MAX_FRAME_SIZE]{};
            std::size_t frame_size = 0;
            bool ready = false;
            (void)asr_fc_hil_parser_push(&parser, input[index], frame,
                sizeof(frame), &frame_size, &ready);
            if (ready) {
                ::close(descriptor);
                terminal->master = -1;
                return;
            }
        }
    }
}
}  // namespace

int main()
{
    PseudoTerminal terminal;
    setenv("ASR_FC_HIL_DEVICE", terminal.path.c_str(), 1);

    SimulationState state;
    state.control.paused = false;
    state.execution.selected_mode =
        SimulationState::ExecutionMode::Nrf5340Hil;
    state.pilot.enabled = true;
    state.mission.phase = SimulationState::FlightPhase::Flying;
    state.time_seconds = 0.01;
    state.rotor_config.maximum_speed_rad_s = 1000.0;
    state.motor_commands.omega_rad_s.fill(500.0);
    state.motor_commands.throttle_0_1.fill(0.5);

    NrfHilModule module;
    module.initialize(state);
    assert(state.execution.nrf_hil_available);
    ::close(terminal.slave);
    terminal.slave = -1;

    std::thread endpoint(consumeOneRequestThenDisconnect, &terminal);
    module.update(0.01, state);
    endpoint.join();

    assert(state.control.paused);
    assert(!state.pilot.enabled);
    assert(state.pilot.reset_controller);
    assert(state.execution.nrf_hil_failures == 1u);
    for (std::size_t index = 0; index < 4; ++index) {
        assert(state.motor_commands.omega_rad_s[index] == 0.0);
        assert(state.motor_commands.throttle_0_1[index] == 0.0);
    }
    std::cout << "AeroDyn nRF HIL disconnect fails closed\n";
    return 0;
}
