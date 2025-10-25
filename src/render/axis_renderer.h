/**
 * @file axis_renderer.h
 * @brief Coordinate axis gizmo overlay renderer
 */

#ifndef AXIS_RENDERER_H
#define AXIS_RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>  // GLEW must come first
#include <string>
#include "core/transform.h"

/**
 * @class AxisRenderer
 * @brief Renders RGB coordinate axes as a visual reference overlay
 *
 * This renderer draws a 3-axis gizmo to indicate world coordinate frame:
 * - **Red**: +X axis
 * - **Green**: +Y axis
 * - **Blue**: +Z axis
 *
 * The axes are rendered as colored line segments originating from the origin.
 * Useful for debugging orientation and as a visual reference for camera position.
 *
 * **Implementation**:
 * - Uses separate shader program from main scene renderer
 * - Geometry stored in VAO/VBO as line list with per-vertex colors
 * - Renders on top of scene geometry (can be depth-tested or overlaid)
 */
class AxisRenderer {
public:
    // === Public Members (for external access) ===
    GLuint vao, vbo;                    ///< Vertex array and buffer objects
    GLuint shaderProgram;               ///< Shader program for axis rendering
    glm::mat4 view, projection;         ///< View and projection matrices
    GLuint modelLoc, viewLoc, projLoc;  ///< Shader uniform locations
    Transform axisTransform;            ///< Transformation utilities

    AxisRenderer();
    ~AxisRenderer();

    /**
     * @brief Initialize axis renderer (load shaders, setup geometry)
     * @return true if initialization succeeded, false on error
     */
    bool init();

    /**
     * @brief Render axes in 2D mode (legacy - deprecated)
     * @param transform Transformation utilities
     */
    void render(const Transform& transform);

    /**
     * @brief Render axes in 3D mode
     * @param transform Transformation utilities (view/projection matrices)
     */
    void render3D(const Transform& transform);

    /**
     * @brief Release OpenGL resources
     */
    void shutdown();

private:
    /**
     * @brief Setup 2D axis geometry (legacy)
     */
    void setupAxisGeometry();

    /**
     * @brief Setup 3D axis geometry with per-vertex colors
     *
     * Creates 6 vertices (2 per axis) with RGB colors.
     */
    void setupAxisGeometry3D();

    /**
     * @brief Load and compile axis shader program
     * @param vertexPath Path to vertex shader source file
     * @param fragmentPath Path to fragment shader source file
     * @return Compiled shader program ID, or 0 on failure
     */
    GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);
};

#endif // AXIS_RENDERER_H
