#include "application.h"
#include "render/renderer.h"
#include "modules/quaternion_demo.h"
#include "modules/first_order_dynamics.h"
#include "modules/sensor_simulator.h"
#include "modules/complementary_estimator.h"
#include "modules/rotor_telemetry.h"
#include "gui/panel_manager.h"
#include "gui/widgets/card.h"
#include "gui/style.h"
#include "gui/panels/control_panel.h"
#include "gui/panels/telemetry_panel.h"
#include "gui/panels/dynamics_panel.h"
#include "gui/panels/estimator_panel.h"
#include "gui/panels/rotor_panel.h"
#include "gui/panels/power_panel.h"
#include "gui/panels/sensor_panel.h"
#include "attitude/euler.h"
#include "attitude/dcm.h"
#include "attitude/quaternion.h"
#include "attitude/attitude_utils.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdint>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

namespace {
constexpr float kTopNavHeight = 64.0f;
constexpr float kDockspaceMargin = 24.0f;

void DrawTopNavigation() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 nav_pos = viewport->Pos;
    const ImVec2 nav_size = ImVec2(viewport->Size.x, kTopNavHeight);

    ImGui::SetNextWindowPos(nav_pos);
    ImGui::SetNextWindowSize(nav_size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 14.0f));
    const ui::Palette& palette = ui::Colors();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(palette.canvas_bg.x, palette.canvas_bg.y, palette.canvas_bg.z, 0.98f));

    ImGuiWindowFlags nav_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoDocking;

    if (ImGui::Begin("TopNavigation", nullptr, nav_flags)) {
        const ui::FontSet& fonts = ui::Fonts();
        bool pushed_heading = false;
        if (fonts.heading) {
            ImGui::PushFont(fonts.heading);
            pushed_heading = true;
        }

        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, palette.accent_base);
        ImGui::TextUnformatted("DF");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, palette.text_primary);
        ImGui::TextUnformatted("Drone Flight Dynamics");
        ImGui::PopStyleColor();
        ImGui::EndGroup();

        float nav_right_width = 420.0f;
        float available = ImGui::GetContentRegionAvail().x;
        if (available > nav_right_width) {
            ImGui::Dummy(ImVec2(available - nav_right_width, 0.0f));
            ImGui::SameLine();
        }

        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
        ImGui::TextUnformatted("Dashboard");
        ImGui::SameLine(0.0f, 24.0f);
        ImGui::TextUnformatted("Settings");
        ImGui::SameLine(0.0f, 24.0f);
        ImGui::TextUnformatted("Logs");
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, 28.0f);
        ui::PushPillButtonStyle(ui::PillStyle::Primary);
        ImGui::Button("Connected");
        ui::PopPillButtonStyle();

        ImGui::SameLine(0.0f, 12.0f);
        ui::PushPillButtonStyle(ui::PillStyle::Secondary);
        ImGui::Button("Link");
        ui::PopPillButtonStyle();
        ImGui::EndGroup();

        if (pushed_heading) {
            ImGui::PopFont();
        }
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}
}


Application::Application() {
    // Constructor code
    window = nullptr;
    fbo = 0;
    renderTexture = 0;
    depthBuffer = 0;
    sceneWidth = 0;
    sceneHeight = 0;
}

Application::~Application() {
    // Destructor code
}

/**
 * @brief Initializes the application, setting up GLFW, OpenGL, ImGui, and all core components.
 *
 * @details This function performs a sequence of critical initialization steps required
 * for the application to run. It handles window creation, OpenGL context setup,
 * graphics library extensions (GLEW), the Dear ImGui user interface, and custom
 * rendering components.
 *
 * The order of these initializations is important due to dependencies between libraries.
 *
 * @return `true` if all initializations are successful, `false` otherwise.
 */

