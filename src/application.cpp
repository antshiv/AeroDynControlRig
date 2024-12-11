#include "application.h"
#include "renderer.h"
#include "attitude/euler.h"
#include "attitude/dcm.h"
#include "attitude/attitude_utils.h"
#include <ostream>
#include <iostream>

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

void Application::render() {
        renderer.renderFrame();

        // Clear the screen with a color (RGBA)
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
}

void Application::shutdown() {
    // Cleanup resources
    //renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

