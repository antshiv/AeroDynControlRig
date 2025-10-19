#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <array>
#include <glm/glm.hpp>
#include "attitude/euler.h"

struct SimulationState {
    EulerAngles euler{0.0, 0.0, 0.0, EULER_ZYX};
    std::array<double, 4> quaternion{1.0, 0.0, 0.0, 0.0};
    glm::mat4 model_matrix{1.0f};
    double rotation_speed_deg_per_sec{30.0};
};

#endif // SIMULATION_STATE_H
