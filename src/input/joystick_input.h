#ifndef AERODYN_JOYSTICK_INPUT_H
#define AERODYN_JOYSTICK_INPUT_H

#include <array>

struct SimulationState;

class JoystickInput {
public:
    void poll(SimulationState& state);

private:
    std::array<bool, 15> previous_buttons_{};
    bool was_connected_{false};
};

#endif
