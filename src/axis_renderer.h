#ifndef AXIS_RENDERER_H
#define AXIS_RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>  // GLEW must come first
#include <string>
#include "transform.h"

class AxisRenderer {
public:
    GLuint vao, vbo;
    GLuint shaderProgram;
    glm::mat4 view, projection;

    Transform axisTransform;  // Add Transform member

    AxisRenderer();
    ~AxisRenderer();

    bool init();
    void render(const Transform& transform);
    void shutdown();

private:
    void setupAxisGeometry();
    GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);
};

#endif // AXIS_RENDERER_H

