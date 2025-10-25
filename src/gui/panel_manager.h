/**
 * @file panel_manager.h
 * @brief Registry and orchestrator for ImGui UI panels
 */

#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include <memory>
#include <vector>

#include "gui/panel.h"

class Camera;
struct SimulationState;

/**
 * @class PanelManager
 * @brief Manages registration and rendering of all UI panels
 *
 * The PanelManager acts as a central registry for all ImGui panels in the
 * application. It:
 * - Stores unique_ptr ownership of all panels
 * - Iterates through panels and calls draw() each frame
 * - Provides a single point of control for UI rendering
 *
 * **Usage**:
 * 1. Register panels during initialization: `registerPanel(std::make_unique<MyPanel>())`
 * 2. Call `drawAll()` each frame to render all registered panels
 *
 * **Design**:
 * - Panels are stored in registration order
 * - Each panel is responsible for creating its own ImGui window
 * - PanelManager does not manage window layout (handled by ImGui docking)
 */
class PanelManager {
public:
    /**
     * @brief Register a panel for rendering
     *
     * Takes ownership of the panel via unique_ptr.
     *
     * @param panel Unique pointer to a Panel-derived instance
     */
    void registerPanel(std::unique_ptr<Panel> panel);

    /**
     * @brief Render all registered panels
     *
     * Calls draw() on each panel in registration order.
     *
     * @param state Reference to simulation state (passed to panels)
     * @param camera Reference to camera (passed to panels)
     */
    void drawAll(SimulationState& state, Camera& camera);

private:
    std::vector<std::unique_ptr<Panel>> panels_; ///< Registered panels (owned)
};

#endif // PANEL_MANAGER_H
