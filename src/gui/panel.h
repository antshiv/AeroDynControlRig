/**
 * @file panel.h
 * @brief Base interface for ImGui UI panels in the AeroDynControlRig
 */

#ifndef PANEL_H
#define PANEL_H

#include <string>

class Camera;
struct SimulationState;

/**
 * @class Panel
 * @brief Abstract base class for all UI panels
 *
 * Panels provide modular UI components rendered using Dear ImGui. Each panel
 * has read/write access to the simulation state and camera, allowing interactive
 * control and visualization of simulation parameters.
 *
 * Panels are managed by PanelManager and rendered in the main UI layout.
 *
 * @see PanelManager
 * @see SimulationState
 */
class Panel {
public:
    virtual ~Panel() = default;

    /**
     * @brief Get the display name of this panel
     *
     * Used by PanelManager to label windows and menu items.
     *
     * @return C-string containing the panel's display name
     */
    virtual const char* name() const = 0;

    /**
     * @brief Render the panel's ImGui interface
     *
     * Called each frame to draw the panel's UI elements. Use ImGui commands
     * to create controls, visualizations, and interactive elements.
     *
     * @param state Reference to the shared simulation state (read/write)
     * @param camera Reference to the rendering camera (read/write)
     */
    virtual void draw(SimulationState& state, Camera& camera) = 0;
};

#endif // PANEL_H
