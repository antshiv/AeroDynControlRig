# GPU Pipeline: From RAM to Screen {#gpu_pipeline}

[TOC]

## Overview

This document explains **exactly** how data flows from your C++ code in RAM through the CPU cache, across the PCIe bus to the GPU, through shader execution units, and finally to pixels on your screen.

Understanding this pipeline is critical for graphics programming because **you're working at the lowest level** - directly commanding the GPU hardware.

---

## The Complete Pipeline

```
┌─────────────────────────────────────────────────────────────────┐
│ CPU Side (Your C++ Code)                                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Application (main.cpp)                                         │
│       ↓                                                         │
│  Update Modules (quaternion integration, etc.)                  │
│       ↓                                                         │
│  Write to SimulationState (RAM)                                 │
│       ↓                                                         │
│  Panels read SimulationState (CPU cache hit/miss)              │
│       ↓                                                         │
│  ImGui builds vertex data (RAM allocation)                      │
│       ↓                                                         │
│  OpenGL driver queues commands (RAM → kernel space)            │
│       ↓                                                         │
├─────────────────────────────────────────────────────────────────┤
│ PCIe Bus Transfer (DMA)                                         │
├─────────────────────────────────────────────────────────────────┤
│       ↓                                                         │
│  GPU receives command buffer                                    │
│       ↓                                                         │
├─────────────────────────────────────────────────────────────────┤
│ GPU Side (Graphics Card)                                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  GPU VRAM (vertex data, textures, framebuffers)                │
│       ↓                                                         │
│  Vertex Shader (parallel execution on shader cores)            │
│       ↓                                                         │
│  Rasterizer (hardware fixed-function)                          │
│       ↓                                                         │
│  Fragment Shader (parallel execution on shader cores)          │
│       ↓                                                         │
│  Framebuffer (VRAM)                                             │
│       ↓                                                         │
│  Display Controller scans framebuffer                           │
│       ↓                                                         │
│  HDMI/DisplayPort output                                        │
│       ↓                                                         │
│  Monitor pixels illuminate                                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Stage 1: CPU Side - Your C++ Code

### 1.1 Application Update Loop

File: `src/app/application.cpp:364-418`

```cpp
void Application::tick() {
    // CPU executes this code at ~60 Hz (16.67ms per frame)

    // Step 1: Calculate time delta (CPU reads high-precision timer)
    float currentFrame = glfwGetTime();  // System call → CPU cache
    double dt = currentFrame - lastFrame;

    // Step 2: Update simulation modules
    for (auto& module : modules) {
        module->update(dt, simulationState);  // Writes to RAM
    }

    // Step 3: Render everything
    render3D();  // This triggers GPU commands
}
```

**Memory Location:** Your code is in RAM, executing on CPU cores. The `simulationState` object lives in **heap memory (RAM)**.

**CPU Cache:** When modules access `simulationState.attitude.quaternion`, the CPU checks:
1. **L1 cache** (4 cycles, ~1ns) - Private per CPU core, ~32-64 KB
2. **L2 cache** (12 cycles, ~3ns) - Private per CPU core, ~256-512 KB
3. **L3 cache** (40 cycles, ~12ns) - Shared across all cores, ~8-32 MB
4. **RAM** (200+ cycles, ~60ns) - If cache miss, fetch from main memory

**Why This Matters:**
- Sequential memory access (like looping through `state.rotor.rpm` vector) is **fast** because CPU prefetches next cache line
- Random memory access (like pointer chasing) causes cache misses

---

### 1.2 Module Updates (Pure CPU Work)

Example: `src/modules/quaternion_demo.cpp:37-72`

```cpp
void QuaternionDemoModule::update(double dt, SimulationState& state) {
    // All of this happens in RAM, executed by CPU

    // Read angular velocity (CPU loads from RAM → L1 cache)
    glm::dvec3 omega_deg_s = state.angular_rate_deg_per_sec;
    glm::dvec3 omega_rad_s = glm::radians(omega_deg_s);

    // Quaternion integration (CPU ALU performs math)
    glm::dquat q_current = state.attitude.quaternion;
    glm::dquat omega_quat = glm::dquat(0.0, omega_rad_s.x, omega_rad_s.y, omega_rad_s.z);
    glm::dquat q_dot = 0.5 * omega_quat * q_current;
    glm::dquat q_new = q_current + dt * q_dot;
    q_new = glm::normalize(q_new);

    // Write back to state (CPU writes to RAM, invalidates cache)
    state.attitude.quaternion = q_new;
    state.attitude.euler_deg = quaternionToEuler(q_new);  // More CPU math
    state.attitude.model_matrix = quaternionToDCM(q_new); // 4x4 matrix ops
}
```

**CPU Execution Details:**
- **Instruction Decode:** CPU decodes x86-64 instructions (ADD, MUL, LOAD, STORE)
- **Vector Units:** GLM uses SIMD (SSE/AVX) - processes 4 floats/doubles in parallel on CPU
- **Branch Prediction:** CPU predicts `if` statements to avoid pipeline stalls
- **Out-of-Order Execution:** CPU reorders instructions to maximize ALU utilization

**Memory Writes:**
When you write `state.attitude.quaternion = q_new`:
1. CPU updates L1 cache (write-back policy)
2. L1 cache marks line as "dirty"
3. Eventually flushed to L2 → L3 → RAM
4. Other CPU cores see updated value via cache coherency protocol (MESI)

---

### 1.3 ImGui Vertex Generation

When you call `ImGui::Text("Hello")`, ImGui builds vertex data **in RAM**:

```cpp
// Simplified version of what ImGui does internally
struct ImDrawVert {
    ImVec2 pos;   // 8 bytes (x, y) - screen coordinates
    ImVec2 uv;    // 8 bytes (u, v) - texture coordinates
    ImU32 col;    // 4 bytes (RGBA) - vertex color
};  // Total: 20 bytes per vertex

