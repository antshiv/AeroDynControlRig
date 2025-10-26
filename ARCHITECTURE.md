# AeroDynControlRig - Complete Architecture Explained

## Your Mental Map: How Everything Works Together

This document explains the entire rendering pipeline from **keyboard input** to **photons hitting your eyeball**.

Every step is documented with **INPUT → PROCESS → OUTPUT** so you can trace the complete data flow.

---

## Part 1: The Big Picture - 4 Major Systems

```
┌─────────────────┐
│  1. ImGui (UI)  │ ← Panels, text, buttons, plots
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ 2. GLFW (Input) │ ← Keyboard, mouse events
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ 3. OpenGL (GPU) │ ← Draws 3D scene (cube, axes)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ 4. Math Library │ ← Quaternion ↔ Euler ↔ DCM conversions
└─────────────────┘
```

---

## Part 2: ImGui - The UI System

### What is ImGui?

ImGui is an **immediate mode GUI library**. It does NOT transpile to VAO/VBO/EBO directly, but it **uses** VAO/VBO/EBO internally.

### How ImGui Renders

```cpp
// You write this in your code:
ImGui::Text("Roll: %.2f°", roll);
ImGui::SliderFloat("Zoom", &zoom, 0.1f, 10.0f);
```

**Behind the scenes, ImGui does this:**

1. **Generates geometry** (quads for backgrounds, glyphs for text)
2. **Creates a draw list** (list of triangles to render)
3. **Builds VAO/VBO/EBO** internally each frame
4. **Issues draw calls** to OpenGL

### ImGui's Rendering Pipeline

```
Your Code (C++)
    ↓
ImGui::Text("Hello")
    ↓
ImGui builds vertices:
    - Background quad (2 triangles)
    - Text glyphs (quads with texture coordinates)
    ↓
ImGui creates draw commands:
    - glBindTexture(font_atlas)
    - glDrawElements(GL_TRIANGLES, ...)
    ↓
GPU renders text to framebuffer
```

### What ImGui Gives Us

- **Panels** → Colored quads with rounded corners
- **Text** → Textured quads using font atlas
- **Buttons** → Interactive quads with hover/click states
- **Plots (ImPlot)** → Line primitives with axes/labels

**ImGui does NOT understand quaternions or 3D math.** It only draws 2D rectangles and text.

---

## Part 3: GLFW - Input Handling

### What is GLFW?

GLFW is a **window and input library**. It:
- Creates the OS window
- Handles keyboard/mouse input
- Provides an OpenGL context

### How Input Events Work

```cpp
// 1. Register callback functions:
glfwSetKeyCallback(window, Application::keyCallback);
glfwSetCursorPosCallback(window, Application::mouseCallback);

// 2. GLFW calls your callback when events occur:
void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        // User pressed 'W', apply pitch rotation
        pitch += 5.0;
    }
}

// 3. Poll for events each frame:
void Application::tick() {
    glfwPollEvents();  // Processes all pending events
    // ... update simulation ...
    // ... render frame ...
}
```

### Event Flow

```
User presses 'W'
    ↓
OS sends key event
    ↓
GLFW receives event
    ↓
GLFW calls keyCallback()
    ↓
Your code: pitch += 5.0
    ↓
Next frame: quaternion updated
    ↓
Cube rotates on screen
```

**GLFW does NOT handle game logic.** It only provides the events. You write the logic to respond to them.

---

## Part 4: Quaternion → Screen Pixels (The Core Question)

### The Complete Pipeline

```
1. Input        → User presses 'I' (pitch up)
2. Math         → Update quaternion (0.707, 0.707, 0, 0)
3. Conversion   → Quaternion → Rotation Matrix (4x4 DCM)
4. GPU Upload   → Send matrix to shader as uniform
5. Vertex Shader→ Transform cube vertices using matrix
6. Rasterization→ GPU fills in triangles
7. Fragment Shader→ GPU colors pixels
8. Framebuffer  → Pixels written to screen
```

