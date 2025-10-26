# Complete Rendering Pipeline: Keyboard → HDMI → Photons

## The Journey of a Single Keypress

This document traces **every single step** from pressing 'I' on your keyboard to seeing the cube rotate on your monitor.

---

## STAGE 1: Input Capture (OS → GLFW → Your Code)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  User presses 'I' key on keyboard                │
│ PROCESS: OS keyboard driver → GLFW event queue          │
│ OUTPUT:  keyCallback(GLFW_KEY_I, GLFW_PRESS) called     │
└──────────────────────────────────────────────────────────┘
```

**Physical Layer:**
1. Keyboard switch closes (electrical contact)
2. Keyboard controller sends USB HID report
3. OS USB driver receives report
4. OS translates to key event
5. OS sends event to application (GLFW)

**Code (application.cpp:461-489):**
```cpp
INPUT:  GLFW_KEY_I pressed
PROCESS:
    if (manual_rotation_mode) {
        pitch += rotation_deg;  // 5° or 1° depending on Shift
        euler_to_quaternion({roll, pitch, yaw}, q_new);
    }
OUTPUT: Updated quaternion in SimulationState
```

---

## STAGE 2: Math Transformations (CPU)

### Step 2A: Euler → Quaternion

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  Euler angles {roll: 0°, pitch: 45°, yaw: 0°}    │
│ PROCESS: euler_to_quaternion() from attitude library    │
│ OUTPUT:  Quaternion {w: 0.924, x: 0.383, y: 0, z: 0}    │
└──────────────────────────────────────────────────────────┘
```

**Math Details:**
```
pitch = 45° = 0.785 radians
half_angle = 0.785 / 2 = 0.393 radians

w = cos(0.393) = 0.924
x = sin(0.393) * 1.0 = 0.383  (pitch is around X-axis)
y = 0
z = 0
```

**Memory Location:** RAM (CPU), in `simulationState.quaternion[4]`

---

### Step 2B: Quaternion → DCM (3×3 Rotation Matrix)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  Quaternion {w: 0.924, x: 0.383, y: 0, z: 0}     │
│ PROCESS: quaternion_to_dcm() from attitude library      │
│ OUTPUT:  DCM (3×3 matrix)                                │
│          ┌  1.000   0.000   0.000 ┐                      │
│          │  0.000   0.707  -0.707 │                      │
│          │  0.000   0.707   0.707 ┘                      │
└──────────────────────────────────────────────────────────┘
```

**What DCM Represents:**
- Row 0: Where X-axis points after rotation → (1, 0, 0) unchanged
- Row 1: Where Y-axis points after rotation → (0, 0.707, -0.707)
- Row 2: Where Z-axis points after rotation → (0, 0.707, 0.707)

**Memory Location:** RAM (CPU), temporary variable `double dcm[3][3]`

---

### Step 2C: DCM → OpenGL Matrix (4×4)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  DCM (3×3 doubles)                                │
│ PROCESS: Expand to 4×4 by adding identity for translation│
│ OUTPUT:  glm::mat4 (4×4 floats, column-major)            │
│          ┌  1.000   0.000   0.000   0.000 ┐              │
│          │  0.000   0.707   0.707   0.000 │              │
│          │  0.000  -0.707   0.707   0.000 │              │
│          │  0.000   0.000   0.000   1.000 ┘              │
└──────────────────────────────────────────────────────────┘
```

**Code (application.cpp):**
```cpp
INPUT:  double dcm[3][3]
PROCESS:
    glm::mat4 modelMatrix(1.0f);  // Identity
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            modelMatrix[col][row] = dcm[row][col];
        }
    }
OUTPUT: glm::mat4 stored in simulationState.model_matrix
```

**Memory Location:** RAM (CPU), in `simulationState.model_matrix`

**Note:** Column-major means columns are contiguous in memory:
```
Memory layout: [col0: 1,0,0,0, col1: 0,0.707,-0.707,0, col2: 0,0.707,0.707,0, col3: 0,0,0,1]
```

---

## STAGE 3: Upload to GPU (CPU → GPU Memory)