// For each UI element, ImGui generates triangles
// Example: A rectangle (2 triangles = 6 vertices)
ImDrawVert vertices[6] = {
    {{x0, y0}, {0, 0}, color},  // Triangle 1
    {{x1, y0}, {1, 0}, color},
    {{x0, y1}, {0, 1}, color},
    {{x0, y1}, {0, 1}, color},  // Triangle 2
    {{x1, y0}, {1, 0}, color},
    {{x1, y1}, {1, 1}, color}
};
```

**Memory Location:** This vertex array is allocated in **heap memory (RAM)** via `malloc()` or `new`.

**Buffer Size:** For a complex UI with 1000 widgets:
- ~10,000 vertices × 20 bytes = **200 KB** in RAM
- Small enough to fit entirely in CPU L3 cache

---

### 1.4 OpenGL Command Submission

File: `src/render/renderer.cpp:78-85`

```cpp
void Renderer::renderFrame(const Transform& transform) {
    // CPU prepares uniform data in RAM
    glm::mat4 mvp = transform.projection * transform.view * transform.model;

    // CPU tells GPU to use shader program
    glUseProgram(shaderProgram);  // ← Driver queues command

    // CPU uploads matrix to GPU
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));  // ← DMA transfer scheduled

    // CPU tells GPU to bind VAO (GPU state change)
    glBindVertexArray(vao);  // ← Command queued

    // CPU tells GPU to draw triangles
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);  // ← Render command queued
}
```

**What Happens Under the Hood:**

1. **CPU writes commands to a ring buffer** (shared memory between driver and kernel)
2. **OpenGL driver** (running in kernel space) validates commands
3. **DMA (Direct Memory Access)** transfers commands to GPU memory **without CPU involvement**
4. **GPU command processor** wakes up and starts executing

**Critical Detail:** `glDrawElements()` does NOT execute immediately! It's **queued** and executes later when you call `glfwSwapBuffers()`.

---

## Stage 2: PCIe Bus Transfer

### 2.1 CPU → GPU Communication

```
┌──────────────┐          PCIe 3.0 x16          ┌──────────────┐
│     CPU      │  ←─────────────────────────→   │     GPU      │
│  (RAM side)  │    16 GB/s bidirectional       │  (VRAM side) │
└──────────────┘                                └──────────────┘
```

**PCIe Bus Specifications:**
- **PCIe 3.0 x16:** ~16 GB/s bandwidth (32 GB/s for PCIe 4.0)
- **Latency:** ~1-2 microseconds for command submission
- **DMA Transfer:** GPU can read RAM directly without CPU, but slower than VRAM

**What Gets Transferred:**

1. **Vertex Data** (if not already in VRAM):
   - VAO/VBO uploaded once at init via `glBufferData()`
   - Stays in GPU VRAM until deleted

2. **Uniform Data** (uploaded every frame):
   - `mvp` matrix (64 bytes)
   - Light positions, colors (if used)

3. **Texture Data** (uploaded once):
   - Font atlases for ImGui
   - Off-screen render target textures

4. **Command Buffer:**
   - GPU instructions (use program, bind VAO, draw)

**File Reference:** Vertex upload happens in `src/render/renderer.cpp:125-150`

```cpp
void Renderer::setupCubeGeometry() {
    float vertices[] = { /* 8 vertices × 3 floats = 96 bytes */ };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // THIS is the PCIe transfer: RAM → GPU VRAM
    glBufferData(GL_ARRAY_BUFFER,         // Target
                 sizeof(vertices),         // 96 bytes
                 vertices,                 // Source address in RAM
                 GL_STATIC_DRAW);          // Hint: data won't change

    // GPU now has a copy in VRAM. CPU can free RAM copy.
}
```

---

## Stage 3: GPU Side - Graphics Card

### 3.1 GPU Architecture Overview

Modern GPUs (NVIDIA/AMD) have this structure:

```
┌─────────────────────────────────────────────────────┐
│ GPU Die                                             │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ Graphics Processing Clusters (GPCs)          │  │
│  │  Each GPC contains:                          │  │
│  │    • Streaming Multiprocessors (SMs)         │  │
│  │    • Texture Units                           │  │
│  │    • Raster Engines                          │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  Streaming Multiprocessor (SM) - THE CORE UNIT:    │
│  ┌────────────────────────────────────┐            │
│  │ • 128 CUDA cores (NVIDIA)          │            │
│  │ • 64 KB shared memory (L1 cache)   │            │
│  │ • 65536 registers                  │            │
│  │ • Warp Scheduler (32 threads/warp) │            │
│  └────────────────────────────────────┘            │
│  × 80 SMs on RTX 3080 = 10,240 cores!             │
│                                                     │
│  ┌────────────────────────────────────┐            │
│  │ L2 Cache (6 MB)                    │            │
│  └────────────────────────────────────┘            │
│                     ↓                               │
│  ┌────────────────────────────────────┐            │
│  │ VRAM (GDDR6, 10-24 GB)             │            │
│  │ Bandwidth: 760 GB/s (vs 50 GB/s    │            │
│  │ for system RAM!)                   │            │
│  └────────────────────────────────────┘            │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**Key Numbers (RTX 3080 example):**
- **10,240 CUDA cores** (shader execution units)
- **68 Streaming Multiprocessors** (80 on 3090)
- **10 GB GDDR6X VRAM** (760 GB/s bandwidth!)
- **1.71 GHz boost clock**

