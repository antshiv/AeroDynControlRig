/**
 * @page opengl_tutorial OpenGL Deep Dive Tutorial
 * @brief Complete guide to OpenGL 3.3 Core Profile with examples from our application
 *
 * @tableofcontents
 */

# OpenGL Deep Dive Tutorial

## What is OpenGL?

**OpenGL (Open Graphics Library)** is a cross-platform API for rendering 2D and 3D graphics. Think of it as a translator between your C++ code and your graphics card (GPU).

### Why Use OpenGL?

- **Hardware Acceleration**: GPU can process millions of vertices in parallel
- **Cross-Platform**: Works on Windows, macOS, Linux
- **Mature**: Battle-tested since 1992
- **Well-Documented**: Tons of resources and tutorials

### OpenGL in Our Application

We use **OpenGL 3.3 Core Profile** - a modern version that:
- Removes deprecated legacy features
- Uses programmable shaders (GLSL)
- Requires explicit vertex specification (VAO/VBO)

---

## The Mental Model: CPU ↔ GPU Pipeline

Before diving into code, understand the **fundamental concept**:

```
┌─────────────────────────────────────────────────────────────┐
│                    YOUR C++ CODE (CPU)                      │
│  1. Prepare vertex data (positions, colors, etc.)          │
│  2. Upload data to GPU memory                              │
│  3. Tell GPU how to interpret the data                     │
│  4. Issue draw command                                     │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼ glDrawElements()
┌─────────────────────────────────────────────────────────────┐
│                  GRAPHICS CARD (GPU)                        │
│  5. Vertex Shader: Transform each vertex                   │
│  6. Rasterizer: Generate fragments (potential pixels)      │
│  7. Fragment Shader: Color each fragment                   │
│  8. Write to framebuffer (screen or texture)               │
└─────────────────────────────────────────────────────────────┘
```

**Key Insight**: You prepare data once, then the GPU processes it in parallel at incredible speed.

---

## Core Concept 1: Vertices

### What is a Vertex?

A **vertex** is a point in space with associated data. In our app, each vertex has:
- **Position** (x, y, z)
- **Color** (r, g, b)

### Example from `renderer.cpp`

```cpp
// Define 8 vertices for a cube
// Each vertex: 6 floats (3 for position + 3 for color)
float vertices[] = {
    // Position          Color
    // x     y     z     r    g    b
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  // Vertex 0: Red
     0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Vertex 1: Green
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Vertex 2: Blue
    -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  // Vertex 3: Yellow
    // ... 4 more vertices for the back face
};
```

**Why floats?** GPUs work with floating-point numbers. Coordinates are typically in range [-1, 1] for normalized device coordinates (NDC).

---

## Core Concept 2: VBO (Vertex Buffer Object)

### What is a VBO?

A **VBO** is a chunk of GPU memory that stores vertex data. Think of it as an array on the graphics card.

### Why Use VBOs?

- **Performance**: Data stays on GPU (no CPU→GPU transfer every frame)
- **Efficiency**: GPU can access data instantly

### Creating a VBO in Our Code

```cpp
// From renderer.cpp setupCubeGeometry3D()

// Step 1: Generate a buffer ID
GLuint vbo;
glGenBuffers(1, &vbo);

// Step 2: Bind it (make it "active")
glBindBuffer(GL_ARRAY_BUFFER, vbo);

// Step 3: Upload vertex data to GPU
glBufferData(
    GL_ARRAY_BUFFER,           // Target: array buffer
    sizeof(vertices),          // Size in bytes
    vertices,                  // Pointer to data
    GL_STATIC_DRAW             // Usage hint: data won't change
);
```

**Think of binding like selecting a tool**: You "bind" (select) a VBO before operating on it.

### Buffer Usage Hints

| Hint | Meaning | Use When |
|------|---------|----------|
| `GL_STATIC_DRAW` | Data set once, used many times | Geometry that doesn't change (our cube) |
| `GL_DYNAMIC_DRAW` | Data changes occasionally | Updated every few frames |
| `GL_STREAM_DRAW` | Data changes every frame | Particle systems, dynamic meshes |

