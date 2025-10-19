#include "application.h"
#include "render/renderer.h"
#include "modules/quaternion_demo.h"
#include "attitude/euler.h"
#include "attitude/dcm.h"
#include "attitude/attitude_utils.h"
#include <ostream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
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
    for (auto& module : modules) {
        module->initialize(simulationState);
    }
    transform.model = simulationState.model_matrix;
}



void Application::tick() {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    updateCamera(deltaTime);

    for (auto& module : modules) {
        module->update(deltaTime, simulationState);
    }

    transform.model = simulationState.model_matrix;
    render3D();
}

void Application::update2D_old() {}

void Application::update2D() {}



void Application::render() {
        renderer.renderFrame();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
}

void Application::render2D() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // First render main scene
    glViewport(0, 0, width, height);
    renderer.renderFrame2D(transform);
    
    // Then render axis overlay
    int axisSize = std::min(width, height) / 6;
    glViewport(width - axisSize - 10, height - axisSize - 10, axisSize, axisSize);
    axisRenderer.render(transform);
    
    // Swap buffers once at the end of frame
    glfwSwapBuffers(window);
    glfwPollEvents();
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
    // Retrieve the Application instance from the user pointer
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) {
        std::cerr << "Application instance is null in keyCallback" << std::endl;
        return;
    }

    Transform& transform = app->transform; // Access the Transform object

    std::cout << "Key pressed: " << key << std::endl;
    std::cout << "Transform Model Matrix: " << glm::to_string(transform.model) << std::endl;

    // Check if the key is pressed or held
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        // Translation
        case GLFW_KEY_W: // Move up (positive Y-axis)
            transform.setTranslation(glm::vec3(0.0f, 0.1f, 0.0f));
            break;
        case GLFW_KEY_S: // Move down (negative Y-axis)
            transform.setTranslation(glm::vec3(0.0f, -0.1f, 0.0f));
            break;
        case GLFW_KEY_A: // Move left (negative X-axis)
            transform.setTranslation(glm::vec3(-0.1f, 0.0f, 0.0f));
            break;
        case GLFW_KEY_D: // Move right (positive X-axis)
            transform.setTranslation(glm::vec3(0.1f, 0.0f, 0.0f));
            break;
        case GLFW_KEY_R: // Move forward (positive Z-axis)
            transform.setTranslation(glm::vec3(0.0f, 0.0f, 0.1f));
            break;
        case GLFW_KEY_F: // Move backward (negative Z-axis)
            transform.setTranslation(glm::vec3(0.0f, 0.0f, -0.1f));
            break;

        // Rotation
        case GLFW_KEY_Q: // Rotate counterclockwise around Z-axis
            transform.setRotation(glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case GLFW_KEY_E: // Rotate clockwise around Z-axis
            transform.setRotation(glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case GLFW_KEY_T: // Rotate counterclockwise around X-axis
            transform.setRotation(glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case GLFW_KEY_Y: // Rotate clockwise around X-axis
            transform.setRotation(glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case GLFW_KEY_G: // Rotate counterclockwise around Y-axis
            transform.setRotation(glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case GLFW_KEY_H: // Rotate clockwise around Y-axis
            transform.setRotation(glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            break;

        // Scaling
        case GLFW_KEY_Z: // Scale up uniformly
            transform.setScale(glm::vec3(1.1f, 1.1f, 1.1f));
            break;
        case GLFW_KEY_X: // Scale down uniformly
            transform.setScale(glm::vec3(0.9f, 0.9f, 0.9f));
            break;

        default:
            break;
        }
    }
}


void Application::mouseCallback_old(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = xpos, lastY = ypos;
    double deltaX = xpos - lastX;
    double deltaY = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    // Update yaw and pitch based on mouse movement
    simulationState.euler.yaw += deltaX * 0.005f;
    simulationState.euler.pitch += deltaY * 0.005f;

    // Clamp pitch to prevent flipping
//    currentOrientation.pitch = glm::clamp(currentOrientation.pitch, -glm::half_pi<float>(), glm::half_pi<float>());
}

void Application::renderAxis() {
}

// Camera mouse callback
void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = xpos, lastY = ypos;
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->camera.processMouseMovement(xOffset, yOffset);
    }
}

// Camera scroll callback
void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->camera.processMouseScroll(yoffset);
    }
}