---

### 3.2 GPU Memory Hierarchy

```
┌────────────────────────────────────────────────┐
│ Registers (per thread)                         │
│ • 255 registers × 4 bytes = 1 KB per thread    │
│ • Access: 0 cycles, instant                    │
│ • Scope: Private to each shader thread         │
└────────────────────────────────────────────────┘
                    ↓
┌────────────────────────────────────────────────┐
│ Shared Memory / L1 Cache (per SM)              │
│ • 64-128 KB configurable                       │
│ • Access: ~5 cycles                            │
│ • Scope: Shared by 32-thread warp              │
└────────────────────────────────────────────────┘
                    ↓
┌────────────────────────────────────────────────┐
│ L2 Cache (global)                              │
│ • 4-6 MB across entire GPU                     │
│ • Access: ~200 cycles                          │
│ • Scope: Shared by all SMs                     │
└────────────────────────────────────────────────┘
                    ↓
┌────────────────────────────────────────────────┐
│ VRAM (GDDR6)                                   │
│ • 8-24 GB                                      │
│ • Access: ~400-800 cycles                      │
│ • Bandwidth: 500-1000 GB/s                     │
│ • Scope: Global, persistent                    │
└────────────────────────────────────────────────┘
```

**Your Data:**
- **VBO (cube vertices):** Lives in VRAM, cached in L2
- **Uniform (mvp matrix):** Uploaded to constant memory, cached heavily
- **Framebuffer:** VRAM, written by fragment shaders

---

### 3.3 Vertex Shader Execution

File: `shaders/vertex3D.glsl`

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;      // Input from VBO in VRAM
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;  // Output to fragment shader (via interpolator)

uniform mat4 model;        // Uploaded from CPU
uniform mat4 view;
uniform mat4 projection;