/**
 * @brief Initialize the complete application stack
 *
 * This function executes the critical initialization sequence for all subsystems.
 * The order is **carefully arranged** to satisfy dependencies between components.
 *
 * @note Initialization Order Diagram
 *
 * @code
 * ┌─────────────────────────────────────────────────────────────┐
 * │ Application::init() - Initialization Sequence               │
 * └─────────────────────────────────────────────────────────────┘
 *
 * Step 1: glfwInit()
 *    │
 *    ├─► Initializes GLFW library
 *    └─► Prepares windowing system
 *
 * Step 2: glfwCreateWindow()
 *    │
 *    ├─► Creates OS window (800x600)
 *    ├─► Creates OpenGL 3.3 context
 *    └─► Returns window handle
 *
 * Step 3: glfwMakeContextCurrent()
 *    │
 *    ├─► Binds OpenGL context to this thread
 *    └─► All subsequent GL calls use this context
 *
 * Step 4: glewInit()
 *    │
 *    ├─► Loads OpenGL function pointers
 *    ├─► Enables access to GL 3.3+ functions
 *    └─► MUST happen after context is current
 *
 * Step 5: ImGui::CreateContext()
 *    │
 *    ├─► Creates ImGui context
 *    ├─► Enables ImGui docking
 *    └─► MUST happen after GL context exists
 *
 * Step 6: ImGui_ImplGlfw_InitForOpenGL()
 *         ImGui_ImplOpenGL3_Init()
 *    │
 *    ├─► Links ImGui to GLFW (input handling)
 *    ├─► Links ImGui to OpenGL (rendering)
 *    └─► Compiles ImGui's internal shaders
 *
 * Step 7: renderer.init()
 *         axisRenderer.init()
 *    │
 *    ├─► Compiles GLSL shaders (vertex + fragment)
 *    ├─► Creates VAO/VBO/EBO for geometry
 *    ├─► Uploads cube and axis vertices to GPU
 *    └─► MUST happen after glewInit()
 *
 * Step 8: initializeModules()
 *    │
 *    ├─► Creates QuaternionDemoModule
 *    ├─► Creates SensorSimulatorModule
 *    ├─► Creates ComplementaryEstimatorModule
 *    ├─► Creates FirstOrderDynamicsModule
 *    ├─► Creates RotorTelemetryModule
 *    └─► Calls initialize() on each module
 *
 * Step 9: initializePanels()
 *    │
 *    ├─► Registers ControlPanel
 *    ├─► Registers TelemetryPanel
 *    ├─► Registers DynamicsPanel
 *    ├─► Registers EstimatorPanel
 *    ├─► Registers RotorPanel
 *    ├─► Registers SensorPanel
 *    └─► Registers PowerPanel
 *
 * Result: Application ready for main loop
 * @endcode
 *
 * @section init_dependencies Dependency Chain
 *
 * The initialization order **cannot be changed** without breaking the application:
 *
 * - GLFW must initialize before creating windows
 * - Window must exist before creating OpenGL context
 * - OpenGL context must be current before GLEW init
 * - GLEW must load GL functions before calling any GL commands
 * - GL context must exist before ImGui init
 * - ImGui must initialize before compiling its backends
 * - Shaders can only compile after GLEW loads GL functions
 * - Modules/panels can initialize anytime after their dependencies
 *
 * @return true if all initialization steps succeed
 * @return false if any step fails (with error logged to stderr)
 *
 * @see Application::tick() for the main loop that uses these initialized resources
 * @see Application::shutdown() for the reverse teardown sequence
 */
