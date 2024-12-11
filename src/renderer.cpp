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

// Renderer.cpp
float vertices[] = {
    -0.5f, -0.5f, 0.0f, // Bottom-left
     0.5f, -0.5f, 0.0f, // Bottom-right
     0.0f,  0.5f, 0.0f  // Top-center
};


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
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    std::cout << "GLEW initialized successfully." << std::endl;

    glEnable(GL_DEPTH_TEST);

    // Compile and link shaders
    shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (!shaderProgram) {
        std::cerr << "Failed to create shader program" << std::endl;
        return false;
    }

    // Get uniform locations
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projLoc = glGetUniformLocation(shaderProgram, "projection");

    if (modelLoc == -1 || viewLoc == -1 || projLoc == -1) {
        std::cerr << "Error: Uniform location not found. Check shader code." << std::endl;
        return false;
    }

    // Set up geometry
    setupCubeGeometry();
	
    setDefaultMatrices();

    return true;
}

void Renderer::renderFrame() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear screen and depth buffer

    glUseProgram(shaderProgram);

    // Pass transformation matrices to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error during rendering: " << error << std::endl;
    }
}


void Renderer::setModelMatrix(const float* modelMat) {
    // Copy modelMat into this->model
    //std::memcpy(model, modelMat, 16*sizeof(float));
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
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  // Back-bottom-left
         0.5f, -0.5f, -0.5f,  // Back-bottom-right
         0.5f,  0.5f, -0.5f,  // Back-top-right
        -0.5f,  0.5f, -0.5f,  // Back-top-left
        -0.5f, -0.5f,  0.5f,  // Front-bottom-left
         0.5f, -0.5f,  0.5f,  // Front-bottom-right
         0.5f,  0.5f,  0.5f,  // Front-top-right
        -0.5f,  0.5f,  0.5f   // Front-top-left
    };

    GLuint indices[] = {
        0, 1, 2,  0, 2, 3,  // Back face
        4, 5, 6,  4, 6, 7,  // Front face
        0, 1, 5,  0, 5, 4,  // Bottom face
        3, 2, 6,  3, 6, 7,  // Top face
        0, 3, 7,  0, 7, 4,  // Left face
        1, 2, 6,  1, 6, 5   // Right face
    };

    indexCount = sizeof(indices) / sizeof(indices[0]);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderer::setDefaultMatrices() {
    model = glm::mat4(1.0f); // Identity matrix

    view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    projection = glm::perspective(
        glm::radians(45.0f),          // Field of view
        800.0f / 600.0f,              // Aspect ratio
        0.1f,                         // Near plane
        100.0f                        // Far plane
    );
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

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    GLuint vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        std::cerr << "Vertex shader compilation failed for: " << vertexPath << std::endl;
        return 0;
    }

    GLuint fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        std::cerr << "Fragment shader compilation failed for: " << fragmentPath << std::endl;
        return 0;
    }

    // Link shaders into a program
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

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

