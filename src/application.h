#ifndef APPLICATION_H
#define APPLICATION_H

#include <GL/glew.h>  // MUST come first
#include <GLFW/glfw3.h>
#include "attitude/euler.h" // from your attitude library
#include "renderer.h"
#include "transform.h"
#include "axis_renderer.h"

class Application {
public:
    Application();
    ~Application();

    Transform transform; // Transformation manager

    AxisRenderer axisRenderer;
    int windowHeight, windowWidth;
	
    
    // Initialize the application: create window, init renderer, etc.
    bool init();

    // Check if the application should keep running
    bool running() const;

    // Update logic (e.g., handle user input, update orientation)
    void update();
    void update2D();
    void update2D_old();

    // Render the current frame
    void render();
    void render2D();
    void renderAxis();

    // Cleanup before exit
    void shutdown();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouseCallback(GLFWwindow* window, double xpos, double ypos);

private:
    GLFWwindow* window;
    Renderer renderer;

    // Orientation state from the attitude library
    EulerAngles currentOrientation;

    // Handle input, like keyboard controls
    void processInput();
};

#endif // APPLICATION_H