---

## Core Concept 3: VAO (Vertex Array Object)

### What is a VAO?

A **VAO** stores the **format** of your vertex data. It remembers:
- Which VBO to use
- How data is laid out (stride, offset)
- Which attributes are enabled

### Analogy

- **VBO** = A filing cabinet (stores the data)
- **VAO** = An index card (tells you how files are organized)

### Creating a VAO in Our Code

```cpp
// From renderer.cpp setupCubeGeometry3D()

// Step 1: Generate VAO
GLuint vao;
glGenVertexArrays(1, &vao);

// Step 2: Bind it (make it active)
glBindVertexArray(vao);

// Step 3: Tell OpenGL how data is laid out
// (Must be done AFTER binding VBO)

// Attribute 0: Position (3 floats)
glVertexAttribPointer(
    0,                          // Attribute index (matches shader layout location)
    3,                          // Number of components (x, y, z)
    GL_FLOAT,                   // Data type
    GL_FALSE,                   // Normalized? (No)
    6 * sizeof(float),          // Stride: bytes between consecutive vertices
    (void*)0                    // Offset: bytes from start to this attribute
);
glEnableVertexAttribArray(0);

// Attribute 1: Color (3 floats)
glVertexAttribPointer(
    1,                          // Attribute index
    3,                          // Number of components (r, g, b)
    GL_FLOAT,                   // Data type
    GL_FALSE,                   // Normalized? (No)
    6 * sizeof(float),          // Stride: same as above
    (void*)(3 * sizeof(float))  // Offset: skip 3 floats (position)
);
glEnableVertexAttribArray(1);
```

### Understanding Stride and Offset

Our vertex data layout:
```
[x, y, z, r, g, b] [x, y, z, r, g, b] [x, y, z, r, g, b] ...
 \_____/  \_____/
Position  Color
```

- **Stride**: 6 floats (distance from one vertex to the next)
- **Position offset**: 0 (starts at beginning)
- **Color offset**: 3 floats (skip position to get to color)

---

## Core Concept 4: EBO (Element Buffer Object)

### What is an EBO?

An **EBO** (or **IBO** - Index Buffer Object) stores **indices** that reference vertices, avoiding duplication.

### The Problem: Duplicate Vertices

A cube has 6 faces. Each face has 2 triangles. That's 12 triangles × 3 vertices = **36 vertices**.

But a cube only has **8 unique corners**! We're duplicating data.

### The Solution: Indexed Drawing

```cpp
// From renderer.cpp

// Instead of storing 36 vertices, store 8 unique ones
float vertices[8 * 6] = { /* ... */ };

// Then define triangles using indices
unsigned int indices[] = {
    // Front face
    0, 1, 2,    // Triangle 1
    2, 3, 0,    // Triangle 2

    // Back face
    4, 5, 6,    // Triangle 3
    6, 7, 4,    // Triangle 4

    // ... more faces
};
```

### Creating an EBO

```cpp
GLuint ebo;
glGenBuffers(1, &ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(indices),
    indices,
    GL_STATIC_DRAW
);
```

### Drawing with Indices

```cpp
// Instead of:
// glDrawArrays(GL_TRIANGLES, 0, 36);  // Draw 36 vertices

// Use:
glDrawElements(
    GL_TRIANGLES,      // Mode
    36,                // Count (number of indices)
    GL_UNSIGNED_INT,   // Index type
    0                  // Offset in EBO
);
```

**Benefit**: 8 vertices instead of 36! Saves memory and bandwidth.

---

## Core Concept 5: Shaders

### What are Shaders?

**Shaders** are programs that run **on the GPU**. Written in **GLSL** (OpenGL Shading Language), they look like C.

### Two Required Shaders

