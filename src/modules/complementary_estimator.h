#ifndef COMPLEMENTARY_ESTIMATOR_H
#define COMPLEMENTARY_ESTIMATOR_H

#include <array>
#include <glm/glm.hpp>

#include "core/module.h"

class ComplementaryEstimatorModule : public Module {
public:
    void initialize(SimulationState& state) override;
    void update(double dt, SimulationState& state) override;

    void setGains(float kp, float ki) {
        kp_ = kp;
        ki_ = ki;
    }

private:
    std::array<double, 4> q_est_{1.0, 0.0, 0.0, 0.0};
    glm::vec3 bias_{0.0f};
    float kp_{2.0f};
    float ki_{0.05f};
};

#endif // COMPLEMENTARY_ESTIMATOR_H
