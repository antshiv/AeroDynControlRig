#ifndef PANEL_H
#define PANEL_H

#include <string>

class Camera;
struct SimulationState;

class Panel {
public:
    virtual ~Panel() = default;
    virtual const char* name() const = 0;
    virtual void draw(SimulationState& state, Camera& camera) = 0;
};

#endif // PANEL_H