1. **Vertex Shader**: Runs once per vertex (transforms position)
2. **Fragment Shader**: Runs once per fragment/pixel (determines color)

### Vertex Shader Example (from `shaders/vertex_shader.glsl`)

```glsl
#version 330 core

// Input: per-vertex data (from VAO)
layout (location = 0) in vec3 aPos;    // Position
layout (location = 1) in vec3 aColor;  // Color

// Output: data passed to fragment shader
out vec3 ourColor;

// Uniforms: data shared across all vertices
uniform mat4 model;        // Object → World transform
uniform mat4 view;         // World → Camera transform
uniform mat4 projection;   // Camera → Screen transform

void main() {
    // Transform vertex position through all matrices
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Pass color to fragment shader
    ourColor = aColor;
}
```

**Key Points**:
- `layout (location = X)`: Matches `glVertexAttribPointer` index
- `uniform`: Set from C++ code, same for all vertices
- `gl_Position`: Built-in output (final vertex position)

### Fragment Shader Example (from `shaders/fragment_shader.glsl`)

```glsl
#version 330 core

// Input: from vertex shader (interpolated)
in vec3 ourColor;

// Output: final pixel color
out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0);  // RGB + Alpha
}
```

**Interpolation Magic**: If vertex 0 is red and vertex 1 is green, pixels in between smoothly blend from red to green!

### Compiling Shaders in C++

```cpp
// From renderer.cpp loadShader()

// Step 1: Create shader objects
GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

// Step 2: Attach source code
glShaderSource(vertexShader, 1, &vertexSource, NULL);
glShaderSource(fragmentShader, 1, &fragmentSource, NULL);

// Step 3: Compile
glCompileShader(vertexShader);
glCompileShader(fragmentShader);

// Step 4: Check for errors
GLint success;
glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
}

// Step 5: Link into program
GLuint shaderProgram = glCreateProgram();
glAttachShader(shaderProgram, vertexShader);
glAttachShader(shaderProgram, fragmentShader);
glLinkProgram(shaderProgram);

// Step 6: Clean up (shaders no longer needed after linking)
glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);
```

---

## Core Concept 6: Uniforms

### What are Uniforms?

**Uniforms** are variables in shaders that:
- Are set from C++ code
- Remain constant for an entire draw call
- Can be accessed by all vertices/fragments

### Setting Uniforms in Our Code

```cpp
// From renderer.cpp renderFrame3D()

// Step 1: Get uniform location
GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

// Step 2: Set uniform values (before drawing)
glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));
```

### Uniform Types

| C++ Function | GLSL Type | Use Case |
|--------------|-----------|----------|
| `glUniform1f()` | `float` | Single float value |
| `glUniform3f()` | `vec3` | Position, color (RGB) |
| `glUniform4f()` | `vec4` | Color (RGBA) |
| `glUniformMatrix4fv()` | `mat4` | Transformation matrices |

---

## Core Concept 7: Coordinate Transformations

### The Four Coordinate Spaces

Every vertex goes through **four transformations**:

```
Local Space          World Space         View Space          Clip Space
(Model)              (Scene)             (Camera)            (Screen)

  Cube                 Cube                Cube                Screen
┌────────┐          ┌────────┐          ┌────────┐          ┌────────┐
│   /\   │          │   /\   │          │   /\   │          │   /\   │
│  /  \  │  model   │  /  \  │  view    │  /  \  │  proj    │  /  \  │
│ /____\ │  ────►   │ /____\ │  ────►   │ /____\ │  ────►   │ /____\ │
│        │  matrix  │        │  matrix  │        │  matrix  │  -1→+1 │
└────────┘          └────────┘          └────────┘          └────────┘

    │                    │                    │                    │
    └────────────────────┴────────────────────┴────────────────────┘
                gl_Position = projection * view * model * localPos
```

### 1. Local Space (Model Space)

**What**: Coordinates relative to the object's origin

**Example**: A cube centered at (0, 0, 0) with size 1.0

