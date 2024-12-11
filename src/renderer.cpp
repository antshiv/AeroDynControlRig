#include "renderer.h"
#include <iostream>
#include <GL/glew.h>
// If using Glad: #include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <fstream>
#include <sstream>

Renderer::Renderer() {
    // Initialize members if needed
    vao = 0; vbo = 0; ebo = 0;
    shaderProgram = 0;
    indexCount = 0;
    modelLoc = viewLoc = projLoc = -1;
    // Initialize model matrix or leave as is
}

Renderer::~Renderer() {
    // Any cleanup if needed
}


// Forward declare a helper for creating shaders
static GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);

bool Renderer::init() {
    // Initialize GLEW or GLAD if not done elsewhere
    // Check if context is current in Application::init()

    shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (!shaderProgram) {
        std::cerr << "Failed to create shader program.\n";
        return false;
    }

    // Setup geometry (VAO, VBO, EBO) and indexCount, model/view/proj matrix locations
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projLoc = glGetUniformLocation(shaderProgram, "projection");

    setupCubeGeometry(); // Implement this to create VAO, VBO, EBO, set indexCount
    return true;
}

void Renderer::renderFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // Set your uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::setModelMatrix(const float* modelMat) {
    // Copy modelMat into this->model
    std::memcpy(model, modelMat, 16*sizeof(float));
}

void Renderer::setViewMatrix(const glm::mat4& v) {
    view = v;
}

void Renderer::setProjectionMatrix(const glm::mat4& p) {
    projection = p;
}

void Renderer::shutdown() {
    // Cleanup: glDeleteProgram(shaderProgram), glDeleteBuffers, etc.
}

void Renderer::setupCubeGeometry() {
    // Create VAO, VBO, EBO, set indexCount
    // This sets up the cube vertices and indices
}

// ... implement createShaderProgram(...) as per previous snippet or inline here
static std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static GLuint compileShader(const char* source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check compile errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vSource = loadShaderSource(vertexPath);
    std::string fSource = loadShaderSource(fragmentPath);

    GLuint vertexShader = compileShader(vSource.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fSource.c_str(), GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program Linking Failed:\n" << infoLog << std::endl;
    }

    // Shaders are linked into the program now, so we can delete them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}



