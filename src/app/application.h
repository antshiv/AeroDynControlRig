/**
 * @file application.h
 * @brief Main application class coordinating rendering, simulation, and UI
 */

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

/**
 * @class Application
 * @brief Main application orchestrating the simulation, rendering, and UI
 *
 * The Application class manages the complete lifecycle of the AeroDynControlRig:
 * - Initializes GLFW window, OpenGL context, and ImGui
 * - Manages simulation modules (dynamics, sensors, estimators)
 * - Orchestrates 3D rendering (scene + axis overlays)
 * - Renders ImGui dashboard with multiple panels
 * - Handles user input via GLFW callbacks
 *
 * Architecture:
 * - Modules update simulation state each tick
 * - Panels visualize and control simulation state
 * - Renderer displays 3D scene to off-screen framebuffer
 * - ImGui displays framebuffer texture alongside control panels
 */
class Application {
public:
    Application();
    ~Application();

    // === Public Members (Legacy) ===
    Transform transform;        ///< Transformation matrix utilities
    Camera camera;              ///< 3D scene camera
    AxisRenderer axisRenderer;  ///< Coordinate axis overlay renderer
    int windowHeight;           ///< Current window height (pixels)
    int windowWidth;            ///< Current window width (pixels)
    float lastFrame = 0.0f;     ///< Timestamp of last frame (seconds)

    /**
     * @brief Initialize the application subsystems
     *
     * Initialization sequence:
     * 1. GLFW library and window creation
     * 2. OpenGL context and GLEW extension loading
     * 3. Dear ImGui setup with docking/viewports
     * 4. Renderers (main scene + axis overlay)
     * 5. Simulation modules (dynamics, sensors, estimators, etc.)
     * 6. UI panels
     *
     * @return true if initialization succeeded, false otherwise
     */
    bool init();

    /**
     * @brief Check if the application should continue running
     * @return true if window is still open, false if close requested
     */
    bool running() const;

    /**
     * @brief Execute one frame: update simulation, render 3D, render UI
     *
     * Called each iteration of the main loop. Updates modules, renders
     * 3D scene to framebuffer, and draws ImGui interface.
     */
    void tick();

    /**
     * @brief Render the 3D scene (cube + axes)
     *
     * Called by tick() to render geometry to the off-screen framebuffer.
     * The resulting texture is displayed in the ImGui viewport.
     */
    void render3D();

    /**
     * @brief Clean up resources before exit
     *
     * Destroys render targets, shuts down ImGui, and terminates GLFW.
     */
    void shutdown();

    // === GLFW Callbacks ===
    /**
     * @brief Keyboard input callback
     * @param window GLFW window handle
     * @param key Key code
     * @param scancode Platform-specific scancode
     * @param action GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT
     * @param mods Modifier key flags
     */
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * @brief Mouse movement callback
     * @param window GLFW window handle
     * @param xpos Mouse X position
     * @param ypos Mouse Y position
     */
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

    /**
     * @brief Mouse scroll callback
     * @param window GLFW window handle
     * @param xoffset Horizontal scroll offset
     * @param yoffset Vertical scroll offset
     */
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    // === Core Subsystems ===
    GLFWwindow* window;                              ///< GLFW window handle
    Renderer renderer;                               ///< Main 3D scene renderer

    // === Off-Screen Rendering ===
    GLuint fbo;                                      ///< Framebuffer object for off-screen rendering
    GLuint renderTexture;                            ///< Color attachment texture
    GLuint depthBuffer;                              ///< Depth/stencil renderbuffer
    int sceneWidth = 0;                              ///< Current render target width
    int sceneHeight = 0;                             ///< Current render target height

    // === Simulation State and Modules ===
    SimulationState simulationState;                 ///< Shared simulation state
    std::vector<std::unique_ptr<Module>> modules;    ///< Registered simulation modules
    PanelManager panelManager;                       ///< UI panel manager

    // === Initialization Helpers ===
    /**
     * @brief Initialize all simulation modules
     *
     * Creates instances of:
     * - QuaternionDemoModule (attitude dynamics)
     * - FirstOrderDynamicsModule (test system)
     * - SensorSimulatorModule (IMU simulation)
     * - ComplementaryEstimatorModule (sensor fusion)
     * - RotorTelemetryModule (rotor calculations)
     */
    void initializeModules();

    /**
     * @brief Initialize all UI panels
     *
     * Registers panels for:
     * - Simulation control (pause, timestep, etc.)
     * - Telemetry display
     * - Dynamics configuration
     * - Estimator diagnostics
     * - Rotor visualization
     * - Power monitoring
     * - Sensor readouts
     */
    void initializePanels();

    // === Rendering Helpers ===
    /**
     * @brief Update camera position based on input
     * @param deltaTime Time since last frame (seconds)
     */
    void updateCamera(float deltaTime);

    /**
     * @brief Render 3D scene to off-screen texture
     *
     * Creates/resizes render target as needed and renders the scene.
     *
     * @param size Desired render target dimensions
     * @return ImGui texture ID for displaying the rendered scene
     */
    ImTextureID renderSceneToTexture(const ImVec2& size);

    /**
     * @brief Ensure render target exists with specified dimensions
     *
     * Creates or resizes the framebuffer if dimensions have changed.
     *
     * @param width Target width (pixels)
     * @param height Target height (pixels)
     * @return true if render target is ready, false on error
     */
    bool ensureRenderTarget(int width, int height);

    /**
     * @brief Destroy the off-screen render target
     *
     * Releases framebuffer, texture, and depth buffer resources.
     */
    void destroyRenderTarget();
    void captureAttitudeHistorySample();

    // === UI Layout Modes ===
    /**
     * @brief Render the new dashboard layout (7-panel design)
     * @param io ImGui IO context
     */
    void renderDashboardLayout(ImGuiIO& io);

    /**
     * @brief Render the legacy single-window layout
     */
    void renderLegacyLayout();
};

#endif // APPLICATION_H
