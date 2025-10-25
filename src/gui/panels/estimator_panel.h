/**
 * @file estimator_panel.h
 * @brief State estimator diagnostics and visualization
 */

#ifndef ESTIMATOR_PANEL_H
#define ESTIMATOR_PANEL_H

#include "gui/panel.h"

/**
 * @class EstimatorPanel
 * @brief UI panel displaying state estimator outputs and diagnostics
 *
 * Shows:
 * - Estimated attitude (quaternion and Euler angles)
 * - Comparison with ground truth (if available)
 * - Estimator-specific parameters (e.g., filter gains, bias estimates)
 * - Convergence metrics
 *
 * Useful for debugging sensor fusion algorithms and tuning estimator parameters.
 */
class EstimatorPanel : public Panel {
public:
    const char* name() const override { return "Estimator"; }
    void draw(SimulationState& state, Camera& camera) override;
};

#endif // ESTIMATOR_PANEL_H