bool Application::init() {
    // Step 1: Initialize GLFW (Graphics Library Framework)
    // GLFW is used to create windows, contexts, and manage user input.
    // It must be initialized before any window or OpenGL context can be created.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Step 2: Create the GLFW window and OpenGL context
    // This creates a window with an associated OpenGL context, which is where
    // all rendering commands will be executed. The window dimensions are 800x600.
    window = glfwCreateWindow(800, 600, "Dynamic Control System Test Rig", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); // Terminate GLFW if window creation fails
        return false;
    }

    // Make the newly created window's OpenGL context current on the calling thread.
    // All subsequent OpenGL commands will be directed to this context.
    glfwMakeContextCurrent(window);
    std::cout << "Window created successfully: " << window << std::endl;

    // Step 3: Set up GLFW callbacks for user input
    // Callbacks link GLFW events (like key presses or scrolling) to application functions.
    // glfwSetWindowUserPointer allows us to pass a pointer to the Application instance
    // into static callback functions.
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    // Mouse callback is commented out, but would be set here if active.
    glfwSetScrollCallback(window, scrollCallback);

    // Step 4: Initialize GLEW (OpenGL Extension Wrangler Library)
    // GLEW manages OpenGL extensions and provides a convenient way to access
    // modern OpenGL functions. It must be initialized after a valid OpenGL
    // rendering context is created and made current.
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    // Check and log OpenGL version and renderer information for debugging.
    const GLubyte* rendererName = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    if (rendererName && version) {
        std::cout << "Renderer: " << rendererName << "\nOpenGL version: " << version << std::endl;
    } else {
        std::cerr << "Failed to retrieve OpenGL context information." << std::endl;
        return false;
    }

    // Step 5: Initialize ImGui (Dear ImGui) context
    // ImGui is a bloat-free graphical user interface library. Its context
    // must be created after the OpenGL context is ready.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Enable docking feature for flexible window layouts.
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Step 6: Configure ImGui style and load fonts
    // Sets the visual theme (dark mode) and loads custom fonts for the UI.
    ImGui::StyleColorsDark();
    ui::ApplyTheme(ImGui::GetStyle());
    ui::LoadFonts(io);

    // Step 7: Initialize ImGui backends for GLFW and OpenGL
    // These backends bridge ImGui with the underlying windowing system (GLFW)
    // and rendering API (OpenGL), allowing ImGui to draw its UI elements.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); // Specifies the GLSL version for shaders

    // Step 8: Initialize custom renderers
    // The application uses a primary 3D renderer and a separate axis renderer.
    // Their initialization involves compiling shaders and setting up VAOs/VBOs.
    if (!renderer.init()) {
        std::cerr << "Renderer initialization failed." << std::endl;
        return false;
    }

    if (!axisRenderer.init()) {
        std::cerr << "Axis renderer initialization failed." << std::endl;
        return false;
    }

    // Step 9: Set up initial orthographic projection for 2D rendering (if applicable)
    // This defines the viewing volume for 2D elements.
    float aspectRatio = 800.0f / 600.0f; // Adjust dynamically if needed
    transform.setOrthographic(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);

    // Step 10: Initialize application modules and GUI panels
    // These are custom components that encapsulate specific application logic
    // (e.g., physics simulation, sensor data) and their corresponding UI panels.
    initializeModules();
    initializePanels();
    lastFrame = glfwGetTime(); // Record the time for delta time calculations

    return true;
}

bool Application::running() const {
    //std::cout << "Checking window state. Window pointer: " << window << std::endl;
    if (window) {
        bool shouldClose = glfwWindowShouldClose(window);
        //std::cout << "glfwWindowShouldClose result: " << shouldClose << std::endl;
        return !shouldClose;
    } else {
        std::cerr << "Window is NULL in running()" << std::endl;
        return false;
    }
}

void Application::initializeModules() {
    modules.emplace_back(std::make_unique<QuaternionDemoModule>());
    modules.emplace_back(std::make_unique<FirstOrderDynamicsModule>());
    modules.emplace_back(std::make_unique<SensorSimulatorModule>());
    modules.emplace_back(std::make_unique<ComplementaryEstimatorModule>());
    modules.emplace_back(std::make_unique<RotorTelemetryModule>());
    for (auto& module : modules) {
        module->initialize(simulationState);
    }
    transform.model = simulationState.model_matrix;
}



void Application::tick() {
    float currentFrame = glfwGetTime();
    double real_dt = static_cast<double>(currentFrame - lastFrame);
    lastFrame = currentFrame;

    updateCamera(static_cast<float>(real_dt));

    // Handle body rotation speed adjustments via keyboard.
    glm::dvec3& body_rates = simulationState.angular_rate_deg_per_sec;
    auto adjust_rotation = [&](int key, int axis, double direction) {
        if (glfwGetKey(window, key) == GLFW_PRESS) {
            const double kRotationAccelDegPerSec2 = 180.0;
            body_rates[axis] += direction * kRotationAccelDegPerSec2 * real_dt;
        }
    };

    // Roll (X) via Q/E
    adjust_rotation(GLFW_KEY_Q, 0, 1.0);
    adjust_rotation(GLFW_KEY_E, 0, -1.0);

    // Pitch (Y) via Up/Down arrows or I/K
    adjust_rotation(GLFW_KEY_UP, 1, 1.0);
    adjust_rotation(GLFW_KEY_DOWN, 1, -1.0);
    adjust_rotation(GLFW_KEY_I, 1, 1.0);
    adjust_rotation(GLFW_KEY_K, 1, -1.0);

    // Yaw (Z) via Left/Right arrows or J/L
    adjust_rotation(GLFW_KEY_LEFT, 2, 1.0);
    adjust_rotation(GLFW_KEY_RIGHT, 2, -1.0);
    adjust_rotation(GLFW_KEY_J, 2, 1.0);
    adjust_rotation(GLFW_KEY_L, 2, -1.0);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        body_rates = glm::dvec3(0.0f);
    }

    if (!simulationState.control.paused) {
        double dt = simulationState.control.use_fixed_dt
                        ? simulationState.control.fixed_dt
                        : real_dt * simulationState.control.time_scale;

        simulationState.last_dt = dt;

        if (dt > 0.0) {
            for (auto& module : modules) {
                module->update(dt, simulationState);
            }
        }
    } else {
        simulationState.last_dt = 0.0;
    }

    transform.model = simulationState.model_matrix;
    render3D();
}

