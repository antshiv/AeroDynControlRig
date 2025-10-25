#ifndef APPLICATION_H
#define APPLICATION_H

#include <GL/glew.h>  // MUST come first
#include <GLFW/glfw3.h>
#include "attitude/euler.h" // from your attitude library
#include <memory>
#include <vector>

#include "render/renderer.h"
#include "core/transform.h"
#include "render/axis_renderer.h"
#include "render/camera.h"
#include "core/simulation_state.h"
#include "core/module.h"
#include "gui/panel_manager.h"
#include "imgui.h"

class Application {
public:
    Application();
    ~Application();

    Transform transform; // Transformation manager
    Camera camera;

    AxisRenderer axisRenderer;
    int windowHeight, windowWidth;
    float lastFrame = 0.0f;  // Initialize to 0	
    
    // Initialize the application: create window, init renderer, etc.
    bool init();

    // Check if the application should keep running
    bool running() const;

    void tick();
    void render3D();

    // Cleanup before exit
    void shutdown();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    GLFWwindow* window;
    Renderer renderer;

    GLuint fbo;           // Framebuffer object
    GLuint renderTexture; // Color attachment
    GLuint depthBuffer;   // Depth attachment

    SimulationState simulationState;
    std::vector<std::unique_ptr<Module>> modules;
    PanelManager panelManager;

    void updateCamera(float deltaTime);
    void initializeModules();
    void initializePanels();

    ImTextureID renderSceneToTexture(const ImVec2& size);
    bool ensureRenderTarget(int width, int height);
    void destroyRenderTarget();
    void renderDashboardLayout(ImGuiIO& io);
    void renderLegacyLayout();
    int sceneWidth = 0;
    int sceneHeight = 0;
};

#endif // APPLICATION_H