Let me explain each step in detail:

---

### Step 1: Quaternion Representation

**Example quaternion:**
```cpp
q = {w: 0.707, x: 0.707, y: 0, z: 0}
```

This represents: **90° rotation around X-axis**

**Why 0.707?**
- Quaternion = [cos(θ/2), sin(θ/2) * axis]
- For θ = 90°: cos(45°) = 0.707, sin(45°) = 0.707
- Axis = (1, 0, 0) → X-axis

---

### Step 2: Converting Quaternion → Rotation Matrix

**OpenGL does NOT understand quaternions!** We must convert to a **4x4 matrix**.

```cpp
// In application.cpp (happens every frame):

// 1. Get quaternion from simulation state
double q[4] = {
    state.quaternion[0],  // w
    state.quaternion[1],  // x
    state.quaternion[2],  // y
    state.quaternion[3]   // z
};

// 2. Convert to Euler angles (for display only)
double roll, pitch, yaw;
quaternion_to_euler(q, &roll, &pitch, &yaw);

// 3. Convert to DCM (Direction Cosine Matrix = Rotation Matrix)
double dcm[3][3];
quaternion_to_dcm(q, dcm);

// 4. Build 4x4 matrix for OpenGL
glm::mat4 modelMatrix(1.0f);  // Identity matrix
for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
        modelMatrix[col][row] = dcm[row][col];
    }
}

// 5. Store in simulation state
state.model_matrix = modelMatrix;
```

**The resulting matrix looks like this:**

```
┌                    ┐
│  1    0    0    0  │  ← Row 0: X-axis direction after rotation
│  0  cos -sin  0  │  ← Row 1: Y-axis direction after rotation
│  0  sin  cos  0  │  ← Row 2: Z-axis direction after rotation
│  0    0    0    1  │  ← Row 3: Translation (0 for now)
└                    ┘
```

---

### Step 3: Applying Rotation to Cube Vertices

**The cube is made of 8 corners (vertices):**

```cpp
// Original cube vertices (in model space):
glm::vec3 vertices[8] = {
    {-1, -1, -1},  // Corner 0: back-bottom-left
    { 1, -1, -1},  // Corner 1: back-bottom-right
    { 1,  1, -1},  // Corner 2: back-top-right
    {-1,  1, -1},  // Corner 3: back-top-left
    {-1, -1,  1},  // Corner 4: front-bottom-left
    { 1, -1,  1},  // Corner 5: front-bottom-right
    { 1,  1,  1},  // Corner 6: front-top-right
    {-1,  1,  1}   // Corner 7: front-top-left
};
```

**Do we apply q * v * q' for each vertex?**

**NO!** We use the matrix instead:

```cpp
// Vertex shader (runs on GPU for each vertex):
gl_Position = projection * view * model * vec4(aPos, 1.0);
                                  ↑
                                  This is the rotation matrix!
```

**Matrix-vector multiplication:**

```
┌                    ┐   ┌  -1  ┐   ┌  rotated_x  ┐
│  1    0    0    0  │   │  -1  │   │  rotated_y  │
│  0  cos -sin  0  │ × │  -1  │ = │  rotated_z  │
│  0  sin  cos  0  │   │   1  │   │      1      │
│  0    0    0    1  │   └     ┘   └             ┘
└                    ┘
```

**This happens for ALL 8 vertices in parallel on the GPU.**

---

### Step 4: Why Matrix Instead of Quaternion?

**Quaternion rotation formula:**
```
v' = q * v * q_conjugate
```

**Problems:**
1. Requires 2 quaternion multiplications per vertex
2. OpenGL shaders don't have quaternion operations built-in
3. Would need to implement quaternion math in GLSL (slow)

**Matrix rotation is better:**
1. Single matrix-vector multiply (4x4 * 4x1)
2. GPUs are **optimized** for matrix operations (hardware acceleration)
3. Works with projection and view matrices (can combine into one MVP matrix)

