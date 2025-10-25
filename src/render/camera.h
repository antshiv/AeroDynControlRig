/**
 * @file camera.h
 * @brief First-person and orbital camera for 3D scene navigation
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @enum Camera_Movement
 * @brief Camera movement directions for keyboard input
 */
enum Camera_Movement {
    FORWARD,    ///< Move in the direction camera is facing
    BACKWARD,   ///< Move away from direction camera is facing
    LEFT,       ///< Strafe left
    RIGHT,      ///< Strafe right
    UP,         ///< Move upward in world space
    DOWN        ///< Move downward in world space
};

/**
 * @class Camera
 * @brief First-person camera with mouse look and WASD movement
 *
 * Implements a free-flying camera suitable for inspecting 3D scenes.
 * The camera supports:
 * - **Mouse look**: Yaw/pitch control via mouse movement
 * - **WASD movement**: Translation in view-relative coordinates
 * - **Scroll zoom**: Adjusts field of view
 *
 * **Coordinate System**:
 * - Position: Camera location in world space
 * - Front: Normalized look direction
 * - Up: Local up vector (orthogonal to front)
 * - Right: Local right vector (cross product of front and up)
 *
 * The camera generates view and projection matrices for rendering.
 */
class Camera {
public:
    // === Camera State ===
    glm::vec3 position;  ///< Camera position in world space
    glm::vec3 front;     ///< Normalized forward direction vector
    glm::vec3 up;        ///< Normalized up direction vector (local)
    glm::vec3 right;     ///< Normalized right direction vector (local)
    glm::vec3 worldUp;   ///< World space up vector (typically +Y)
    glm::vec3 target;    ///< Look-at target point (for orbital mode)

    // === Orientation (Euler angles) ===
    float yaw;           ///< Horizontal rotation (degrees)
    float pitch;         ///< Vertical rotation (degrees)
    float zoom;          ///< Field of view (degrees), adjusted by scroll

    /**
     * @brief Construct a camera with specified initial state
     * @param startPosition Initial camera position
     * @param startUp World up vector
     * @param startYaw Initial yaw angle (degrees)
     * @param startPitch Initial pitch angle (degrees)
     */
    Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = -90.0f,
           float startPitch = 0.0f);

    /**
     * @brief Get the view matrix for rendering
     * @return 4x4 view matrix (world → camera transform)
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief Get the perspective projection matrix
     * @param aspectRatio Viewport width / height
     * @return 4x4 projection matrix (camera → clip space)
     */
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    /**
     * @brief Process keyboard input (legacy GLFW key codes - deprecated)
     * @param key GLFW key code
     */
    void processKeyboardInput_old(int key);

    /**
     * @brief Process mouse movement for look control
     * @param xOffset Horizontal mouse displacement (pixels)
     * @param yOffset Vertical mouse displacement (pixels)
     * @param constrainPitch Prevent camera from flipping upside-down
     */
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

    /**
     * @brief Process mouse scroll for zoom (FOV adjustment)
     * @param yOffset Scroll wheel offset
     */
    void processMouseScroll(float yOffset);

    /**
     * @brief Process keyboard input for camera movement
     * @param direction Movement direction enum
     * @param deltaTime Time since last frame (seconds) for frame-rate independent movement
     */
    void processKeyboardInput(Camera_Movement direction, float deltaTime);

    /**
     * @brief Reset camera position/orientation/zoom to defaults
     */
    void reset();

    /**
     * @brief Orbit camera around target by adjusting yaw/pitch.
     * @param yawDegrees Change in yaw (degrees)
     * @param pitchDegrees Change in pitch (degrees)
     */
    void orbit(float yawDegrees, float pitchDegrees);

    /**
     * @brief Pan camera laterally/vertically in world space.
     * @param rightDelta Offset along camera right vector (units)
     * @param upDelta Offset along camera up vector (units)
     */
    void pan(float rightDelta, float upDelta);

    /**
     * @brief Move camera forward/backward along its front vector.
     * @param distance Positive to move forward (toward look direction)
     */
    void dolly(float distance);

    /**
     * @brief Adjust field of view-based zoom.
     * @param delta Positive value zooms in (narrows FOV)
     */
    void zoomBy(float delta);

    /**
     * @brief Set field of view directly.
     * @param newZoom Field of view in degrees (clamped to valid range)
     */
    void setZoom(float newZoom);

private:
    float movementSpeed = 2.5f;  ///< Camera movement speed (units/second)

    /**
     * @brief Recalculate front/right/up vectors from yaw/pitch
     *
     * Called after mouse movement to update camera basis vectors.
     */
    void updateCameraVectors();
};

#endif // CAMERA_H
