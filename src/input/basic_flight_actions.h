#ifndef AERODYN_BASIC_FLIGHT_ACTIONS_H
#define AERODYN_BASIC_FLIGHT_ACTIONS_H

struct SimulationState;

enum class BasicFlightAction {
    Takeoff,
    Land,
    TogglePause,
    FitView,
    Reset,
    EmergencyStop,
};

void applyBasicFlightAction(BasicFlightAction action, bool input_ready,
                            SimulationState& state);

#endif
