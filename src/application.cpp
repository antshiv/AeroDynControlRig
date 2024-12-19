#include "application.h"
#include "renderer.h"
#include "attitude/euler.h"
#include "attitude/dcm.h"
#include "attitude/attitude_utils.h"
#include <ostream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>


Application::Application() {
    // Constructor code
}

Application::~Application() {
    // Destructor code
}


bool Application::init() {
    // Initialize window, OpenGL context
    // Setup any needed data structures
    // Initialize orientation
//    currentOrientation.roll = deg2rad(0.0);
//    currentOrientation.pitch = deg2rad(0.0);
//    currentOrientation.yaw = deg2rad(0.0);
//    currentOrientation.order = EULER_ZYX;

    float aspectRatio = 800.0f / 600.0f;  // or dynamically: window_width / window_height


    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    window = glfwCreateWindow(800, 600, "Dynamic Control System Test Rig", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

	glfwMakeContextCurrent(window);
	std::cout << "Window created successfully: " << window << std::endl;

    glfwMakeContextCurrent(window);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

	    // Set the Application instance as the GLFW user pointer
    glfwSetWindowUserPointer(window, this);

    // Set the key callback
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        static_cast<Application*>(glfwGetWindowUserPointer(w))->mouseCallback(w, x, y);
    });
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

   if (!renderer.init()) {
	std::cerr << "Renderer initialization failed." << std::endl;
	return false;
    }

   if (!axisRenderer.init()) {
        std::cerr << "Axis renderer initialization failed." << std::endl;
        return false;
    }

        // Set orthographic projection for 2D
    // Set orthographic projection matching window dimensions
    // Option 1: Maintain height of 2 units (-1 to 1) and scale width by aspect ratio
    //transform.setOrthographic(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    transform.setOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
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



void Application::update() {
    // Update your Euler angles or quaternions here:
    // For example, rotate yaw a bit each frame:
    currentOrientation.yaw += deg2rad(0.5);

    // Convert Euler to DCM
    double dcm[3][3];
    euler_to_dcm(&currentOrientation, dcm);

    // Convert DCM to a 4x4 matrix for OpenGL
    float R[16] = {
        (float)dcm[0][0], (float)dcm[0][1], (float)dcm[0][2], 0.0f,
        (float)dcm[1][0], (float)dcm[1][1], (float)dcm[1][2], 0.0f,
        (float)dcm[2][0], (float)dcm[2][1], (float)dcm[2][2], 0.0f,
        0.0f,             0.0f,             0.0f,             1.0f
    };

    renderer.setModelMatrix(R);
}

void Application::update2D_old() {
    //transform.resetModel();
    currentOrientation.yaw += deg2rad(0.5);

    double dcm[3][3];
    euler_to_dcm(&currentOrientation, dcm);

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = dcm[0][0]; rotation[0][1] = dcm[0][1];
    rotation[1][0] = dcm[1][0]; rotation[1][1] = dcm[1][1];

    transform.model = rotation; // Update model matrix with rotation
}

void Application::update2D() {
    // Add yaw rotation based on Euler angles
    currentOrientation.yaw += deg2rad(0.5);

    // Compute rotation matrix from Euler angles
    double dcm[3][3];
    euler_to_dcm(&currentOrientation, dcm);

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = dcm[0][0]; rotation[0][1] = dcm[0][1];
    rotation[1][0] = dcm[1][0]; rotation[1][1] = dcm[1][1];

    // Combine the current model matrix with the rotation matrix
    transform.model = rotation * transform.model; // Apply rotation without overwriting

    // Debug output to verify transformations
    std::cout << "Updated Model Matrix: " << glm::to_string(transform.model) << std::endl;
}



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
    currentOrientation.yaw += deltaX * 0.005f; // Sensitivity adjustment
    currentOrientation.pitch += deltaY * 0.005f;

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

void Application::render3D() {
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

void Application::update3D() {
    // Update camera based on input
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardInput(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardInput(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardInput(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardInput(RIGHT, deltaTime);
}