### Step 3A: Vertex Data (One-Time Upload at Startup)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  Cube vertices (C++ array in RAM)                │
│         float vertices[] = {                             │
│             -1,-1,-1,  0,0,-1,  0.8,0.8,0.8,  // Vertex 0│
│              1,-1,-1,  0,0,-1,  0.8,0.8,0.8,  // Vertex 1│
│             ...                                          │
│         };                                               │
│ PROCESS: glBufferData(GL_ARRAY_BUFFER, ...)             │
│ OUTPUT:  VBO in GPU VRAM (persistent storage)           │
└──────────────────────────────────────────────────────────┘
```

**Code (renderer.cpp initialization):**
```cpp
INPUT:  vertices[] array (288 floats = 32 vertices × 9 floats each)
PROCESS:
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
OUTPUT: VBO handle (GLuint), data in GPU VRAM
```

**Memory Transfer:**
- DMA (Direct Memory Access) copies RAM → GPU VRAM
- PCIe bus (typically 16 GB/s bandwidth)
- Takes ~microseconds for small data

**GPU VRAM Layout:**
```
VBO Address 0x1000: [-1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 0.8, 0.8, 0.8, ...]
                     └─position──┘ └─normal──┘ └─color─┘
```

---

### Step 3B: Transformation Matrix (Every Frame)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  glm::mat4 model_matrix (RAM)                    │
│ PROCESS: glUniformMatrix4fv()                           │
│ OUTPUT:  Shader uniform in GPU VRAM                     │
└──────────────────────────────────────────────────────────┘
```

**Code (renderer.cpp every frame):**
```cpp
INPUT:  transform.model (glm::mat4, 64 bytes in RAM)
PROCESS:
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &transform.model[0][0]);
OUTPUT: Matrix stored in GPU constant memory
```

**GPU Memory:**
- Uniform buffer object (fast read-only memory)
- Accessible by all shader cores simultaneously
- Updated every frame (60 Hz)

---

## STAGE 4: Vertex Processing (GPU Shader Cores)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  vec3 aPos = (-1, -1, -1)  // Cube corner from VBO│
│         mat4 model                 // Rotation matrix    │
│         mat4 view                  // Camera transform   │
│         mat4 projection            // Perspective        │
│ PROCESS: Vertex shader runs on GPU (shaders/vertex3D.glsl)│
│ OUTPUT:  gl_Position = screen coordinates               │
│          vColor = color for fragment shader              │
└──────────────────────────────────────────────────────────┘
```

**Vertex Shader Code (vertex3D.glsl):**
```glsl
INPUT:
    layout (location = 0) in vec3 aPos;     // (-1, -1, -1)
    layout (location = 2) in vec3 aColor;   // (0.8, 0.8, 0.8)
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

PROCESS:
    void main() {
        // Step 1: Apply rotation (model matrix)
        vec4 worldPos = model * vec4(aPos, 1.0);
        // worldPos = [1 0 0 0] [-1]   = [-1]
        //            [0 .7 .7 0] [-1]   = [-1.4]
        //            [0 -.7 .7 0] [-1]   = [0]
        //            [0 0 0 1] [1]      = [1]

        // Step 2: Apply camera (view matrix)
        vec4 viewPos = view * worldPos;

        // Step 3: Apply perspective (projection matrix)
        gl_Position = projection * viewPos;

        // Step 4: Pass color to fragment shader
        vColor = aColor;
    }

OUTPUT:
    gl_Position = (0.324, -0.512, 0.867, 1.0)  // Screen coords
    vColor = (0.8, 0.8, 0.8)
```

**GPU Hardware:**
- Runs on CUDA cores (NVIDIA) or Stream processors (AMD)
- Processes 8 vertices in parallel (one per cube corner)
- Each core does matrix multiplication (16 multiply-adds)

**Performance:**
- ~100 clock cycles per vertex
- GPU clock: 1.5 GHz
- Can process millions of vertices per second

---

## STAGE 5: Primitive Assembly & Rasterization (GPU Fixed-Function)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  3 vertices (triangle)                            │
│         Vertex A: screen pos (100, 200), color (0.8, 0.8, 0.8)│
│         Vertex B: screen pos (150, 250), color (0.8, 0.8, 0.8)│
│         Vertex C: screen pos (120, 280), color (0.8, 0.8, 0.8)│
│ PROCESS: Rasterizer determines which pixels are inside   │
│ OUTPUT:  Fragments (one per pixel in triangle)           │
└──────────────────────────────────────────────────────────┘
```

