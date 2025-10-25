#include "renderer.h"
#include <iostream>
#include <GL/glew.h>
// If using Glad: #include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <string>
#include <fstream>
#include <sstream>

Renderer::Renderer() {
    // Initialize members if needed
    shaderProgram = 0;
    backgroundShaderProgram = 0;
    cubeVao = cubeVbo = cubeEbo = 0;
    backgroundVao = backgroundVbo = 0;
    cubeIndexCount = 0;
    modelLoc = viewLoc = projLoc = -1;
    lightDirLoc = ambientColorLoc = lightColorLoc = -1;
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
    glDepthFunc(GL_LESS);  // Pass if incoming Z < stored Z (closer to camera)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);   // Cull back-facing triangles
    glFrontFace(GL_CCW);   // Front faces have counter-clockwise winding

    // Compile and link shaders
    shaderProgram = createShaderProgram("shaders/vertex3D.glsl", "shaders/fragment3D.glsl");
    if (!shaderProgram) {
        std::cerr << "Failed to create shader program" << std::endl;
        return false;
    }

    backgroundShaderProgram = createShaderProgram("shaders/background_vertex.glsl",
                                                  "shaders/background_fragment.glsl");
    if (!backgroundShaderProgram) {
        std::cerr << "Failed to create background shader program" << std::endl;
        return false;
    }

    // Get uniform locations
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projLoc = glGetUniformLocation(shaderProgram, "projection");
    lightDirLoc = glGetUniformLocation(shaderProgram, "lightDirection");
    ambientColorLoc = glGetUniformLocation(shaderProgram, "ambientColor");
    lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");

    cameraPosLoc = glGetUniformLocation(shaderProgram, "cameraPosition");

    if (modelLoc == -1 || viewLoc == -1 || projLoc == -1 ||
        lightDirLoc == -1 || ambientColorLoc == -1 || lightColorLoc == -1 ||
        cameraPosLoc == -1) {
        std::cerr << "Error: Uniform location not found. Check shader code." << std::endl;
        return false;
    }

    // Set up geometry
    setupCubeGeometry3D();
    setupBackgroundQuad();

    setDefaultMatrices();

    return true;
}

void Renderer::renderFrame() {
    Transform transform;
    transform.model = model;
    transform.view = view;
    transform.projection = projection;
    renderFrame3D(transform);
}

void Renderer::renderFrame2D(const Transform& transform) {
    renderFrame3D(transform);
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
    if (cubeVao) {
        glDeleteVertexArrays(1, &cubeVao);
        cubeVao = 0;
    }
    if (cubeVbo) {
        glDeleteBuffers(1, &cubeVbo);
        cubeVbo = 0;
    }
    if (cubeEbo) {
        glDeleteBuffers(1, &cubeEbo);
        cubeEbo = 0;
    }

    if (backgroundVao) {
        glDeleteVertexArrays(1, &backgroundVao);
        backgroundVao = 0;
    }
    if (backgroundVbo) {
        glDeleteBuffers(1, &backgroundVbo);
        backgroundVbo = 0;
    }

    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    if (backgroundShaderProgram) {
        glDeleteProgram(backgroundShaderProgram);
        backgroundShaderProgram = 0;
    }
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

void Renderer::renderFrame3D(const Transform& transform) {
    glClearColor(0.06f, 0.08f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glUseProgram(backgroundShaderProgram);
    glBindVertexArray(backgroundVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform.model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(transform.view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(transform.projection));

    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirection));
    glUniform3fv(ambientColorLoc, 1, glm::value_ptr(ambientColor));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(directionalColor));
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(transform.camera_position));

    if (cubeIndexCount > 0) {
        glBindVertexArray(cubeVao);
        glDrawElements(GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::setupCubeGeometry3D() {
    float vertices[] = {
        // Position            // Normal            // Color
        // Front face
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.86f, 0.27f, 0.41f,
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.86f, 0.27f, 0.41f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.98f, 0.44f, 0.62f,
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.98f, 0.44f, 0.62f,
        // Back face
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.39f, 0.64f, 0.93f,
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.39f, 0.64f, 0.93f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.53f, 0.78f, 0.97f,
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.53f, 0.78f, 0.97f,
        // Left face
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.33f, 0.86f, 0.69f,
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.33f, 0.86f, 0.69f,
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.47f, 0.94f, 0.77f,
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.47f, 0.94f, 0.77f,
        // Right face
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.97f, 0.74f, 0.33f,
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.97f, 0.74f, 0.33f,
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.99f, 0.83f, 0.47f,
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.99f, 0.83f, 0.47f,
        // Top face
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   0.43f, 0.77f, 0.97f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   0.43f, 0.77f, 0.97f,
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   0.54f, 0.84f, 0.98f,
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   0.54f, 0.84f, 0.98f,
        // Bottom face
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   0.81f, 0.46f, 0.97f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   0.81f, 0.46f, 0.97f,
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.65f, 0.36f, 0.95f,
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.65f, 0.36f, 0.95f,
    };

    unsigned int indices[] = {
        // Front face (CCW from outside: +Z direction)
        0, 1, 2, 2, 3, 0,
        // Back face (CCW from outside: -Z direction, reverse winding)
        5, 4, 7, 7, 6, 5,
        // Left face (CCW from outside: -X direction)
        8, 9, 10, 10, 11, 8,
        // Right face (CCW from outside: +X direction, reverse winding)
        13, 12, 15, 15, 14, 13,
        // Top face (CCW from outside: +Y direction)
        16, 17, 18, 18, 19, 16,
        // Bottom face (CCW from outside: -Y direction, reverse winding)
        21, 20, 23, 23, 22, 21
    };

    glGenVertexArrays(1, &cubeVao);
    glGenBuffers(1, &cubeVbo);
    glGenBuffers(1, &cubeEbo);

    glBindVertexArray(cubeVao);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    cubeIndexCount = sizeof(indices) / sizeof(unsigned int);

    glBindVertexArray(0);
}

void Renderer::setupBackgroundQuad() {
    float vertices[] = {
        // Positions   // UV
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
    };

    glGenVertexArrays(1, &backgroundVao);
    glGenBuffers(1, &backgroundVbo);
    glBindVertexArray(backgroundVao);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