```cpp
// Vertex positions are in local space
glm::vec3 vertex = glm::vec3(-0.5f, -0.5f, -0.5f);
```

### 2. World Space

**What**: Coordinates in the global scene

**How**: Apply **model matrix**

```cpp
// From application.cpp
// The quaternion demo updates this matrix based on attitude
glm::mat4 modelMatrix = state.model_matrix;

// This matrix might rotate the cube:
// modelMatrix = glm::rotate(mat4(1.0), angle, axis);
```

### 3. View Space (Camera Space)

**What**: Coordinates relative to the camera

**How**: Apply **view matrix**

```cpp
// From camera.cpp
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(
        position,          // Camera position
        position + front,  // Look-at point
        up                 // Up vector
    );
}
```

**Think of it**: The world moves opposite to the camera. If camera moves right, world moves left.

### 4. Clip Space (NDC - Normalized Device Coordinates)

**What**: Final coordinates in range [-1, 1] for x, y, z

**How**: Apply **projection matrix**

```cpp
// From camera.cpp
glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(
        glm::radians(zoom),  // Field of view (FOV)
        aspectRatio,         // Width / Height
        0.1f,                // Near clipping plane
        100.0f               // Far clipping plane
    );
}
```

**Perspective**: Objects farther away appear smaller (realistic)

**Orthographic**: Objects stay same size regardless of distance (CAD, 2D)

---

## Core Concept 8: FBO (Framebuffer Object)

### What is an FBO?

A **Framebuffer** is the destination for rendering. By default, you render to the screen. An **FBO** lets you render to a texture instead.

### Why Use FBOs?

In our application, we:
1. Render 3D scene to an FBO (off-screen texture)
2. Display that texture in an ImGui window
3. Overlay UI on top

This allows ImGui and OpenGL rendering to coexist!

### Creating an FBO in Our Code

```cpp
// From application.cpp ensureRenderTarget()

// Step 1: Create FBO
GLuint fbo;
glGenFramebuffers(1, &fbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

// Step 2: Create a texture (color attachment)
GLuint renderTexture;
glGenTextures(1, &renderTexture);
glBindTexture(GL_TEXTURE_2D, renderTexture);
glTexImage2D(
    GL_TEXTURE_2D,
    0,                 // Mipmap level
    GL_RGB,            // Internal format
    width, height,     // Size
    0,                 // Border (must be 0)
    GL_RGB,            // Format
    GL_UNSIGNED_BYTE,  // Type
    NULL               // No initial data
);

// Step 3: Attach texture to FBO
glFramebufferTexture2D(
    GL_FRAMEBUFFER,
    GL_COLOR_ATTACHMENT0,  // Attachment point
    GL_TEXTURE_2D,
    renderTexture,
    0                      // Mipmap level
);

// Step 4: Create depth buffer (for 3D depth testing)
GLuint depthBuffer;
glGenRenderbuffers(1, &depthBuffer);
glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

// Step 5: Attach depth buffer to FBO
glFramebufferRenderbuffer(
    GL_FRAMEBUFFER,
    GL_DEPTH_STENCIL_ATTACHMENT,
    GL_RENDERBUFFER,
    depthBuffer
);

// Step 6: Check if framebuffer is complete
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Framebuffer is not complete!" << std::endl;
}

// Step 7: Unbind (switch back to default framebuffer)
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```

### Rendering to an FBO

```cpp
// From application.cpp renderSceneToTexture()

// Step 1: Bind FBO (renders go to texture now)
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

// Step 2: Clear it
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Step 3: Render scene
renderer.renderFrame3D(transform);

// Step 4: Unbind (switch back to screen)
glBindFramebuffer(GL_FRAMEBUFFER, 0);

// Step 5: Use texture in ImGui
ImGui::Image((void*)(intptr_t)renderTexture, ImVec2(800, 600));
```

---

## Complete Rendering Loop

Putting it all together, here's what happens every frame in `Application::tick()`:

