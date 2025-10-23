#include "application.h"
#include "render/renderer.h"
#include "modules/quaternion_demo.h"
#include "modules/first_order_dynamics.h"
#include "modules/sensor_simulator.h"
#include "modules/complementary_estimator.h"
#include "gui/panel_manager.h"
#include "gui/panels/control_panel.h"
#include "gui/panels/telemetry_panel.h"
#include "gui/panels/dynamics_panel.h"
#include "gui/panels/estimator_panel.h"
#include "attitude/euler.h"
#include "attitude/dcm.h"
#include "attitude/attitude_utils.h"
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


Application::Application() {
    // Constructor code
}

Application::~Application() {
    // Destructor code
}

bool Application::init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Create GLFW window before any ImGui initialization
    window = glfwCreateWindow(800, 600, "Dynamic Control System Test Rig", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    std::cout << "Window created successfully: " << window << std::endl;

    // Set GLFW callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    //glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
    //    static_cast<Application*>(glfwGetWindowUserPointer(w))->mouseCallback(w, x, y);
    //});
    glfwSetScrollCallback(window, scrollCallback);

    // Check OpenGL version
    const GLubyte* rendererName = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    if (rendererName && version) {
        std::cout << "Renderer: " << rendererName << "\nOpenGL version: " << version << std::endl;
    } else {
        std::cerr << "Failed to retrieve OpenGL context information." << std::endl;
        return false;
    }

    // Initialize ImGui context AFTER the window is created
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io; // Suppress unused variable warning

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Initialize ImGui backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize renderers
    if (!renderer.init()) {
        std::cerr << "Renderer initialization failed." << std::endl;
        return false;
    }

    if (!axisRenderer.init()) {
        std::cerr << "Axis renderer initialization failed." << std::endl;
        return false;
    }

    // Set orthographic projection for 2D
    float aspectRatio = 800.0f / 600.0f; // Adjust dynamically if needed
    transform.setOrthographic(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);

    initializeModules();
    initializePanels();
    lastFrame = glfwGetTime();

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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Step 2: Render OpenGL 3D content
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // Set view and projection matrices
    transform.view = camera.getViewMatrix();
    transform.projection = camera.getProjectionMatrix(static_cast<float>(width) / height);

    // Render the 3D scene
    renderer.renderFrame3D(transform);
    // Render axis overlay (with its own view/projection)
    int axisSize = std::min(width, height) / 3;
    glViewport(width - axisSize - 10, height - axisSize - 10, axisSize, axisSize);
    axisRenderer.render3D(transform);

    // Step 4: Render ImGui UI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    panelManager.drawAll(simulationState, camera);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Step 5: Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
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
    panelManager.registerPanel(std::make_unique<DynamicsPanel>());
    panelManager.registerPanel(std::make_unique<EstimatorPanel>());
}
