#ifndef APPLICATION_H
#define APPLICATION_H

#include <GLFW/glfw3.h>
#include "attitude/euler.h" // from your attitude library
#include "renderer.h"

class Application {
public:
    Application();
    ~Application();

    // Initialize the application: create window, init renderer, etc.
    bool init();

    // Check if the application should keep running
    bool running() const;

    // Update logic (e.g., handle user input, update orientation)
    void update();

    // Render the current frame
    void render();

    // Cleanup before exit
    void shutdown();

private:
    GLFWwindow* window;
    Renderer renderer;

    // Orientation state from the attitude library
    EulerAngles currentOrientation;

    // Handle input, like keyboard controls
    void processInput();
};

#endif // APPLICATION_H