**Rasterization Process:**

1. **Clip Space → Screen Space**
```
INPUT:  gl_Position = (0.324, -0.512, 0.867, 1.0)  // Normalized coords
PROCESS:
    screenX = (0.324 + 1.0) / 2.0 * 1920 = 1272 pixels
    screenY = (-0.512 + 1.0) / 2.0 * 1080 = 263 pixels
OUTPUT: Pixel coordinates (1272, 263)
```

2. **Triangle Fill**
```
INPUT:  Triangle vertices at pixels (100,200), (150,250), (120,280)
PROCESS:
    For each pixel (x, y) in bounding box:
        if (insideTriangle(x, y, v0, v1, v2)):
            generate fragment at (x, y)
OUTPUT: ~1000 fragments for this triangle
```

3. **Attribute Interpolation**
```
INPUT:  Vertex colors: A=(0.8,0.8,0.8), B=(0.8,0.8,0.8), C=(0.8,0.8,0.8)
        Pixel at (125, 230) is 40% from A, 30% from B, 30% from C
PROCESS:
    color = 0.4*A + 0.3*B + 0.3*C = (0.8, 0.8, 0.8)
OUTPUT: Fragment color (0.8, 0.8, 0.8)
```

**GPU Hardware:**
- Fixed-function pipeline (not programmable)
- Dedicated rasterizer units
- Processes triangles in parallel

---

## STAGE 6: Fragment Processing (GPU Shader Cores)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  Fragment at pixel (1272, 263)                   │
│         vColor = (0.8, 0.8, 0.8)  // Interpolated        │
│         vNormal = (0, 1, 0)       // Interpolated        │
│         vWorldPos = (0.5, 0.5, 0) // Interpolated        │
│ PROCESS: Fragment shader (shaders/fragment3D.glsl)      │
│ OUTPUT:  FragColor = (0.64, 0.64, 0.64, 1.0)            │
└──────────────────────────────────────────────────────────┘
```

**Fragment Shader Code (fragment3D.glsl):**
```glsl
INPUT:
    in vec3 vColor;     // (0.8, 0.8, 0.8)
    in vec3 vNormal;    // (0, 1, 0)
    in vec3 vWorldPos;  // (0.5, 0.5, 0)

PROCESS:
    void main() {
        // Lighting calculation
        vec3 lightPos = vec3(5.0, 5.0, 5.0);
        vec3 lightDir = normalize(lightPos - vWorldPos);
        // lightDir = normalize((4.5, 4.5, 5.0)) = (0.54, 0.54, 0.60)

        float diffuse = max(dot(vNormal, lightDir), 0.0);
        // diffuse = dot((0,1,0), (0.54,0.54,0.60)) = 0.54

        vec3 ambient = vec3(0.3);
        vec3 result = (ambient + diffuse) * vColor;
        // result = (0.3 + 0.54) * (0.8, 0.8, 0.8) = (0.67, 0.67, 0.67)

        FragColor = vec4(result, 1.0);
    }

OUTPUT:
    FragColor = (0.67, 0.67, 0.67, 1.0)  // RGBA
```

**GPU Hardware:**
- Same shader cores as vertex shader
- Runs millions of times per frame (once per pixel)
- Highly parallel (thousands of fragments processed simultaneously)

---

## STAGE 7: Per-Fragment Operations (GPU Fixed-Function)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  Fragment at (1272, 263)                         │
│         Color: (0.67, 0.67, 0.67, 1.0)                  │
│         Depth: 0.867                                     │
│ PROCESS: Depth test, blending, write to framebuffer     │
│ OUTPUT:  Updated framebuffer pixel                      │
└──────────────────────────────────────────────────────────┘
```