void Application::render3D1() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspectRatio = static_cast<float>(width) / height;
    
    // Main 3D viewport
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);  // Enable depth testing for 3D
    
    // Update the transform with camera matrices
    transform.view = camera.getViewMatrix();
    transform.projection = camera.getProjectionMatrix(aspectRatio);
    
    // Render main 3D scene
    renderer.renderFrame3D(transform);
    
    // Render axis overlay (with its own view/projection)
    int axisSize = std::min(width, height) / 6;
    glViewport(width - axisSize - 10, height - axisSize - 10, axisSize, axisSize);
    axisRenderer.render3D(transform);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
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

    // Control Panel
    ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("3D Scene Controls");
    ImGui::SliderFloat("Camera Zoom", &camera.zoom, 0.1f, 100.0f);
    float rotationSpeed = static_cast<float>(simulationState.rotation_speed_deg_per_sec);
    if (ImGui::SliderFloat("Rotation Speed (deg/s)", &rotationSpeed, 0.1f, 180.0f)) {
        simulationState.rotation_speed_deg_per_sec = rotationSpeed;
    }
    if (ImGui::Button("Reset View")) {
        camera.reset();
    }
    ImGui::End();

    // Data Display
    ImGui::Begin("Data", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Orientation Data:");
    ImGui::Text("Roll: %.1f", rad2deg(simulationState.euler.roll));
    ImGui::Text("Pitch: %.1f", rad2deg(simulationState.euler.pitch));
    ImGui::Text("Yaw: %.1f", rad2deg(simulationState.euler.yaw));
    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Step 5: Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::updateCamera(float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardInput(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardInput(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardInput(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardInput(RIGHT, deltaTime);
}

void Application::render3DView() {
    if (ImGui::Begin("3D View")) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        int width = static_cast<int>(size.x);
        int height = static_cast<int>(size.y);

        // Set OpenGL viewport and clear buffers
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Set camera matrices and render the 3D scene
        transform.view = camera.getViewMatrix();
        transform.projection = camera.getProjectionMatrix(static_cast<float>(width) / height);
        renderer.renderFrame3D(transform);

        // Render the axis overlay
        int axisSize = std::min(width, height) / 6;
        glViewport(width - axisSize - 10, height - axisSize - 10, axisSize, axisSize);
        axisRenderer.render3D(transform);
    }
    ImGui::End();
}

void Application::renderTopView() {
    if (ImGui::Begin("Top View")) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        int width = static_cast<int>(size.x);
        int height = static_cast<int>(size.y);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Set an orthographic top-down view
        transform.setOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        renderer.renderFrame3D(transform);

        // Render the axis overlay
        axisRenderer.render3D(transform);
    }
    ImGui::End();
}

void Application::renderFrontView() {
    if (ImGui::Begin("Front View")) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        int width = static_cast<int>(size.x);
        int height = static_cast<int>(size.y);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Set an orthographic front view
        transform.setOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        renderer.renderFrame3D(transform);

        // Render the axis overlay
        axisRenderer.render3D(transform);
    }
    ImGui::End();
}

void Application::renderSideView() {
    if (ImGui::Begin("Side View")) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        int width = static_cast<int>(size.x);
        int height = static_cast<int>(size.y);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Set an orthographic side view
        transform.setOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        renderer.renderFrame3D(transform);

        // Render the axis overlay
        axisRenderer.render3D(transform);
    }
    ImGui::End();
}


void Application::renderControlPanel() {
    if (ImGui::Begin("Control Panel")) {
        ImGui::Text("Control Panel");
        static float rotationSpeed = 1.0f;
        ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.1f, 10.0f);
        ImGui::Text("Use this panel to adjust settings and view data.");
    }
    ImGui::End();
}
