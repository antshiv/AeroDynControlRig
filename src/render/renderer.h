#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "core/transform.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize OpenGL state, load shaders, setup VAOs/VBOs
    bool init();

    // Called each frame to render
    void renderFrame();
    void renderFrame2D(const Transform& transform);

    void renderFrame3D(const Transform& transform);

    // Set the model matrix for the current frame
    void setModelMatrix(const float* modelMat);

    // Provide a way to set view and projection matrices
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& proj);

    // Optional shutdown if needed
    void shutdown();

private:
    unsigned int shaderProgram;
    unsigned int vao, vbo, ebo;

    unsigned int indexCount;

    // Locations of uniforms
    int modelLoc;
    int viewLoc;
    int projLoc;

    // Matrices
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    // Helper methods
    unsigned int loadShader(const char* vertexSrc, const char* fragmentSrc);
    void setupCubeGeometry();
    void setupCubeGeometry3D();
    void setupAxisGeometry();
    void setDefaultMatrices();

};

#endif // RENDERER_H
