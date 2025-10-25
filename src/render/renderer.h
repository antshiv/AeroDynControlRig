/**
 * @file renderer.h
 * @brief Main 3D scene renderer using OpenGL 3.3
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "core/transform.h"

/**
 * @class Renderer
 * @brief OpenGL 3.3 renderer for the main 3D scene
 *
 * This class manages rendering of the primary 3D geometry (currently a colored cube)
 * using the modern OpenGL pipeline (VAO/VBO, GLSL shaders).
 *
 * **Rendering Pipeline**:
 * 1. Load and compile vertex/fragment shaders
 * 2. Setup cube geometry in GPU buffers
 * 3. Each frame: bind shaders, set matrices, draw indexed geometry
 *
 * **Coordinate Spaces**:
 * - Model: Local object space → transformed by model matrix
 * - View: Camera/eye space → transformed by view matrix
 * - Projection: Clip space → transformed by projection matrix
 *
 * The model matrix is driven by SimulationState (attitude quaternion),
 * allowing the cube to visualize vehicle orientation.
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * @brief Initialize OpenGL resources (shaders, geometry buffers)
     * @return true if initialization succeeded, false on error
     */
    bool init();

    /**
     * @brief Render a frame (legacy 2D mode - deprecated)
     */
    void renderFrame();

    /**
     * @brief Render a frame with 2D transformation (legacy)
     * @param transform 2D transformation utilities
     */
    void renderFrame2D(const Transform& transform);

    /**
     * @brief Render a 3D frame with the current model/view/projection matrices
     * @param transform 3D transformation utilities
     */
    void renderFrame3D(const Transform& transform);

    /**
     * @brief Set the model matrix (object → world transform)
     * @param modelMat Pointer to 4x4 model matrix (column-major, 16 floats)
     */
    void setModelMatrix(const float* modelMat);

    /**
     * @brief Set the view matrix (world → camera transform)
     * @param view 4x4 view matrix
     */
    void setViewMatrix(const glm::mat4& view);

    /**
     * @brief Set the projection matrix (camera → clip space transform)
     * @param proj 4x4 projection matrix (typically perspective)
     */
    void setProjectionMatrix(const glm::mat4& proj);

    /**
     * @brief Release OpenGL resources (shaders, VAOs, VBOs)
     */
    void shutdown();

private:
    // === OpenGL Resources ===
    unsigned int shaderProgram;              ///< Lighting shader program ID
    unsigned int backgroundShaderProgram;    ///< Background gradient shader ID

    unsigned int cubeVao;                   ///< Cube VAO
    unsigned int cubeVbo;                   ///< Cube VBO
    unsigned int cubeEbo;                   ///< Cube EBO

    unsigned int backgroundVao;             ///< Background quad VAO
    unsigned int backgroundVbo;             ///< Background quad VBO

    unsigned int cubeIndexCount;            ///< Index count for cube

    // === Shader Uniform Locations ===
    int modelLoc;                  ///< Location of model matrix uniform
    int viewLoc;                   ///< Location of view matrix uniform
    int projLoc;                   ///< Location of projection matrix uniform
    int lightDirLoc;               ///< Location of light direction uniform
    int ambientColorLoc;           ///< Location of ambient color uniform
    int lightColorLoc;             ///< Location of directional light color uniform
    int cameraPosLoc;              ///< Location of camera position uniform

    // === Transformation Matrices ===
    glm::mat4 model;               ///< Model matrix (object → world)
    glm::mat4 view;                ///< View matrix (world → camera)
    glm::mat4 projection;          ///< Projection matrix (camera → clip space)

    // === Lighting Parameters ===
    glm::vec3 lightDirection{glm::normalize(glm::vec3(-0.35f, -0.70f, -0.25f))};
    glm::vec3 ambientColor{0.36f, 0.38f, 0.46f};
    glm::vec3 directionalColor{1.12f, 1.08f, 1.02f};

    // === Initialization Helpers ===
    /**
     * @brief Load, compile, and link GLSL shaders
     * @param vertexSrc Vertex shader source code
     * @param fragmentSrc Fragment shader source code
     * @return Shader program ID, or 0 on failure
     */
    unsigned int loadShader(const char* vertexSrc, const char* fragmentSrc);

    /**
     * @brief Setup 3D cube geometry with per-vertex colors
     */
    void setupCubeGeometry3D();

    /**
     * @brief Setup full-screen background quad
     */
    void setupBackgroundQuad();

    /**
     * @brief Initialize matrices to identity/default values
     */
    void setDefaultMatrices();
};

#endif // RENDERER_H
