#include "axis_renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

AxisRenderer::AxisRenderer() : vao(0), vbo(0), shaderProgram(0) {}

AxisRenderer::~AxisRenderer() {
    shutdown();
}

bool AxisRenderer::init() {
    // Load and compile shaders
    shaderProgram = createShaderProgram("shaders/vertex3D.glsl", "shaders/fragment3D.glsl");
    if (!shaderProgram) {
        std::cerr << "Failed to create shader program for axis." << std::endl;
        return false;
    }

    // Set up geometry for the axis
    setupAxisGeometry3D();

    // Set up fixed view and projection for bottom-left rendering
    view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    projection = glm::perspective(
        glm::radians(45.0f), // Field of view
        1.0f,                // Aspect ratio for square viewport
        0.1f,                // Near plane
        100.0f               // Far plane
    );

    return true;
}

void AxisRenderer::setupAxisGeometry() {
    float vertices[] = {
        // Positions       // Colors
        // X-axis (Red)
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        // Y-axis (Green)
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        // Z-axis (Blue)
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void AxisRenderer::render(const Transform& mainTransform) {
    glUseProgram(shaderProgram);

    // Debug: Print main transform matrix
    std::cout << "Main transform model matrix:" << std::endl;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            std::cout << mainTransform.model[i][j] << " ";
        }
        std::cout << std::endl;
    }

    // Extract rotation and create new model matrix
    glm::mat3 rotationOnly = glm::mat3(mainTransform.model);
    axisTransform.model = glm::mat4(rotationOnly);

    // Debug: Print axis transform matrix
    std::cout << "Axis transform model matrix:" << std::endl;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            std::cout << axisTransform.model[i][j] << " ";
        }
        std::cout << std::endl;
    }

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &axisTransform.model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &axisTransform.view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &axisTransform.projection[0][0]);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void AxisRenderer::shutdown() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

static std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static GLuint compileShader(const char* filepath, GLenum shaderType) {
    // Load shader source
    std::string source = loadShaderSource(filepath);
    if (source.empty()) {
        std::cerr << "Shader source is empty for file: " << filepath << std::endl;
        return 0;
    }

    // Debug output: Print shader source
    std::cout << "Compiling Shader: " << filepath << "\n" << source << std::endl;

    // Compile shader
    const char* src = source.c_str();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader Compilation Failed (" << filepath << "):\n" << infoLog << std::endl;
        return 0;
    }

    return shader;
}