**Performance comparison:**
- Quaternion: ~30 operations per vertex
- Matrix: ~16 operations per vertex (and GPU does it in 1 cycle)

---

### Step 5: The Full Rendering Pipeline (Detailed)

```
CPU Side (application.cpp):
┌─────────────────────────────────────────┐
│ 1. User presses 'I' key                 │
│    → keyCallback() triggered            │
│    → pitch += 5.0°                      │
└─────────────────┬───────────────────────┘
                  ▼
┌─────────────────────────────────────────┐
│ 2. Convert to quaternion                │
│    euler_to_quaternion(&euler, q_new)   │
│    → q = {0.731, 0.682, 0, 0}           │
└─────────────────┬───────────────────────┘
                  ▼
┌─────────────────────────────────────────┐
│ 3. Convert to rotation matrix           │
│    quaternion_to_dcm(q, dcm)            │
│    → 4x4 matrix stored in state         │
└─────────────────┬───────────────────────┘
                  ▼
┌─────────────────────────────────────────┐
│ 4. Upload to GPU                        │
│    glUniformMatrix4fv(modelLoc, ...)    │
│    → Matrix copied to GPU memory (VRAM) │
└─────────────────┬───────────────────────┘
                  ▼
GPU Side (vertex shader):
┌─────────────────────────────────────────┐
│ 5. Process each vertex (×8 for cube)    │
│    vec4 worldPos = model * vec4(aPos,1) │
│    → Rotates vertex around origin       │
└─────────────────┬───────────────────────┘
                  ▼
┌─────────────────────────────────────────┐
│ 6. Apply camera view transform          │
│    vec4 viewPos = view * worldPos       │
│    → Positions relative to camera       │
└─────────────────┬───────────────────────┘
                  ▼
┌─────────────────────────────────────────┐
│ 7. Apply perspective projection         │
│    gl_Position = projection * viewPos   │
│    → Converts 3D → 2D screen coords     │
└─────────────────┬───────────────────────┘
                  ▼
GPU Rasterizer:
┌─────────────────────────────────────────┐
│ 8. Rasterization (GPU fills triangles)  │
│    - Interpolates vertex positions      │
│    - Determines which pixels to fill    │
│    → Generates fragments (potential px) │
└─────────────────┬───────────────────────┘
                  ▼
GPU Side (fragment shader):
┌─────────────────────────────────────────┐
│ 9. Shading (color each pixel)           │
│    FragColor = vColor                   │
│    → Each pixel gets RGB value          │
└─────────────────┬───────────────────────┘
                  ▼
Framebuffer:
┌─────────────────────────────────────────┐
│ 10. Write to framebuffer                │
│     - Depth test (z-buffer)             │
│     - Blend (if transparent)            │
│     → Final pixel written to VRAM       │
└─────────────────┬───────────────────────┘
                  ▼
┌─────────────────────────────────────────┐
│ 11. Display on monitor                  │
│     - OS copies framebuffer to screen   │
│     - Vsync (60 Hz typically)           │
│     → Photons reach your eyeballs       │
└─────────────────────────────────────────┘
```

---

## Part 5: How OpenGL Draws the Cube

### What is VAO, VBO, EBO?

Think of it like a recipe and ingredients:

- **VBO** (Vertex Buffer Object) = **Ingredients**
  - Raw data: positions, colors, normals
  - Lives in GPU memory (VRAM)

- **VAO** (Vertex Array Object) = **Recipe**
  - Describes how to interpret VBO data
  - "Position is 3 floats, color is 3 floats, etc."

- **EBO** (Element Buffer Object) = **Assembly Instructions**
  - Indices that define which vertices form each triangle
  - Avoids duplicating shared vertices

### Cube Structure