void Application::shutdown() {
    destroyRenderTarget();

    // Cleanup Dear ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup resources
    //renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) {
        std::cerr << "Application instance is null in keyCallback" << std::endl;
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    // === KEYBOARD-CONTROLLED QUATERNION ROTATION ===
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        const double rotation_deg = 5.0;  // Degrees per key press

        // Convert current quaternion to Euler angles
        double roll, pitch, yaw;
        double q[4] = {
            app->simulationState.quaternion[0],  // w
            app->simulationState.quaternion[1],  // x
            app->simulationState.quaternion[2],  // y
            app->simulationState.quaternion[3]   // z
        };
        quaternion_to_euler(q, &roll, &pitch, &yaw);

        // Convert to degrees for easier manipulation
        roll = rad2deg(roll);
        pitch = rad2deg(pitch);
        yaw = rad2deg(yaw);

        // Apply rotation based on key
        if (key == GLFW_KEY_W) {
            // Pitch up
            pitch += rotation_deg;
        }
        else if (key == GLFW_KEY_S) {
            // Pitch down
            pitch -= rotation_deg;
        }
        else if (key == GLFW_KEY_A) {
            // Roll left
            roll += rotation_deg;
        }
        else if (key == GLFW_KEY_D) {
            // Roll right
            roll -= rotation_deg;
        }
        else if (key == GLFW_KEY_Q) {
            // Yaw left
            yaw += rotation_deg;
        }
        else if (key == GLFW_KEY_E) {
            // Yaw right
            yaw -= rotation_deg;
        }
        else if (key == GLFW_KEY_R) {
            // Reset to identity quaternion (no rotation)
            app->simulationState.quaternion = {1.0, 0.0, 0.0, 0.0};
            app->simulationState.angular_rate_deg_per_sec = glm::dvec3(0.0, 0.0, 0.0);
            return;
        }
        else {
            return;  // No rotation key pressed
        }

        // Convert back to radians and then to quaternion
        EulerAngles euler_angles;
        euler_angles.roll = deg2rad(roll);
        euler_angles.pitch = deg2rad(pitch);
        euler_angles.yaw = deg2rad(yaw);
        euler_angles.order = EULER_ZYX;  // Yaw-Pitch-Roll order

        double q_new[4];
        euler_to_quaternion(&euler_angles, q_new);

        // Update state
        app->simulationState.quaternion = {q_new[0], q_new[1], q_new[2], q_new[3]};
    }
}

void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    static double lastX = xpos;
    static double lastY = ypos;
    float xOffset = static_cast<float>(xpos - lastX);
    float yOffset = static_cast<float>(lastY - ypos); // Reversed since y goes bottom→top
    lastX = xpos;
    lastY = ypos;

    app->camera.processMouseMovement(xOffset, yOffset);
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    app->camera.processMouseScroll(static_cast<float>(yoffset));
}