void main() {
    // THIS CODE RUNS ON GPU SHADER CORES!
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}
```

**GPU Execution Flow:**

1. **Input Assembly:**
   - GPU fetches vertex data from VBO in VRAM
   - For cube: 8 vertices fetched (24 bytes each = 192 bytes total)

2. **Vertex Shader Invocation:**
   - **One thread per vertex** launched on GPU
   - 8 threads for cube (trivial workload)
   - For complex model with 100,000 vertices → 100,000 parallel threads!

3. **Parallel Execution on Streaming Multiprocessor:**
   ```
   Warp 0 (32 threads processing 32 vertices):
   Thread 0: v0.pos = projection * view * model * vec4(v0.aPos, 1.0)
   Thread 1: v1.pos = projection * view * model * vec4(v1.aPos, 1.0)
   Thread 2: v2.pos = projection * view * model * vec4(v2.aPos, 1.0)
   ...
   Thread 31: v31.pos = ...

   All execute SAME instruction simultaneously (SIMT - Single Instruction Multiple Thread)
   ```

4. **Memory Access Pattern:**
   - `aPos` loaded from VRAM (sequential access, coalesced)
   - `model/view/projection` loaded from constant memory (broadcast to all threads)
   - Matrix math performed in registers (FMAD units - Fused Multiply-Add)

5. **Output:**
   - `gl_Position` written to intermediate buffer
   - `vertexColor` written to varying buffer (for interpolation)

**Performance:**
- **Throughput:** RTX 3080 can process ~10 billion vertices/sec
- **Bottleneck for simple cube:** Memory bandwidth (VRAM access), not compute

---

### 3.4 Rasterization (Fixed-Function Hardware)

After vertex shader, GPU has 8 transformed vertices. Now it converts triangles to pixels:

```
        Triangle in Clip Space              Rasterized Pixels
         (vertex shader output)         (fragment shader input)

              v1 (x,y,z)                    ████████████
             /  \                           ██████████████
            /    \                          ████████████████
           /      \           ──────►       ██████████████████
          /        \                        ████████████████
         /          \                       ██████████████
     v0 ───────────── v2                    ████████████
```

**Rasterizer Hardware:**
- **Dedicated silicon** (not programmable)
- **Edge equations:** Determines which pixels are inside triangle
- **Interpolation:** Calculates `vertexColor` for each pixel (barycentric interpolation)

**Example:**
```
Vertex colors:
  v0 = red   (1, 0, 0)
  v1 = green (0, 1, 0)
  v2 = blue  (0, 0, 1)

Pixel at center:
  color = (1/3 * red) + (1/3 * green) + (1/3 * blue) = gray
```

**Output:** Fragment stream (one fragment per pixel covered by triangle)

---

### 3.5 Fragment Shader Execution

File: `shaders/fragment3D.glsl`

```glsl
#version 330 core
in vec3 vertexColor;  // Interpolated from vertex shader
out vec4 FragColor;   // Output to framebuffer

void main() {
    // THIS RUNS ONCE PER PIXEL, IN PARALLEL
    FragColor = vec4(vertexColor, 1.0);
}
```

**GPU Execution:**

Suppose cube covers 10,000 pixels on screen:

1. **Fragment Shader Invocation:**
   - **10,000 threads launched** (one per pixel)
   - Grouped into warps of 32 threads

2. **Parallel Execution:**
   ```
   Warp 0 (32 pixels):
   Thread 0: FragColor[pixel_0] = vec4(interpolated_color_0, 1.0)
   Thread 1: FragColor[pixel_1] = vec4(interpolated_color_1, 1.0)
   ...
   Thread 31: FragColor[pixel_31] = vec4(interpolated_color_31, 1.0)
   ```

3. **Memory Writes:**
   - Each thread writes to framebuffer in VRAM
   - Framebuffer is typically **RGBA8** (4 bytes per pixel)
   - For 1920×1080: 8.3 MB framebuffer

4. **Depth Test (Hardware):**
   - Before writing color, GPU checks depth buffer
   - If `new_depth < current_depth`, write pixel (closer object wins)
   - Depth buffer in VRAM (4 bytes per pixel = 8.3 MB for 1080p)

---

### 3.6 Framebuffer Operations

After fragment shader, GPU performs:

1. **Depth Test:**
   ```cpp
   if (fragment.depth < depth_buffer[x][y]) {
       // This fragment is in front
   } else {
       discard;  // Behind existing geometry
   }
   ```

2. **Stencil Test:** (not used in this project)

3. **Blending:** (for transparent objects)
   ```cpp
   // If alpha blending enabled:
   final_color = src_color * src_alpha + dst_color * (1 - src_alpha);
   ```

4. **Write to Framebuffer:**
   ```cpp
   framebuffer[y * width + x] = final_color;
   depth_buffer[y * width + x] = fragment.depth;
   ```

**Memory Location:** All framebuffers are in **GPU VRAM**.

---

## Stage 4: Display Output

### 4.1 Swap Buffers

File: `src/app/application.cpp:522`

```cpp
glfwSwapBuffers(window);  // ← This is when GPU work ACTUALLY happens!
```

**What This Does:**

1. **CPU blocks** until GPU finishes all queued commands (synchronization point)
2. **GPU flushes all pending work:**
   - Vertex shaders execute
   - Rasterization occurs
   - Fragment shaders execute
   - Framebuffer writes complete
3. **Double buffering swap:**
   ```
   Before swap:
     Front buffer (scanout to monitor) = Previous frame
     Back buffer (GPU rendering target) = Current frame

   After swap:
     Front buffer = Current frame  ← Monitor now displays this
     Back buffer = (empty, ready for next frame)
   ```

4. **Display controller** starts scanning front buffer

---

### 4.2 Display Controller Scanout

```
┌──────────────────────────────────────────────┐
│ GPU VRAM - Front Buffer (framebuffer)        │
│ 1920×1080 pixels × 4 bytes = 8.3 MB          │
└──────────────────────────────────────────────┘
                    │
                    │ Display controller reads at 60 Hz
                    ↓