```cpp
void Application::tick() {
    // 1. Update simulation state
    for (auto& module : modules) {
        module->update(dt, simulationState);
    }

    // 2. Render 3D scene to off-screen texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, sceneWidth, sceneHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3. Set shader uniforms
    renderer.setModelMatrix(glm::value_ptr(simulationState.model_matrix));
    renderer.setViewMatrix(camera.getViewMatrix());
    renderer.setProjectionMatrix(camera.getProjectionMatrix(aspect));

    // 4. Draw geometry
    renderer.renderFrame3D(transform);

    // 5. Unbind FBO (back to screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 6. Render ImGui UI
    ImGui::NewFrame();
    ImGui::Begin("Viewport");
    ImGui::Image((void*)(intptr_t)renderTexture, viewportSize);
    ImGui::End();

    panelManager.drawAll(simulationState, camera);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 7. Swap buffers (show frame)
    glfwSwapBuffers(window);
}
```

---

## Common Pitfalls and Solutions

### Problem: Black Screen

**Causes**:
- Forgot to bind VAO before drawing
- Shader compilation failed (check with `glGetShaderiv`)
- Uniforms not set
- Depth test enabled but no depth buffer

**Solution**:
```cpp
// Always bind VAO before drawing
glBindVertexArray(vao);
glUseProgram(shaderProgram);
// Set uniforms...
glDrawElements(...);
```

### Problem: Vertices in Wrong Position

**Causes**:
- Matrix multiplication order wrong
- Forgot to transpose matrix
- Using radians vs degrees

**Solution**:
```cpp
// Correct order: projection * view * model
glm::mat4 mvp = projection * view * model;

// GLM uses radians
glm::perspective(glm::radians(45.0f), aspect, near, far);
```

### Problem: Memory Leaks

**Causes**:
- Not deleting buffers/textures when done

**Solution**:
```cpp
// In destructor or shutdown()
glDeleteVertexArrays(1, &vao);
glDeleteBuffers(1, &vbo);
glDeleteBuffers(1, &ebo);
glDeleteProgram(shaderProgram);
```

---

## Performance Tips

### 1. Minimize State Changes

**Bad** (slow):
```cpp
for (int i = 0; i < 1000; i++) {
    glBindVertexArray(vao[i]);
    glDrawElements(...);
}
```

**Good** (fast):
```cpp
glBindVertexArray(vao);
for (int i = 0; i < 1000; i++) {
    // Update uniforms if needed
    glDrawElements(...);
}
```

### 2. Use Instancing for Repeated Objects

```cpp
// Draw 1000 cubes in one call
glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, 1000);
```

### 3. Batch Similar Objects

Render all opaque objects first, then all transparent objects.

---

## Debugging Tools

### 1. glGetError()

```cpp
GLenum err = glGetError();
if (err != GL_NO_ERROR) {
    std::cerr << "OpenGL Error: " << err << std::endl;
}
```

### 2. Shader Info Log

```cpp
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "Shader Error:\n" << infoLog << std::endl;
}
```

### 3. RenderDoc

Download **RenderDoc** (free tool) to:
- Capture frames
- Inspect all OpenGL state
- See every draw call
- Debug shaders

---

## Next Steps

1. **Modify the cube**: Change colors in `renderer.cpp`
2. **Add more geometry**: Create a pyramid or sphere
3. **Experiment with shaders**: Add lighting or textures
4. **Learn about**: Textures, lighting, normal mapping

## Resources

- [LearnOpenGL](https://learnopengl.com/) - Best OpenGL tutorial
- [OpenGL Reference](https://www.khronos.org/registry/OpenGL-Refpages/gl4/) - Official docs
- [Shader Toy](https://www.shadertoy.com/) - Fragment shader playground

---

**You now understand the core OpenGL concepts used in AeroDynControlRig!**

@see @ref imgui_tutorial for ImGui guide
@see @ref opengl_imgui_integration for how they work together
