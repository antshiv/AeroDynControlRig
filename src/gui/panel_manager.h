#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include <memory>
#include <vector>

#include "gui/panel.h"

class Camera;
struct SimulationState;

class PanelManager {
public:
    void registerPanel(std::unique_ptr<Panel> panel);
    void drawAll(SimulationState& state, Camera& camera);

private:
    std::vector<std::unique_ptr<Panel>> panels_;
};

#endif // PANEL_MANAGER_H