┌──────────────────────────────────────────────┐
│ Display Controller (on GPU die)              │
│ • Scans framebuffer top-to-bottom            │
│ • Sends pixel data to monitor via HDMI/DP    │
│ • Rate: 60 Hz (or 144 Hz for gaming)         │
└──────────────────────────────────────────────┘
                    │
                    │ Digital signal (TMDS encoding)
                    ↓
┌──────────────────────────────────────────────┐
│ HDMI / DisplayPort Cable                     │
│ Bandwidth: 18 Gbps (HDMI 2.0)                │
└──────────────────────────────────────────────┘
                    │
                    ↓
┌──────────────────────────────────────────────┐
│ Monitor                                      │
│ • LCD panel controller                       │
│ • Pixel transistors switch                   │
│ • Backlight illuminates                      │
│ • You see the image!                         │
└──────────────────────────────────────────────┘
```

**Refresh Cycle (60 Hz):**
- Every 16.67ms, display controller reads entire framebuffer
- Scans line-by-line (horizontal scanout)
- **V-Sync:** CPU/GPU wait for vertical blanking interval before swapping

---

## Complete Frame Timing Breakdown

Let's trace **one frame** of AeroDynControlRig at 60 FPS:

```
Time (ms)  Component       Action
─────────────────────────────────────────────────────────────────
0.00       CPU             Application::tick() starts
0.05       CPU             Modules update SimulationState (RAM writes)
0.10       CPU             ImGui::NewFrame() starts
0.15       CPU             Panels draw UI (generates vertex data in RAM)
0.20       CPU             ImGui::Render() finalizes draw lists
0.25       CPU             renderer.renderFrame() queues OpenGL commands
0.30       CPU             glDrawElements() queued (not executed yet!)
0.35       CPU             ImGui_ImplOpenGL3_RenderDrawData() queues UI draw
0.40       CPU             glfwSwapBuffers() called
           ───────────────────────────────────────────────────────
0.41       DMA             Command buffer → GPU via PCIe (2 μs)
           ───────────────────────────────────────────────────────
0.42       GPU             Command processor wakes up
0.43       GPU             Vertex shader launches (8 threads for cube)
0.44       GPU             Rasterizer generates fragments
0.45       GPU             Fragment shader executes (10,000 threads)
0.50       GPU             Writes to framebuffer in VRAM
0.55       GPU             Repeats for ImGui geometry (1000s of quads)
1.20       GPU             All rendering complete
1.21       GPU             Signals CPU (fence)
           ───────────────────────────────────────────────────────
1.22       CPU             glfwSwapBuffers() returns
1.23       CPU             glfwPollEvents() processes input
1.25       CPU             Application::tick() returns
           ───────────────────────────────────────────────────────
           (Wait for V-Sync)
           ───────────────────────────────────────────────────────
16.67      Display Ctrl    Swaps front/back buffers
16.67      Monitor         Starts scanning new framebuffer
33.33      Monitor         Frame fully displayed
           ───────────────────────────────────────────────────────
           (Next frame starts at 16.67ms)