```cpp
// Each face of the cube is 2 triangles (12 total for 6 faces)

// Example: Front face
float vertices[] = {
    // Position (x,y,z)    Normal (nx,ny,nz)  Color (r,g,b)
    -1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,  0.8f, 0.8f, 0.8f,  // Bottom-left
     1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,  0.8f, 0.8f, 0.8f,  // Bottom-right
     1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,  0.8f, 0.8f, 0.8f,  // Top-right
    -1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,  0.8f, 0.8f, 0.8f,  // Top-left
};

// Indices (define 2 triangles for the face)
unsigned int indices[] = {
    0, 1, 2,  // Triangle 1: bottom-left, bottom-right, top-right
    0, 2, 3   // Triangle 2: bottom-left, top-right, top-left
};
```

### Creating the Cube on GPU

```cpp
// 1. Generate buffer IDs
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glGenBuffers(1, &EBO);

// 2. Bind VAO (start recording recipe)
glBindVertexArray(VAO);

// 3. Upload vertex data to GPU
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 4. Upload index data to GPU
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

// 5. Tell GPU how to read vertex data
// Attribute 0 = Position (3 floats, stride=9 floats, offset=0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// Attribute 1 = Normal (3 floats, stride=9 floats, offset=3)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// Attribute 2 = Color (3 floats, stride=9 floats, offset=6)
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
glEnableVertexAttribArray(2);

// 6. Unbind (finish recording recipe)
glBindVertexArray(0);
```

### Drawing the Cube

```cpp
// Every frame:
glUseProgram(shaderProgram);                    // Activate shader
glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);  // Upload rotation
glBindVertexArray(VAO);                         // Use the recipe
glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);  // Draw 12 triangles (36 vertices)
glBindVertexArray(0);                           // Done
```

---

## Part 6: Shaders - The GPU Programs

### What is a Shader?

A **shader** is a program that runs on the GPU. Written in **GLSL** (OpenGL Shading Language).

### Vertex Shader (vertex3D.glsl)

Runs **once per vertex** (8 times for cube corners).

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;      // Input: vertex position
layout (location = 1) in vec3 aNormal;   // Input: normal vector
layout (location = 2) in vec3 aColor;    // Input: color

out vec3 vColor;     // Output: pass color to fragment shader
out vec3 vNormal;    // Output: pass normal to fragment shader
out vec3 vWorldPos;  // Output: world position

uniform mat4 model;       // Rotation matrix (from quaternion)
uniform mat4 view;        // Camera transform
uniform mat4 projection;  // Perspective projection

void main() {
    // 1. Transform vertex to world space (apply rotation)
    vec4 worldPos = model * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;

    // 2. Transform normal (for lighting later)
    vNormal = normalize(mat3(model) * aNormal);

    // 3. Pass color through
    vColor = aColor;

    // 4. Final position (world → camera → screen)
    gl_Position = projection * view * worldPos;
}
```

**What happens:**
- Input: `aPos = (-1, -1, -1)` (cube corner)
- Multiply by rotation matrix: **rotates the point**
- Multiply by view matrix: **positions relative to camera**
- Multiply by projection matrix: **converts to screen coordinates**
- Output: `gl_Position = (screenX, screenY, depth, w)`

### Fragment Shader (fragment3D.glsl)

Runs **once per pixel** (millions of times per frame).

```glsl
#version 330 core
in vec3 vColor;     // Interpolated color from vertex shader
in vec3 vNormal;    // Interpolated normal
in vec3 vWorldPos;  // Interpolated position

out vec4 FragColor; // Output: final pixel color

void main() {
    // Simple Blinn-Phong lighting
    vec3 lightPos = vec3(5.0, 5.0, 5.0);
    vec3 lightDir = normalize(lightPos - vWorldPos);

    float diffuse = max(dot(vNormal, lightDir), 0.0);
    vec3 ambient = vec3(0.3);

    vec3 result = (ambient + diffuse) * vColor;
    FragColor = vec4(result, 1.0);
}
```

**What happens:**
- GPU interpolates color/normal across triangle
- Calculates lighting based on surface orientation
- Outputs final RGB color for this pixel

---

## Part 7: How Rotation Works (The Math)

### From Keyboard to Rotation

```
1. User presses 'I'
   ↓
