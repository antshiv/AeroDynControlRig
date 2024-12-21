#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Usually defined in camera.h
enum Camera_Movement {
    FORWARD,    // Moving in the direction camera is facing
    BACKWARD,   // Moving away from direction camera is facing
    LEFT,       // Strafing left
    RIGHT,      // Strafing right
    UP,         // Moving upward in world space
    DOWN        // Moving downward in world space
};

class Camera {
public:


    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    glm::vec3 target;

    float yaw;
    float pitch;
    float zoom;

    void reset() {
        // Reset camera position and orientation to defaults
        position = glm::vec3(0.0f, 0.0f, 3.0f); // Example position
        target = glm::vec3(0.0f, 0.0f, 0.0f);   // Look at origin
        up = glm::vec3(0.0f, 1.0f, 0.0f);       // Up direction
    }

    Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = -90.0f,
           float startPitch = 0.0f);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    void processKeyboardInput_old(int key);
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void processMouseScroll(float yOffset);
    void processKeyboardInput(Camera_Movement direction, float deltaTime);  // Update

private:
    float movementSpeed = 2.5f;  // Add this as a member variable
    void updateCameraVectors();

};

#endif // CAMERA_H

