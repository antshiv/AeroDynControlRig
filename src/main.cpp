#include "application.h"
#include <iostream>

int main() {
    Application app;
    if (!app.init()) {
        return -1;
    }

    while (app.running()) {
       // app.update();
	std::cout << "Reder running" << std::endl;
        app.render();
    }

    app.shutdown();
    return 0;
} 
/*
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW window hints (optional)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL version 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use core profile

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Dynamic Control System Test Rig", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Load OpenGL function pointers (using GLAD, if available)
    // gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen with a color (RGBA)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up and close
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
*/