2. pitch += 5.0° (in degrees)
   ↓
3. Convert Euler angles to quaternion:
   euler_to_quaternion({roll, pitch, yaw}, q)
   ↓
4. Quaternion now represents the rotation
   ↓
5. Convert quaternion to 3×3 rotation matrix:
   quaternion_to_dcm(q, dcm)
   ↓
6. Expand to 4×4 matrix (add identity for translation)
   ↓
7. Upload to GPU
   ↓
8. GPU multiplies every vertex by this matrix
   ↓
9. Cube appears rotated on screen
```

### Why Quaternions?

**Euler angles have problems:**
- **Gimbal lock** (lose degree of freedom at 90°)
- **Order-dependent** (XYZ ≠ ZYX)
- **Interpolation is wrong** (shortest path isn't straight line)

**Quaternions solve this:**
- **No gimbal lock**
- **Order-independent** (rotation is rotation)
- **Smooth interpolation** (SLERP)
- **Compact** (4 numbers vs 9 for matrix)

**But for rendering, we use matrices because:**
- GPUs are optimized for matrix math
- Can combine multiple transforms (model * view * projection)
- Standard in graphics pipelines

---

## Part 8: Memory and Data Flow

### Where Everything Lives

```
┌─────────────────────────────────────────────┐
│               RAM (System Memory)            │
├─────────────────────────────────────────────┤
│ SimulationState (C++ struct)                │
│  - quaternion: {w, x, y, z}                 │
│  - euler: {roll, pitch, yaw}                │
│  - model_matrix: glm::mat4                  │
│  - angular_rate_deg_per_sec                 │
│                                              │
│ ImGui State                                  │
│  - Window layouts                            │
│  - UI element positions                      │
│  - Draw lists (rebuilt each frame)          │
└──────────────────┬──────────────────────────┘
                   │
          glUniformMatrix4fv()
          glBufferData()
                   │
                   ▼
┌─────────────────────────────────────────────┐
│              VRAM (GPU Memory)               │
├─────────────────────────────────────────────┤
│ Vertex Buffers (VBO)                        │
│  - Cube vertices, colors, normals           │
│  - Axis vertices                             │
│                                              │
│ Shader Uniforms                              │
│  - model matrix (updated every frame)       │
│  - view matrix                               │
│  - projection matrix                         │
│                                              │
│ Textures                                     │
│  - Font atlas (for ImGui text)              │
│  - Icons                                     │
│                                              │
│ Framebuffer                                  │
│  - Color buffer (RGBA pixels)               │
│  - Depth buffer (z-values)                  │
└─────────────────────────────────────────────┘
```

### Data Flow Each Frame

```
1. CPU: glfwPollEvents()
   → Process keyboard/mouse input
   → Update simulation state

2. CPU: Update quaternion
   → Convert to rotation matrix
   → Store in SimulationState

3. CPU: ImGui::NewFrame()
   → Clear draw lists
   → Process UI code
   → Build draw commands

4. CPU: glClear(COLOR | DEPTH)
   → Clear framebuffer

5. CPU → GPU: Upload rotation matrix
   → glUniformMatrix4fv(model, ...)

6. GPU: Vertex shader runs
   → Transform all vertices
   → Output screen positions

7. GPU: Rasterizer fills triangles
   → Determines which pixels to draw

8. GPU: Fragment shader runs
   → Color each pixel

9. CPU: ImGui::Render()
   → Upload UI geometry to GPU
   → GPU draws panels/text on top

10. CPU: glfwSwapBuffers()
    → Display framebuffer on screen
