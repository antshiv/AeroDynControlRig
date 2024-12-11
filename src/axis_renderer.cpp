#include "axis_renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

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
    setupAxisGeometry();

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