**Depth Test:**
```
INPUT:
    Fragment depth = 0.867
    Depth buffer[1272][263] = 0.925  // Current value

PROCESS:
    if (fragment_depth < depth_buffer_value) {
        // This fragment is closer, replace pixel
        framebuffer[1272][263] = (0.67, 0.67, 0.67, 1.0);
        depth_buffer[1272][263] = 0.867;
    } else {
        // Behind existing pixel, discard
    }

OUTPUT:
    Framebuffer pixel updated
    Depth buffer pixel updated
```

**GPU Memory Structure:**
```
Framebuffer (VRAM):
    Address 0x5000: [R: 171, G: 171, B: 171, A: 255] ← Pixel (1272, 263)
                     (0.67 * 255 = 171)

Depth Buffer (VRAM):
    Address 0x8000: [Z: 0.867] ← Depth at (1272, 263)
```

---

## STAGE 8: Framebuffer → Display (GPU → HDMI)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  Framebuffer (1920×1080 pixels in GPU VRAM)      │
│ PROCESS: Display controller scans framebuffer           │
│ OUTPUT:  HDMI signal with pixel data                    │
└──────────────────────────────────────────────────────────┘
```

**Display Controller:**

1. **Scanout Process (60 Hz)**
```
INPUT:  Framebuffer at address 0x5000 in VRAM

PROCESS:
    for (y = 0; y < 1080; y++) {
        for (x = 0; x < 1920; x++) {
            RGBA pixel = framebuffer[y * 1920 + x];
            send_to_hdmi(pixel.r, pixel.g, pixel.b);
        }
        send_hsync();  // End of horizontal line
    }
    send_vsync();  // End of frame

OUTPUT: HDMI signal stream (60 frames per second)
```

2. **HDMI Encoding**
```
INPUT:  RGB pixel (R: 171, G: 171, B: 171)

PROCESS:
    // Convert to HDMI TMDS encoding
    // Differential signaling for noise immunity
    encode_channel(171, hdmi_red_lane);
    encode_channel(171, hdmi_green_lane);
    encode_channel(171, hdmi_blue_lane);

OUTPUT: 3 differential pairs (red, green, blue) + clock
        Bandwidth: 1920×1080×60×24bpp = 3 Gbps
```

**HDMI Physical Signal:**
- 4 twisted pairs (R, G, B, Clock)
- TMDS encoding (transition-minimized differential signaling)
- Voltage swings: ±250mV differential
- Cable length: up to 15 meters

---

## STAGE 9: Monitor Display (HDMI → Photons)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  HDMI signal (digital RGB + sync)                │
│ PROCESS: Monitor decodes signal and drives LCD panel    │
│ OUTPUT:  Visible light from monitor                     │
└──────────────────────────────────────────────────────────┘
```

**Monitor Processing:**

1. **HDMI Receiver**
```
INPUT:  HDMI TMDS signal

PROCESS:
    decode_tmds(hdmi_signal) → RGB digital values
    recover_clock(hdmi_clock_lane)

OUTPUT: RGB = (171, 171, 171) for pixel (1272, 263)
```

2. **LCD Controller**
```
INPUT:  RGB (171, 171, 171) = 67% intensity

PROCESS:
    // Convert 8-bit value to voltage
    voltage = (171 / 255) * 5V = 3.35V

    // Apply to LCD pixel transistors
    set_pixel_voltage(x=1272, y=263, voltage=3.35V)

OUTPUT: Liquid crystals rotate to 67% transparency
```

3. **Backlight & Color Filters**
```
INPUT:  Backlight (white LED): 400 cd/m²
        Liquid crystal: 67% open
        Color filter: RGB subpixels

PROCESS:
    // White backlight passes through:
    light_through_crystal = 400 * 0.67 = 268 cd/m²

    // Each subpixel filters one color:
    red_light = 268 * 0.67 = 180 cd/m²    (R channel)
    green_light = 268 * 0.67 = 180 cd/m²  (G channel)
    blue_light = 268 * 0.67 = 180 cd/m²   (B channel)

OUTPUT: Gray light at 67% intensity
```