GLuint AxisRenderer::createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    GLuint vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// axis_renderer.cpp
void AxisRenderer::setupAxisGeometry3D() {
    // Each axis will have a line and an arrow head
    // X axis - Red
    // Y axis - Green
    // Z axis - Blue
    
    const float AXIS_LENGTH = 1.0f;
    const float ARROW_HEAD_LENGTH = 0.2f;
    const float ARROW_HEAD_WIDTH = 0.05f;

    std::vector<float> vertices = {
        // X-Axis (Red)
        0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,  // Line start
        AXIS_LENGTH, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,  // Line end
        // X-Axis arrow head
        AXIS_LENGTH, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,  // Tip
        AXIS_LENGTH - ARROW_HEAD_LENGTH, ARROW_HEAD_WIDTH, 0.0f,    1.0f, 0.0f, 0.0f,  // Right
        AXIS_LENGTH - ARROW_HEAD_LENGTH, -ARROW_HEAD_WIDTH, 0.0f,   1.0f, 0.0f, 0.0f,  // Left
        AXIS_LENGTH - ARROW_HEAD_LENGTH, 0.0f, ARROW_HEAD_WIDTH,    1.0f, 0.0f, 0.0f,  // Top
        AXIS_LENGTH - ARROW_HEAD_LENGTH, 0.0f, -ARROW_HEAD_WIDTH,   1.0f, 0.0f, 0.0f,  // Bottom

        // Y-Axis (Green)
        0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,  // Line start
        0.0f, AXIS_LENGTH, 0.0f,    0.0f, 1.0f, 0.0f,  // Line end
        // Y-Axis arrow head
        0.0f, AXIS_LENGTH, 0.0f,    0.0f, 1.0f, 0.0f,  // Tip
        ARROW_HEAD_WIDTH, AXIS_LENGTH - ARROW_HEAD_LENGTH, 0.0f,    0.0f, 1.0f, 0.0f,
        -ARROW_HEAD_WIDTH, AXIS_LENGTH - ARROW_HEAD_LENGTH, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, AXIS_LENGTH - ARROW_HEAD_LENGTH, ARROW_HEAD_WIDTH,    0.0f, 1.0f, 0.0f,
        0.0f, AXIS_LENGTH - ARROW_HEAD_LENGTH, -ARROW_HEAD_WIDTH,   0.0f, 1.0f, 0.0f,

        // Z-Axis (Blue)
        0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,  // Line start
        0.0f, 0.0f, AXIS_LENGTH,    0.0f, 0.0f, 1.0f,  // Line end
        // Z-Axis arrow head
        0.0f, 0.0f, AXIS_LENGTH,    0.0f, 0.0f, 1.0f,  // Tip
        0.0f, ARROW_HEAD_WIDTH, AXIS_LENGTH - ARROW_HEAD_LENGTH,    0.0f, 0.0f, 1.0f,
        0.0f, -ARROW_HEAD_WIDTH, AXIS_LENGTH - ARROW_HEAD_LENGTH,   0.0f, 0.0f, 1.0f,
        ARROW_HEAD_WIDTH, 0.0f, AXIS_LENGTH - ARROW_HEAD_LENGTH,    0.0f, 0.0f, 1.0f,
        -ARROW_HEAD_WIDTH, 0.0f, AXIS_LENGTH - ARROW_HEAD_LENGTH,   0.0f, 0.0f, 1.0f
    };

    // Generate and bind buffers
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AxisRenderer::render3D(const Transform& transform) {
    glUseProgram(shaderProgram);

    // Step 1: Extract rotation from transform.model (ignoring translation)
    glm::mat3 rotationOnly = glm::mat3(transform.model);

    // Step 2: Build axis model matrix: Identity with rotation
    axisTransform.model = glm::mat4(rotationOnly);

    // Step 3: Use a fixed view matrix at the origin to decouple translation
    axisTransform.view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),  // Camera position fixed at (0, 0, 5)
        glm::vec3(0.0f, 0.0f, 0.0f),  // Look at the origin
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );

    // Step 4: Keep the same projection matrix as the main scene
    axisTransform.projection = transform.projection;

    // Step 5: Scale the axis (optional)
    float scale = 0.5f;  // Adjust the axis size
    axisTransform.model = glm::scale(axisTransform.model, glm::vec3(scale));

    // Set uniforms
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &axisTransform.model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &axisTransform.view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &axisTransform.projection[0][0]);

    // Step 6: Draw axis lines
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 2); // X-axis line
    glDrawArrays(GL_LINES, 7, 2); // Y-axis line
    glDrawArrays(GL_LINES, 14, 2); // Z-axis line


    // Step 7: Draw arrowheads
    for (int i = 0; i < 3; i++) {
        int baseVertex = 2 + (i * 7);
        glDrawArrays(GL_TRIANGLE_FAN, baseVertex, 5);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void AxisRenderer::renderCornerGizmo(const Transform& transform, int windowWidth, int windowHeight, int corner) {
    // Save current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Define gizmo size (100x100 pixels)
    const int gizmoSize = 100;
    const int margin = 10;

    // Calculate corner position
    int x, y;
    switch (corner) {
        case 0: // Bottom-left
            x = margin;
            y = margin;
            break;
        case 1: // Bottom-right
            x = windowWidth - gizmoSize - margin;
            y = margin;
            break;
        case 2: // Top-left
            x = margin;
            y = windowHeight - gizmoSize - margin;
            break;
        case 3: // Top-right (default)
        default:
            x = windowWidth - gizmoSize - margin;
            y = windowHeight - gizmoSize - margin;
            break;
    }

    // Set viewport to corner region
    glViewport(x, y, gizmoSize, gizmoSize);

    // Disable depth test for overlay
    glDisable(GL_DEPTH_TEST);

    // Use shader program
    glUseProgram(shaderProgram);

    // Extract rotation from main scene (ignore translation)
    glm::mat3 rotationOnly = glm::mat3(transform.model);
    axisTransform.model = glm::mat4(rotationOnly);

    // Use a fixed view matrix for the gizmo
    axisTransform.view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.5f),  // Camera closer for corner view
        glm::vec3(0.0f, 0.0f, 0.0f),  // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );

    // Use simple projection for corner view
    axisTransform.projection = glm::perspective(
        glm::radians(35.0f),  // Narrower FOV for corner
        1.0f,                  // Square aspect ratio
        0.1f,
        100.0f
    );

    // Scale smaller for corner view
    float scale = 0.4f;
    axisTransform.model = glm::scale(axisTransform.model, glm::vec3(scale));

    // Set uniforms
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &axisTransform.model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &axisTransform.view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &axisTransform.projection[0][0]);

    // Draw axis lines
    glBindVertexArray(vao);
    glLineWidth(2.0f);  // Slightly thicker lines for corner view
    glDrawArrays(GL_LINES, 0, 2);   // X-axis line
    glDrawArrays(GL_LINES, 7, 2);   // Y-axis line
    glDrawArrays(GL_LINES, 14, 2);  // Z-axis line

    // Draw arrowheads
    for (int i = 0; i < 3; i++) {
        int baseVertex = 2 + (i * 7);
        glDrawArrays(GL_TRIANGLE_FAN, baseVertex, 5);
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glLineWidth(1.0f);  // Reset line width

    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);

    // Restore original viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