void Application::render3D() {
    transform.model = simulationState.model_matrix;

    // Step 1: Clear the framebuffer
    glClearColor(0.06f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Step 2: Render OpenGL 3D content
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // Set view and projection matrices
    transform.view = camera.getViewMatrix();
    transform.projection = camera.getProjectionMatrix(static_cast<float>(width) / height);

    // Step 4: Render ImGui UI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    if (simulationState.control.use_legacy_ui) {
        renderLegacyLayout();
    } else {
        renderDashboardLayout(io);
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Step 5: Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::renderDashboardLayout(ImGuiIO& io) {
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 dock_pos = ImVec2(viewport->Pos.x + kDockspaceMargin,
                                       viewport->Pos.y + kTopNavHeight + kDockspaceMargin);
        const ImVec2 dock_size = ImVec2(
            std::max(50.0f, viewport->Size.x - 2.0f * kDockspaceMargin),
            std::max(50.0f, viewport->Size.y - kTopNavHeight - 2.0f * kDockspaceMargin));

        ImGui::SetNextWindowPos(dock_pos);
        ImGui::SetNextWindowSize(dock_size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                     ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
        ImGui::Begin("DockSpaceHost", nullptr, host_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        static bool dockspace_initialized = false;
        if (!dockspace_initialized) {
            dockspace_initialized = true;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::DockBuilderSetNodeSize(dockspace_id, dock_size);

            ImGuiID dock_root = dockspace_id;
            ImGuiID dock_right;
            ImGuiID dock_left;
            ImGuiID dock_right_bottom;
            ImGuiID dock_left_bottom;
            ImGuiID dock_bottom_left;
            ImGuiID dock_bottom_center;
            ImGuiID dock_bottom_right;

            ImGui::DockBuilderSplitNode(dock_root, ImGuiDir_Right, 0.32f, &dock_right, &dock_left);
            ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Down, 0.42f, &dock_left_bottom, &dock_left);
            ImGui::DockBuilderSplitNode(dock_left_bottom, ImGuiDir_Right, 0.33f, &dock_bottom_right, &dock_left_bottom);
            ImGui::DockBuilderSplitNode(dock_left_bottom, ImGuiDir_Right, 0.50f, &dock_bottom_center, &dock_bottom_left);
            ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.52f, &dock_right_bottom, &dock_right);

            ImGui::DockBuilderDockWindow("Flight Scene", dock_left);
            ImGui::DockBuilderDockWindow("Rotor Dynamics", dock_right);
            ImGui::DockBuilderDockWindow("Power Monitor", dock_right_bottom);
            ImGui::DockBuilderDockWindow("Estimator", dock_bottom_left);
            ImGui::DockBuilderDockWindow("Control Panel", dock_bottom_center);
            ImGui::DockBuilderDockWindow("Sensor Suite", dock_bottom_right);
            ImGui::DockBuilderDockWindow("Flight Telemetry", dock_bottom_center);
            ImGui::DockBuilderDockWindow("Dynamics", dock_right_bottom);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    DrawTopNavigation();

    ui::CardOptions scene_card;
    scene_card.min_size = ImVec2(640.0f, 420.0f);
    scene_card.allow_scrollbar = false;
    scene_card.background_alpha = 0.0f;
    if (ui::BeginCard("Flight Scene", scene_card, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
        ui::CardHeader("3D Drone Visualization");

        const ui::Palette& palette = ui::Colors();
        const ui::FontSet& fonts = ui::Fonts();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.x = std::max(avail.x, 1.0f);
        avail.y = std::max(avail.y, 1.0f);

        const float footer_height = 68.0f;
        ImVec2 viewport_size = ImVec2(avail.x, std::max(120.0f, avail.y - footer_height));
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 footer_pos = ImVec2(canvas_pos.x, canvas_pos.y + viewport_size.y);
        ImVec2 footer_size = ImVec2(avail.x, footer_height);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRect(canvas_pos,
                           canvas_pos + viewport_size,
                           ImGui::ColorConvertFloat4ToU32(palette.card_border),
                           18.0f,
                           ImDrawFlags_RoundCornersAll,
                           2.0f);

        bool viewport_hovered = false;
        bool viewport_active = false;
        ImTextureID scene_texture = renderSceneToTexture(viewport_size);
        if (scene_texture) {
            ImGui::Image(scene_texture,
                         viewport_size,
                         ImVec2(0.0f, 1.0f),
                         ImVec2(1.0f, 0.0f));
            viewport_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
            viewport_active = ImGui::IsItemActive();

            char quat_buf[96];
            std::snprintf(
                quat_buf,
                sizeof(quat_buf),
                "Quaternion: %.3f, %.3f, %.3f, %.3f",
                simulationState.quaternion[0],
                simulationState.quaternion[1],
                simulationState.quaternion[2],
                simulationState.quaternion[3]);

            char euler_buf[96];
            std::snprintf(
                euler_buf,
                sizeof(euler_buf),
                "Euler (deg): R %.1f  P %.1f  Y %.1f",
                rad2deg(simulationState.euler.roll),
                rad2deg(simulationState.euler.pitch),
                rad2deg(simulationState.euler.yaw));

            ImVec2 text_pos = canvas_pos + ImVec2(18.0f, 18.0f);
            draw_list->AddText(text_pos,
                               ImGui::ColorConvertFloat4ToU32(palette.text_primary),
                               quat_buf);
            draw_list->AddText(text_pos + ImVec2(0.0f, 20.0f),
                               ImGui::ColorConvertFloat4ToU32(palette.text_muted),
                               euler_buf);

            char rates_buf[96];
            std::snprintf(
                rates_buf,
                sizeof(rates_buf),
                "Body Rate (deg/s): R %.1f  P %.1f  Y %.1f",
                simulationState.angular_rate_deg_per_sec.x,
                simulationState.angular_rate_deg_per_sec.y,
                simulationState.angular_rate_deg_per_sec.z);

            draw_list->AddText(text_pos + ImVec2(0.0f, 40.0f),
                               ImGui::ColorConvertFloat4ToU32(palette.text_muted),
                               rates_buf);
        } else {
            ImGui::Dummy(viewport_size);
            ImGui::SetCursorScreenPos(canvas_pos + ImVec2(18.0f, 18.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
            ImGui::TextUnformatted("Scene renderer unavailable");
            ImGui::PopStyleColor();
            ImGui::SetCursorScreenPos(canvas_pos + ImVec2(0.0f, viewport_size.y));
        }

        if (viewport_active || (viewport_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))) {
            const float orbit_sensitivity = 0.25f;
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                camera.processMouseMovement(drag.x * orbit_sensitivity, -drag.y * orbit_sensitivity);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }

        if (viewport_active || viewport_hovered) {
            const float pan_sensitivity = 0.004f;
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
                camera.pan(-drag.x * pan_sensitivity, drag.y * pan_sensitivity);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
            } else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                camera.pan(-drag.x * pan_sensitivity, drag.y * pan_sensitivity);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        // Footer control strip
        ImDrawFlags footer_rounding = ImDrawFlags_RoundCornersBottom;
        draw_list->AddRectFilled(footer_pos,
                                 footer_pos + footer_size,
                                 ImGui::ColorConvertFloat4ToU32(ImVec4(palette.card_header.x,
                                                                       palette.card_header.y,
                                                                       palette.card_header.z,
                                                                       0.96f)),
                                 18.0f,
                                 footer_rounding);

        ImGui::SetCursorScreenPos(footer_pos + ImVec2(24.0f, 18.0f));

        const float orbit_step_deg = 15.0f;
        const float tilt_step_deg = 10.0f;
        const float pan_step_units = 0.35f;
        const float dolly_step_units = 0.6f;
        const float zoom_step_deg = 5.0f;

        auto scene_control_button = [&](const char* id,
                                        const char* label,
                                        const char* icon_code,
                                        auto popup_builder) {
            std::string text = label;
            if (fonts.icon && icon_code) {
                text = std::string(icon_code) + " " + label;
            }
            ImGui::PushID(id);
            ui::PushPillButtonStyle(ui::PillStyle::Secondary);
            if (ImGui::Button(text.c_str(), ImVec2(120.0f, 0.0f))) {
                ImGui::OpenPopup("popup");
            }
            bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
            popup_builder("popup", hovered);
            ui::PopPillButtonStyle();
            ImGui::PopID();
        };

        scene_control_button(
            "rotate",
            "Rotate",
            u8"\ue5d1",
            [&](const char* popup_id, bool hovered) {
                if (hovered) {
                    ImGui::SetTooltip("Orbit the camera. Keys: Q/E, arrow keys, IJKL");
                }
                if (ImGui::BeginPopup(popup_id)) {
                    ImGui::TextUnformatted("Orbit camera");
                    ImGui::Separator();
                    if (ImGui::MenuItem("Orbit left", "Q / ← / J")) {
                        camera.orbit(-orbit_step_deg, 0.0f);
                    }
                    if (ImGui::MenuItem("Orbit right", "E / → / L")) {
                        camera.orbit(orbit_step_deg, 0.0f);
                    }
                    if (ImGui::MenuItem("Tilt up", "I / ↑")) {
                        camera.orbit(0.0f, tilt_step_deg);
                    }
                    if (ImGui::MenuItem("Tilt down", "K / ↓")) {
                        camera.orbit(0.0f, -tilt_step_deg);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Reset view")) {
                        camera.reset();
                    }
                    ImGui::EndPopup();
                }
            });
        ImGui::SameLine(0.0f, 18.0f);

        scene_control_button(
            "pan",
            "Pan",
            u8"\ue55d",
            [&](const char* popup_id, bool hovered) {
                if (hovered) {
                    ImGui::SetTooltip("Translate the camera laterally. Keys: WASD");
                }
                if (ImGui::BeginPopup(popup_id)) {
                    ImGui::TextUnformatted("Pan camera");
                    ImGui::Separator();
                    if (ImGui::MenuItem("Pan left", "A")) {
                        camera.pan(-pan_step_units, 0.0f);
                    }
                    if (ImGui::MenuItem("Pan right", "D")) {
                        camera.pan(pan_step_units, 0.0f);
                    }
                    if (ImGui::MenuItem("Pan up")) {
                        camera.pan(0.0f, pan_step_units);
                    }
                    if (ImGui::MenuItem("Pan down")) {
                        camera.pan(0.0f, -pan_step_units);
                    }
                    ImGui::EndPopup();
                }
            });
        ImGui::SameLine(0.0f, 18.0f);

        scene_control_button(
            "zoom",
            "Zoom",
            u8"\ue8ff",
            [&](const char* popup_id, bool hovered) {
                if (hovered) {
                    ImGui::SetTooltip("Adjust zoom. Use mouse wheel for quick changes.");
                }
                if (ImGui::BeginPopup(popup_id)) {
                    ImGui::TextUnformatted("Zoom & dolly");
                    ImGui::Separator();
                    if (ImGui::MenuItem("Zoom in", "Wheel up")) {
                        camera.zoomBy(zoom_step_deg);
                    }
                    if (ImGui::MenuItem("Zoom out", "Wheel down")) {
                        camera.zoomBy(-zoom_step_deg);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Dolly closer")) {
                        camera.dolly(dolly_step_units);
                    }
                    if (ImGui::MenuItem("Dolly farther")) {
                        camera.dolly(-dolly_step_units);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Reset zoom")) {
                        camera.setZoom(45.0f);
                    }
                    ImGui::EndPopup();
                }
            });
        ImGui::SameLine(0.0f, 18.0f);
        scene_control_button(
            "help",
            "Controls",
            u8"\ue887",
            [&](const char* popup_id, bool hovered) {
                if (hovered) {
                    ImGui::SetTooltip("Show keyboard and mouse shortcuts");
                }
                if (ImGui::BeginPopup(popup_id)) {
                    ImGui::TextUnformatted("Scene controls");
                    ImGui::Separator();
                    ImGui::TextUnformatted("Orbit: left-drag, Q/E, arrows, IJKL");
                    ImGui::TextUnformatted("Pan: right/middle drag, WASD");
                    ImGui::TextUnformatted("Zoom: mouse wheel or Zoom menu");
                    ImGui::TextUnformatted("Reset: Rotate→Reset view, Zoom→Reset zoom");
                    ImGui::Separator();
                    ImGui::TextUnformatted("Space: zero body rates");
                    ImGui::EndPopup();
                }
            });
    }
    ui::EndCard();

    panelManager.drawAll(simulationState, camera);
}

void Application::renderLegacyLayout() {
    ImGui::SetNextWindowPos(ImVec2(32.0f, 32.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(700.0f, 480.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Quaternion Playground")) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.x = std::max(avail.x, 1.0f);
        avail.y = std::max(avail.y, 1.0f);
        ImTextureID scene_texture = renderSceneToTexture(avail);
        if (scene_texture) {
            ImGui::Image(scene_texture,
                         avail,
                         ImVec2(0.0f, 1.0f),
                         ImVec2(1.0f, 0.0f));
        } else {
            ImGui::TextUnformatted("Scene renderer unavailable");
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(760.0f, 32.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360.0f, 260.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Quaternion Controls")) {
        bool use_modern_dashboard = !simulationState.control.use_legacy_ui;
        if (ImGui::Checkbox("Use modern dashboard", &use_modern_dashboard)) {
            simulationState.control.use_legacy_ui = !use_modern_dashboard;
        }

        ImGui::Separator();
        float body_rates[3] = {
            static_cast<float>(simulationState.angular_rate_deg_per_sec.x),
            static_cast<float>(simulationState.angular_rate_deg_per_sec.y),
            static_cast<float>(simulationState.angular_rate_deg_per_sec.z)
        };
        if (ImGui::SliderFloat3("Body Rates (deg/s)", body_rates, -360.0f, 360.0f, "%.1f")) {
            simulationState.angular_rate_deg_per_sec = glm::dvec3(body_rates[0], body_rates[1], body_rates[2]);
        }
        if (ImGui::Button("Zero Rates")) {
            simulationState.angular_rate_deg_per_sec = glm::dvec3(0.0);
        }

        bool paused = simulationState.control.paused;
        if (ImGui::Checkbox("Pause Simulation", &paused)) {
            simulationState.control.paused = paused;
        }

        bool use_fixed_dt = simulationState.control.use_fixed_dt;
        if (ImGui::Checkbox("Use Fixed dt", &use_fixed_dt)) {
            simulationState.control.use_fixed_dt = use_fixed_dt;
        }
        if (simulationState.control.use_fixed_dt) {
            double fixed_dt = simulationState.control.fixed_dt;
            if (ImGui::DragScalar("Fixed dt (s)", ImGuiDataType_Double, &fixed_dt, 0.0001, nullptr, nullptr, "%.4f")) {
                fixed_dt = std::clamp(fixed_dt, 1e-5, 0.5);
                simulationState.control.fixed_dt = fixed_dt;
            }
        } else {
            float time_scale = static_cast<float>(simulationState.control.time_scale);
            if (ImGui::SliderFloat("Time Scale", &time_scale, 0.0f, 2.0f, "%.2f")) {
                simulationState.control.time_scale = std::max(0.0, static_cast<double>(time_scale));
            }
        }

        ImGui::Separator();
        ImGui::Text("Last dt: %.5f s", simulationState.last_dt);
        ImGui::Text("Sim time: %.2f s", simulationState.time_seconds);
        if (ImGui::Button("Reset Simulation Time")) {
            simulationState.time_seconds = 0.0;
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(760.0f, 312.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360.0f, 220.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Orientation State")) {
        ImGui::Text("Quaternion");
        ImGui::Text("[%.4f, %.4f, %.4f, %.4f]",
                    simulationState.quaternion[0],
                    simulationState.quaternion[1],
                    simulationState.quaternion[2],
                    simulationState.quaternion[3]);

        ImGui::Separator();
        ImGui::Text("Euler (deg)");
        ImGui::Text("Roll %.1f  Pitch %.1f  Yaw %.1f",
                    rad2deg(simulationState.euler.roll),
                    rad2deg(simulationState.euler.pitch),
                    rad2deg(simulationState.euler.yaw));

        ImGui::Separator();
        ImGui::Text("Body Rates (deg/s)");
        ImGui::Text("Roll %.1f  Pitch %.1f  Yaw %.1f",
                    simulationState.angular_rate_deg_per_sec.x,
                    simulationState.angular_rate_deg_per_sec.y,
                    simulationState.angular_rate_deg_per_sec.z);
    }
    ImGui::End();
}

void Application::updateCamera(float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardInput(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardInput(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardInput(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardInput(RIGHT, deltaTime);
}

void Application::initializePanels() {
    panelManager.registerPanel(std::make_unique<ControlPanel>());
    panelManager.registerPanel(std::make_unique<TelemetryPanel>());
    panelManager.registerPanel(std::make_unique<RotorPanel>());
    panelManager.registerPanel(std::make_unique<PowerPanel>());
    panelManager.registerPanel(std::make_unique<SensorPanel>());
    panelManager.registerPanel(std::make_unique<DynamicsPanel>());
    panelManager.registerPanel(std::make_unique<EstimatorPanel>());
}

ImTextureID Application::renderSceneToTexture(const ImVec2& size) {
    int requested_width = std::max(1, static_cast<int>(size.x));
    int requested_height = std::max(1, static_cast<int>(size.y));

    if (!ensureRenderTarget(requested_width, requested_height)) {
        return static_cast<ImTextureID>(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, requested_width, requested_height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.06f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Transform scene_transform = transform;
    scene_transform.projection = camera.getProjectionMatrix(static_cast<float>(requested_width) /
                                                            static_cast<float>(requested_height));

    renderer.renderFrame3D(scene_transform);
    axisRenderer.render3D(scene_transform);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int window_width = 0;
    int window_height = 0;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    return static_cast<ImTextureID>(renderTexture);
}

bool Application::ensureRenderTarget(int width, int height) {
    if (width == sceneWidth && height == sceneHeight &&
        fbo != 0 && renderTexture != 0 && depthBuffer != 0) {
        return true;
    }

    destroyRenderTarget();
    sceneWidth = width;
    sceneHeight = height;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    bool complete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!complete) {
        destroyRenderTarget();
        return false;
    }
    return true;
}

void Application::destroyRenderTarget() {
    if (depthBuffer) {
        glDeleteRenderbuffers(1, &depthBuffer);
        depthBuffer = 0;
    }
    if (renderTexture) {
        glDeleteTextures(1, &renderTexture);
        renderTexture = 0;
    }
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    sceneWidth = 0;
    sceneHeight = 0;
}