```

---

## Part 9: Putting It All Together

### The Complete System

```
User Input (GLFW)
    ↓
┌───────────────────────────────┐
│   Application Logic (C++)     │
│                                │
│  ┌──────────────────────────┐ │
│  │  Modules (update loop)   │ │
│  │  - QuadcopterDynamics    │ │
│  │  - SensorSimulator       │ │
│  │  - Complementary Filter  │ │
│  │  - RotorTelemetry        │ │
│  └──────────────────────────┘ │
│             ↓                  │
│  ┌──────────────────────────┐ │
│  │   SimulationState        │ │
│  │   - quaternion           │ │
│  │   - euler angles         │ │
│  │   - angular rates        │ │
│  │   - rotor data           │ │
│  └──────────────────────────┘ │
│             ↓                  │
│  ┌──────────────────────────┐ │
│  │  Math Conversions        │ │
│  │  quaternion → DCM → mat4 │ │
│  └──────────────────────────┘ │
└───────────────┬───────────────┘
                │
        ┌───────┴───────┐
        ▼               ▼
┌──────────────┐  ┌────────────┐
│   ImGui UI   │  │  OpenGL 3D │
│   (2D)       │  │  (3D)      │
├──────────────┤  ├────────────┤
│ - Panels     │  │ - Cube     │
│ - Plots      │  │ - Axes     │
│ - Text       │  │ - Lighting │
│ - Buttons    │  │ - Camera   │
└──────┬───────┘  └─────┬──────┘
       │                │
       └────────┬───────┘
                ▼
         ┌─────────────┐
         │     GPU     │
         │ (Rendering) │
         └──────┬──────┘
                ▼
         ┌─────────────┐
         │   Screen    │
         └─────────────┘
```

---

## Part 10: Common Questions Answered

### Q: Do we apply q*v*q' to all points?

**A:** No! We convert quaternion to matrix once, then GPU multiplies matrix × vertex (faster).

### Q: Do we apply quaternion to each side of the cube?

**A:** No! The cube is one object. We apply the matrix to all vertices, and OpenGL draws the triangles connecting them. The faces stay connected automatically.

### Q: How do sides stay attached?

**A:** They share vertices! When vertex (1,1,1) rotates, all 3 faces using that corner move together.

### Q: Does OpenGL understand quaternions?

**A:** No! OpenGL only understands matrices. We must convert quaternion → matrix.

### Q: Does OpenGL understand Euler angles?

**A:** No! We must convert Euler → quaternion → matrix (or Euler → matrix directly).

### Q: How does it look like a whole object?

**A:** Triangles share vertices (via EBO indices). When vertices move, triangles stay connected. The GPU also interpolates colors/normals across faces (smooth shading).

### Q: Everything is just vertices/shaders/pixels?

**A:** **Yes!** Graphics is fundamentally:
1. Vertices (positions)
2. Triangles (connect vertices)
3. Shaders (programs to transform/color them)
4. Pixels (final output)

Everything else (quaternions, matrices, lighting) is just math to compute the vertex positions and colors.

---

## Recommended Reading Order

1. Read this document top to bottom
2. Open `src/app/application.cpp` and trace `tick()` function
3. Look at `src/render/renderer.cpp` to see OpenGL calls
4. Check `shaders/vertex3D.glsl` to see matrix multiplication
5. Examine `src/modules/quadcopter_dynamics.cpp` to see physics

## Visual Debugging Tips

Add these prints to understand data flow:

```cpp
// In application.cpp, after quaternion update:
std::cout << "Quaternion: " << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << std::endl;
std::cout << "Euler: roll=" << roll << " pitch=" << pitch << " yaw=" << yaw << std::endl;

// In vertex shader:
// (can't print from GPU, but can output as color for debugging)
vColor = vec3(gl_Position.xyz * 0.5 + 0.5);  // Visualize position as color
```

---

**You now have a complete mental map of the system!**