4. **Photon Emission**
```
INPUT:  LED backlight photons

PROCESS:
    1. White LED emits photons (λ = 450-650nm spectrum)
    2. Photons pass through liquid crystal layer
    3. Color filter absorbs unwanted wavelengths
    4. 67% of photons pass through (rest absorbed)

OUTPUT: ~10^15 photons/second emitted from pixel
        Wavelength: balanced RGB spectrum
        Intensity: 180 cd/m²
```

---

## STAGE 10: Human Vision (Photons → Perception)

```
┌──────────────────────────────────────────────────────────┐
│ INPUT:  Photons from monitor (λ = 500-600nm, gray)      │
│ PROCESS: Human eye and brain                            │
│ OUTPUT:  Perception of rotated gray cube                │
└──────────────────────────────────────────────────────────┘
```

**Biological Process:**

1. **Eye Optics**
```
INPUT:  Photons at 180 cd/m² intensity

PROCESS:
    - Cornea refracts light
    - Lens focuses onto retina
    - Photons hit rod/cone cells

OUTPUT: Electrical signals in optic nerve
```

2. **Retinal Response**
```
INPUT:  Photons stimulate cone cells
        - Red cones (L): stimulated at 67%
        - Green cones (M): stimulated at 67%
        - Blue cones (S): stimulated at 67%

PROCESS:
    Photochemical reaction: photon → rhodopsin activation
    Ion channels open → electrical signal

OUTPUT: Neural spike train to brain
```

3. **Visual Cortex**
```
INPUT:  Neural signals from retina

PROCESS:
    - V1 (primary visual cortex): edge detection
    - V2: form and color processing
    - V3: depth perception
    - V5: motion detection

OUTPUT: Conscious perception: "I see a rotated gray cube"
```

---

## Complete Timing Breakdown

**From keypress to perception: ~100 milliseconds total**

| Stage | Time | Location |
|-------|------|----------|
| 1. Keyboard USB report | 1 ms | Keyboard → PC |
| 2. OS event processing | 1 ms | CPU |
| 3. Math calculations (quaternion → matrix) | 0.01 ms | CPU |
| 4. Upload to GPU | 0.05 ms | PCIe bus |
| 5. Vertex shader | 0.1 ms | GPU |
| 6. Rasterization | 0.2 ms | GPU |
| 7. Fragment shader | 0.5 ms | GPU |
| 8. Framebuffer write | 0.1 ms | GPU |
| 9. Display controller scanout | 16.7 ms | GPU → HDMI (60 Hz) |
| 10. HDMI transmission | 0.01 ms | Cable |
| 11. Monitor processing | 5 ms | Monitor LCD |
| 12. Photon travel | 0.000001 ms | Monitor → Eye |
| 13. Neural processing | 50 ms | Eye → Brain |

**Total: ~75 ms (dominated by display refresh and neural processing)**

---

## Memory Map Summary

**CPU RAM:**
- SimulationState struct: ~4 KB
- Vertex arrays (before upload): ~10 KB
- ImGui state: ~1 MB

**GPU VRAM:**
- VBO (vertex buffer): 10 KB
- Framebuffer (1920×1080×4 bytes): 8 MB
- Depth buffer (1920×1080×4 bytes): 8 MB
- Textures (fonts, etc.): 5 MB
- Shader uniforms: 1 KB
- **Total: ~21 MB**

**Bandwidth Usage:**
- CPU → GPU (per frame): ~100 bytes (just the matrix)
- GPU → Monitor (per frame): 8 MB (framebuffer)
- GPU → Monitor (per second): 480 MB (60 fps × 8 MB)

---

## Key Takeaways

1. **4×4 Matrix is NOT for VBO/VAO** - It's a transformation applied to vertices
2. **DCM (3×3) → OpenGL (4×4)** - We expand DCM by adding translation column
3. **ImGui "Transpiles"** - High-level API → OpenGL calls (VAO/VBO/draw commands)
4. **Shaders are Separate** - Not in the matrix, they're GPU programs
5. **Colors are Separate** - Stored in VBO, passed as vertex attributes
6. **Everything is Parallel** - GPU processes millions of pixels simultaneously

**The matrix only transforms GEOMETRY. Everything else (color, texture, lighting) comes from other sources.**
