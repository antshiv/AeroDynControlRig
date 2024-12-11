#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
public:
    glm::mat4 model;      // Local transformations
    glm::mat4 view;       // Camera transformations
    glm::mat4 projection; // Projection matrix

    Transform() 
        : model(glm::mat4(1.0f)), view(glm::mat4(1.0f)), projection(glm::mat4(1.0f)) {}

    // Reset model matrix to identity
    void resetModel() {
        model = glm::mat4(1.0f);
    }

    // Set translation
    void setTranslation(const glm::vec3& translation) {
        model = glm::translate(glm::mat4(1.0f), translation);
    }

    // Set rotation
    void setRotation(float angle, const glm::vec3& axis) {
        model = glm::rotate(glm::mat4(1.0f), angle, axis);
    }

    // Set scaling
    void setScale(const glm::vec3& scale) {
        model = glm::scale(glm::mat4(1.0f), scale);
    }

    // Set view (camera) matrix
    void setView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
        view = glm::lookAt(position, target, up);
    }

    // Set projection matrix (perspective)
    void setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
        projection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }

    // Set projection matrix (orthographic for 2D)
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        projection = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    }
};

#endif // TRANSFORM_H

