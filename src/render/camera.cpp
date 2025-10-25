#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch), zoom(45.0f) {
    updateCameraVectors();
    target = position + front;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(zoom), aspectRatio, 0.1f, 100.0f);
}

void Camera::processKeyboardInput_old(int key) {
    float speed = 0.1f;
    glm::vec3 forward = front * speed;
    glm::vec3 sideways = right * speed;

    if (key == 'W') position += forward;
    if (key == 'S') position -= forward;
    if (key == 'A') position -= sideways;
    if (key == 'D') position += sideways;
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (constrainPitch) {
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset) {
    zoom -= yOffset;
    zoom = glm::clamp(zoom, 1.0f, 45.0f);
}

void Camera::reset() {
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    zoom = 45.0f;
    updateCameraVectors();
}

void Camera::orbit(float yawDegrees, float pitchDegrees) {
    yaw += yawDegrees;
    pitch = glm::clamp(pitch + pitchDegrees, -89.0f, 89.0f);
    updateCameraVectors();
}

void Camera::pan(float rightDelta, float upDelta) {
    glm::vec3 offset = right * rightDelta + up * upDelta;
    position += offset;
    target = position + front;
}

void Camera::dolly(float distance) {
    glm::vec3 offset = front * distance;
    position += offset;
    target = position + front;
}

void Camera::zoomBy(float delta) {
    zoom = glm::clamp(zoom - delta, 1.0f, 45.0f);
}

void Camera::setZoom(float newZoom) {
    zoom = glm::clamp(newZoom, 1.0f, 45.0f);
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
    target = position + front;
}

void Camera::processKeyboardInput(Camera_Movement direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    
    switch(direction) {
        case FORWARD:
            position += front * velocity;
            break;
        case BACKWARD:
            position -= front * velocity;
            break;
        case LEFT:
            position -= right * velocity;
            break;
        case RIGHT:
            position += right * velocity;
            break;
        case UP:
            position += up * velocity;
            break;
        case DOWN:
            position -= up * velocity;
            break;
    }
}
