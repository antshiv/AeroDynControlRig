#include "gui/panel_manager.h"

#include "render/camera.h"
#include "core/simulation_state.h"

void PanelManager::registerPanel(std::unique_ptr<Panel> panel) {
    panels_.emplace_back(std::move(panel));
}

void PanelManager::drawAll(SimulationState& state, Camera& camera) {
    for (auto& panel : panels_) {
        panel->draw(state, camera);
    }
}