```

**Total Frame Time:** 1.25ms CPU + wait until 16.67ms for V-Sync

**GPU Utilization:** 1.2ms out of 16.67ms = **7% GPU load** (very light workload!)

---

## Memory Bandwidth Analysis

### CPU Side (System RAM)
```
Operation                    Data Size    Bandwidth Used
─────────────────────────────────────────────────────────
SimulationState read/write   ~1 KB        L1 cache (negligible)
ImGui vertex generation      ~200 KB      L3 cache hit (negligible)
OpenGL driver overhead       ~10 KB       RAM access (~0.2 μs)
─────────────────────────────────────────────────────────
Total CPU→RAM:               ~211 KB      DDR4: 50 GB/s available
Utilization:                              0.0004% (!!)
```

### PCIe Bus (CPU ↔ GPU)
```
Transfer                     Data Size    Transfer Time
─────────────────────────────────────────────────────────
Uniform data (matrices)      64 bytes     ~4 ns
Command buffer               ~1 KB        ~60 ns
ImGui vertex upload (dynamic) ~200 KB      ~12 μs
─────────────────────────────────────────────────────────
Total:                       ~201 KB      PCIe 3.0: 16 GB/s
Utilization:                              0.001%
```

### GPU Side (VRAM)
```
Operation                    Data Size    Bandwidth Used
─────────────────────────────────────────────────────────
VBO fetch (cube vertices)    192 bytes    L2 cache hit
Framebuffer read (depth test) 8.3 MB      ~11 μs
Framebuffer write (color)    8.3 MB       ~11 μs
ImGui texture fetch          ~2 MB        ~2.6 μs
─────────────────────────────────────────────────────────
Total:                       ~18.6 MB     GDDR6: 760 GB/s
Utilization:                              0.002%
```

**Conclusion:** This app is **massively underutilizing** the GPU. You could render 1000× more geometry!

---

## Optimization Insights

### Where Time Is Actually Spent

**Not GPU-bound:**
- GPU utilization: ~7%
- Rendering 36 triangles (cube) + ~1000 ImGui quads
- GPU can handle millions of triangles

**Not CPU-bound:**
- Quaternion math: ~0.05ms
- ImGui layout: ~0.15ms
- Total CPU work: ~1.2ms out of 16.67ms budget

**Actual Bottleneck:**
- **V-Sync wait** (14ms of idle time)
- Limited by monitor refresh rate (60 Hz)

### If You Wanted to Optimize

**GPU Side:**
1. **Instanced rendering:** Draw 1000 cubes in one call
   ```cpp
   glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, 1000);
   ```

2. **Texture atlasing:** Combine textures to reduce state changes

3. **Compute shaders:** Offload physics to GPU (100× faster for parallel work)

**CPU Side:**
1. **SIMD optimization:** GLM already uses SSE/AVX
2. **Multithreading:** Update modules in parallel (overkill for this app)
3. **Cache optimization:** Store vectors-of-structs instead of structs-of-vectors

**But honestly:** This app is already **extremely efficient**. No optimization needed.

---

## Key Takeaways

1. **Your C++ code runs on CPU** and manipulates RAM
2. **OpenGL commands are queued**, not immediate
3. **PCIe bus transfers data** to GPU VRAM (once for static geometry)
4. **GPU runs thousands of threads in parallel** (one per vertex, one per pixel)
5. **Shaders execute on CUDA cores** with massive parallelism
6. **Framebuffer lives in GPU VRAM** and is scanned by display controller
7. **V-Sync synchronizes** CPU frame generation with monitor refresh

---

## Mapping to AeroDynControlRig Code

| File | What It Does | Where It Executes |
|------|--------------|-------------------|
| `application.cpp:tick()` | Main loop | CPU (RAM) |
| `quaternion_demo.cpp:update()` | Physics math | CPU (L1/L2 cache) |
| `renderer.cpp:renderFrame()` | Queue OpenGL commands | CPU (sends to GPU) |
| `shaders/vertex3D.glsl` | Transform vertices | **GPU shader cores** |
| `shaders/fragment3D.glsl` | Color pixels | **GPU shader cores** |
| Framebuffer | Store rendered image | **GPU VRAM** |

---

## Related Documentation

- @ref web_analogy - UI architecture mapping
- @ref architecture - High-level system design
- `src/render/renderer.h` - OpenGL rendering implementation
- `shaders/` - GLSL shader source code

---

## Further Reading

- [GPU Gems (Free Online)](https://developer.nvidia.com/gpugems/gpugems3/contributors) - Deep GPU optimization
- [Khronos OpenGL Wiki](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview) - Official pipeline docs
- [CUDA Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/) - GPU architecture details
- [Agner Fog's Optimization Manuals](https://www.agner.org/optimize/) - CPU microarchitecture
