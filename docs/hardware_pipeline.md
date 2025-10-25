/**
 * @page hardware_pipeline Hardware Pipeline: RAM → CPU → GPU → Screen
 *
 * @tableofcontents
 *
 * Understanding how OpenGL and ImGui work requires understanding the **hardware pipeline** -
 * how data flows from your RAM, through CPU caches, to the GPU, and finally to your screen.
 *
 * This guide explains the complete journey with concrete examples from our codebase.
 *
 * ---
 *
 * ## The Big Picture: Two Processors on One Die
 *
 * Modern CPUs (Intel, AMD Ryzen with integrated graphics) have **TWO processors on the same chip**:
 *
 * 1. **CPU cores** - Your main processor cores
 *    - 4-16 high-performance cores
 *    - Each has its own cache hierarchy: L1 (32 KB) → L2 (256 KB) → shared L3 (8-32 MB)
 *    - Accesses system RAM (DDR4/DDR5)
 *    - Good at: Sequential logic, branches, complex algorithms
 *    - Bad at: Doing the same operation on millions of items
 *
 * 2. **iGPU (integrated GPU)** - Graphics processor on the same die
 *    - Intel UHD Graphics, AMD Radeon Graphics, Apple M-series GPU
 *    - 96-512 execution units (think of them like mini-cores)
 *    - **Shares system RAM** with CPU (no dedicated VRAM!)
 *    - Has its own L3 cache for graphics (separate from CPU L3)
 *    - Good at: Doing the same operation on millions of items in parallel
 *    - Bad at: Complex branching logic
 *
 * **Key insight for integrated GPUs**:
 * - CPU and GPU are on the **same physical die**
 * - They **share the same system RAM** (DDR4/DDR5)
 * - Memory is partitioned: part for CPU, part reserved for GPU
 * - Data transfer is faster than discrete GPU (no PCIe!) but still requires explicit copies
 * - Memory bandwidth is shared (~50 GB/s split between CPU and GPU)
 *
 * ---
 *
 * ## What is OpenGL?
 *
 * OpenGL is **NOT** a library that runs on the CPU or GPU. It's a **specification** - a contract.
 *
 * Think of it like this:
 * - **HTTP** is a specification (protocol) for web communication
 * - **OpenGL** is a specification (API) for graphics communication
 *
 * ### OpenGL is a Command Queue Interface
 *
 * When you call OpenGL functions like `glDrawElements()`, you are **NOT** executing graphics code.
 * You are **sending commands** to the GPU driver, which queues them for the GPU.
 *
 * ```cpp
 * // This does NOT execute rendering immediately!
 * glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
 *
 * // This just adds a command to the queue:
 * // "Dear GPU, please draw 36 vertices as triangles"
 * ```
 *
 * The **actual rendering** happens on the GPU when:
 * 1. You call `glFlush()` or `glFinish()` (explicit)
 * 2. You call `glfwSwapBuffers()` (implicit flush)
 * 3. The driver decides the queue is full enough
 *
 * ### Who Implements OpenGL?
 *
 * - **NVIDIA** implements OpenGL for NVIDIA GPUs
 * - **AMD** implements OpenGL for AMD GPUs
 * - **Intel** implements OpenGL for Intel integrated graphics
 * - **Mesa** implements OpenGL for open-source drivers (Linux)
 *
 * Each vendor writes a **driver** that translates OpenGL commands into GPU-specific machine code.
 *
 * ---
 *
 * ## What is ImGui?
 *
 * ImGui is a **C++ library** that runs **100% on the CPU**.
 *
 * ### What ImGui Does:
 *
 * 1. **Processes UI logic** (button clicks, text input, layout calculations)
 * 2. **CREATES vertex data** for UI elements (rectangles, text, images)
 * 3. **Builds a draw list** - a list of triangles to draw with their positions, colors, UVs
 *
 * ### What ImGui Does NOT Do:
 *
 * ImGui **does not render anything** itself. It just produces **data** (vertex arrays).
 *
 * Think of ImGui like this:
 * ```
 * You: "ImGui, I want a button labeled 'Start' at position (10, 20)"
 *
 * ImGui: *calculates layout, checks mouse input*
 *         "Okay, I've CREATED this vertex data for you:
 *
 *          Button background (rectangle = 2 triangles = 6 vertices):
 *            Vertex 0: position=(10, 20),   color=(0.2, 0.3, 0.8, 1.0)
 *            Vertex 1: position=(110, 20),  color=(0.2, 0.3, 0.8, 1.0)
 *            Vertex 2: position=(110, 60),  color=(0.2, 0.3, 0.8, 1.0)
 *            Vertex 3: position=(10, 20),   color=(0.2, 0.3, 0.8, 1.0)
 *            Vertex 4: position=(110, 60),  color=(0.2, 0.3, 0.8, 1.0)
 *            Vertex 5: position=(10, 60),   color=(0.2, 0.3, 0.8, 1.0)
 *
 *          Text 'Start' (each letter = 2 triangles with texture coords):
 *            Letter 'S': 6 vertices with UV coords to font atlas...
 *            Letter 't': 6 vertices with UV coords to font atlas...
 *            ... etc
 *
 *          Total: 36 vertices in my draw list"
 *
 * You: "Thanks! Now I'll use OpenGL to upload this to the GPU and render it"
 * ```
 *
 * **ImGui is a geometry generator, not a renderer!**
 *
 * ### ImGui Backends
 *
 * To actually **display** the UI, ImGui needs a **rendering backend**:
 * - `imgui_impl_opengl3.cpp` - Uses OpenGL to render
 * - `imgui_impl_vulkan.cpp` - Uses Vulkan to render
 * - `imgui_impl_dx11.cpp` - Uses DirectX 11 to render
 *
 * Our project uses **`imgui_impl_opengl3.cpp`**, which:
 * 1. Takes ImGui's draw list (CPU data)
 * 2. Uploads it to GPU via OpenGL
 * 3. Issues OpenGL draw calls
 *
 * ---
 *
 * ## OpenGL VBO/VAO: Storage vs Creation
 *
 * **Critical distinction**: OpenGL provides **storage containers** for vertex data.
 * It does NOT create the vertex data itself!
 *
 * ### What YOU Do (Application Code)
 *
 * **You create the geometry** - you decide the vertex positions, colors, etc.
 *
 * **Example from our cube renderer** (`src/render/renderer.cpp`):
 *
 * ```cpp
 * void Renderer::init() {
 *     // YOU create the vertex data (geometry) in CPU memory
 *     float vertices[] = {
 *         // Position          Color
 *         -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  // Red vertex
 *          0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Green vertex
 *          0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  // Blue vertex
 *         // ... 33 more vertices
 *     };
 *
 *     // YOU decided:
 *     // - A cube has 8 corners = 36 vertices (with duplicates for colors)
 *     // - Each vertex has position (x, y, z) and color (r, g, b)
 *     // - Specific values: (-0.5, -0.5, -0.5) for first corner, etc.
 * }
 * ```
 *
 * ### What OpenGL VBO Does (Storage Container)
 *
 * **VBO (Vertex Buffer Object)** is just a **storage container** on the GPU side.
 * Think of it like a `std::vector<float>` but in GPU memory.
 *
 * ```cpp
 * // Create an empty storage container on GPU
 * GLuint vbo;
 * glGenBuffers(1, &vbo);  // GPU: "I created an empty buffer with ID 42"
 *
 * // Make it the "active" buffer (like selecting a file to edit)
 * glBindBuffer(GL_ARRAY_BUFFER, vbo);  // GPU: "Buffer 42 is now active"
 *
 * // Copy YOUR vertex data into the GPU storage container
 * glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 * //           ^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^  ^^^^^^^^
 * //           Where to put it   How big          Pointer to YOUR data
 *
 * // GPU: "I copied your 216 bytes of data into buffer 42"
 * ```
 *
 * **VBO did NOT create the cube geometry!**
 * - VBO just **stored** the geometry you created
 * - It's like uploading a file to cloud storage - the cloud didn't create the file!
 *
 * ### What OpenGL VAO Does (Format Description)
 *
 * **VAO (Vertex Array Object)** describes the **format** of data in the VBO.
 * It's like a "schema" or "struct definition" for the vertex data.
 *
 * ```cpp
 * // Create a VAO (format descriptor)
 * GLuint vao;
 * glGenVertexArrays(1, &vao);  // GPU: "I created format descriptor ID 7"
 * glBindVertexArray(vao);      // GPU: "Format 7 is now active"
 *
 * // Tell OpenGL how to INTERPRET the data in VBO
 * // Position attribute (first 3 floats of each vertex)
 * glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
 * //                    ^  ^                       ^^^^^^^^^^^^^^^^^^  ^^^^^^^^^^
 * //                    |  |                       |                   Offset = 0
 * //                    |  |                       Stride = 6 floats per vertex
 * //                    |  Size = 3 components (x, y, z)
 * //                    Location = 0 (matches shader "layout(location=0)")
 *
 * glEnableVertexAttribArray(0);
 *
 * // Color attribute (next 3 floats of each vertex)
 * glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
 * //                    ^  ^                                          ^^^^^^^^^^^^^^^^^^^^^^^^
 * //                    |  |                                          Offset = 3 floats
 * //                    |  Size = 3 components (r, g, b)
 * //                    Location = 1
 *
 * glEnableVertexAttribArray(1);
 * ```
 *
 * **VAO is just metadata!** It tells the GPU:
 * - "Attribute 0 is 3 floats starting at offset 0, step forward 6 floats to next vertex"
 * - "Attribute 1 is 3 floats starting at offset 3, step forward 6 floats to next vertex"
 *
 * Think of VBO as a flat array: `[x, y, z, r, g, b, x, y, z, r, g, b, ...]`
 *
 * VAO tells the GPU how to parse it: `[(x,y,z), (r,g,b)], [(x,y,z), (r,g,b)], ...`
 *
 * ### Summary: You Create, OpenGL Stores
 *
 * | Component | Role | Analogy |
 * |-----------|------|---------|
 * | **Your code** | Creates vertex data (geometry) | Chef preparing ingredients |
 * | **VBO** | Stores vertex data on GPU | Refrigerator storing ingredients |
 * | **VAO** | Describes data format | Recipe card explaining ingredient layout |
 * | **Shader** | Processes vertices on GPU | Oven cooking the food |
 * | **glDrawElements()** | Triggers rendering | Serving the dish |
 *
 * **You are the geometry creator!** OpenGL is just the storage and processing pipeline.
 *
 * ---
 *
 * ## ImGui: Dynamic Geometry Generator
 *
 * Now here's where ImGui is different from our cube:
 *
 * ### Our Cube (Static Geometry)
 *
 * ```cpp
 * // We create geometry ONCE in init()
 * float vertices[] = { /* 36 vertices for a cube */ };
 * glBufferData(..., vertices, GL_STATIC_DRAW);  // Upload once
 *
 * // Every frame: just draw the SAME geometry
 * glDrawElements(...);  // Same cube, maybe rotated via uniforms
 * ```
 *
 * ### ImGui (Dynamic Geometry)
 *
 * ```cpp
 * // EVERY FRAME, ImGui creates NEW geometry based on UI state
 * void MyPanel::draw() {
 *     if (ImGui::Button("Start")) { /* ... */ }  // Button state might change!
 *     ImGui::Text("FPS: %d", fps);                // Text changes every frame!
 * }
 *
 * // Internally, ImGui generates NEW vertices every frame:
 * // Frame 1: "FPS: 60" → 24 vertices for text
 * // Frame 2: "FPS: 59" → 24 vertices for text (different positions!)
 * // Frame 3: "FPS: 61" → 24 vertices for text (different positions!)
 * ```
 *
 * **Why does ImGui generate new geometry every frame?**
 *
 * Because the UI is **dynamic**:
 * - Text content changes ("FPS: 60" → "FPS: 59")
 * - Button states change (normal → hovered → pressed)
 * - Windows can be moved, resized, opened, closed
 * - New panels can appear
 *
 * ImGui recalculates the ENTIRE UI layout and generates fresh vertex data every frame!
 *
 * ### ImGui's OpenGL Backend: The Glue
 *
 * **File**: `external/imgui/backends/imgui_impl_opengl3.cpp`
 *
 * This is the bridge between ImGui (geometry generator) and OpenGL (storage/renderer):
 *
 * ```cpp
 * void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data) {
 *     // draw_data contains ImGui's vertex arrays (created by ImGui!)
 *
 *     for (int n = 0; n < draw_data->CmdListsCount; n++) {
 *         const ImDrawList* cmd_list = draw_data->CmdLists[n];
 *
 *         // Get ImGui's generated vertices
 *         const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;  // ImGui created this!
 *         const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;   // ImGui created this!
 *
 *         // Upload to GPU (EVERY FRAME because vertices change!)
 *         glBufferData(GL_ARRAY_BUFFER,
 *                      cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
 *                      vtx_buffer,  // ImGui's vertex data
 *                      GL_STREAM_DRAW);  // STREAM = changes every frame
 *
 *         glBufferData(GL_ELEMENT_ARRAY_BUFFER,
 *                      cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
 *                      idx_buffer,  // ImGui's index data
 *                      GL_STREAM_DRAW);
 *
 *         // Draw ImGui's geometry
 *         for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
 *             const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
 *             glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, ...);
 *         }
 *     }
 * }
 * ```
 *
 * **Flow every frame**:
 * 1. ImGui **creates** vertices for buttons, text, windows (CPU)
 * 2. Backend **uploads** vertices to VBO via `glBufferData()` (CPU → GPU)
 * 3. Backend **draws** vertices via `glDrawElements()` (GPU execution)
 *
 * ---
 *
 * ## The Complete Picture
 *
 * ```
 * ┌─────────────────────────────────────────────────────────────────┐
 * │ WHO CREATES GEOMETRY?                                           │
 * ├─────────────────────────────────────────────────────────────────┤
 * │                                                                 │
 * │  ┌──────────────────────┐         ┌──────────────────────┐     │
 * │  │ Our Cube (Renderer)  │         │ ImGui UI             │     │
 * │  │                      │         │                      │     │
 * │  │ WE create geometry:  │         │ IMGUI creates geom:  │     │
 * │  │  float vertices[] =  │         │  Button() generates  │     │
 * │  │  { -0.5, -0.5, ... } │         │  → 6 vertices        │     │
 * │  │                      │         │  Text() generates    │     │
 * │  │ Created ONCE in      │         │  → 24 vertices       │     │
 * │  │ init()               │         │                      │     │
 * │  │                      │         │ Created EVERY frame  │     │
 * │  └──────────┬───────────┘         └──────────┬───────────┘     │
 * │             │                                │                 │
 * │             └────────────┬───────────────────┘                 │
 * │                          ▼                                     │
 * │              ┌───────────────────────┐                         │
 * │              │ OpenGL (Storage Only) │                         │
 * │              │                       │                         │
 * │              │ VBO: Stores vertices  │                         │
 * │              │ VAO: Describes format │                         │
 * │              │ Does NOT create geom! │                         │
 * │              └───────────┬───────────┘                         │
 * │                          ▼                                     │
 * │              ┌───────────────────────┐                         │
 * │              │ GPU (Processes Only)  │                         │
 * │              │                       │                         │
 * │              │ Shaders: Transform    │                         │
 * │              │ vertices              │                         │
 * │              │ Does NOT create geom! │                         │
 * │              └───────────────────────┘                         │
 * └─────────────────────────────────────────────────────────────────┘
 * ```
 *
 * **Key insight**:
 * - **You (or ImGui)** are the geometry creators
 * - **OpenGL** is just the delivery system (VBO = storage, VAO = format)
 * - **GPU** is just the processor (shaders transform what you created)
 *
 * Neither OpenGL nor GPU create geometry - they only store and process what you give them!
 *
 * ---
 *
 * ## The Complete Hardware Pipeline
 *
 * Let's trace a single frame from our application through the hardware:
 *
 * ### Step 1: Application Code (CPU)
 *
 * **File**: `src/app/application.cpp` - Application::tick()
 *
 * ```cpp
 * void Application::tick() {
 *     // This runs on CPU cores
 *     // Data is in system RAM, cached in L1/L2/L3
 *
 *     for (auto& module : modules) {
 *         module->update(dt, state);  // Update quaternion, sensors, etc.
 *     }
 * }
 * ```
 *
 * **Hardware view**:
 * ```
 * CPU Core 0:
 *   ├─ Fetch instruction from L1 I-cache (32 KB)
 *   ├─ Load SimulationState from L1 D-cache (32 KB) or L2 (256 KB) or L3 (8 MB)
 *   ├─ Execute quaternion math (FPU/SIMD units)
 *   └─ Write result back to cache, eventually to RAM
 * ```
 *
 * At this point, all data is **still on the CPU side** in system RAM.
 *
 * ---
 *
 * ### Step 2: Prepare GPU Data (CPU → GPU Transfer)
 *
 * **File**: `src/render/renderer.cpp` - Renderer::init()
 *
 * ```cpp
 * void Renderer::init() {
 *     // CPU: Create vertex data in system RAM
 *     float vertices[] = {
 *         -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  // Red vertex
 *         // ... more vertices
 *     };
 *
 *     // Generate a buffer ID on the GPU
 *     glGenBuffers(1, &vbo);
 *
 *     // Bind it as the active buffer
 *     glBindBuffer(GL_ARRAY_BUFFER, vbo);
 *
 *     // CRITICAL: Transfer data from system RAM to GPU VRAM
 *     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 *     //           ^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^  ^^^^^^^^
 *     //           Target            Size              Pointer to RAM
 * }
 * ```
 *
 * **Hardware view (integrated GPU)**:
 * ```
 * 1. CPU prepares data in system RAM (CPU-accessible region)
 *    RAM Address 0x7fff1234: [vertex data: -0.5, -0.5, -0.5, 1.0, ...]
 *
 * 2. glBufferData() triggers memory copy to GPU-accessible region
 *    CPU side of RAM → GPU side of RAM
 *
 *    Both regions are in the SAME physical RAM chips!
 *    Transfer happens via memory controller on the CPU die.
 *
 * 3. GPU can now access data in its reserved memory region
 *    RAM Address 0x8000000: [vertex data: -0.5, -0.5, -0.5, 1.0, ...]
 *
 *    GPU reads from system RAM (not separate VRAM).
 *    Bandwidth: ~50 GB/s shared between CPU and GPU.
 * ```
 *
 * **Why do we still copy?**
 * - Even though it's the same RAM, the GPU has a reserved region
 * - Driver manages memory partitioning
 * - Copy ensures data is in GPU-optimized layout (cache-friendly)
 * - Still do it once in init() and reuse every frame!
 *
 * ---
 *
 * ### Step 3: Upload Per-Frame Data (CPU → GPU Transfer)
 *
 * **File**: `src/render/renderer.cpp` - Renderer::draw()
 *
 * ```cpp
 * void Renderer::draw(const SimulationState& state) {
 *     // CPU: Read quaternion from system RAM
 *     const auto& q = state.quaternion;
 *
 *     // CPU: Calculate model matrix (4x4 = 16 floats = 64 bytes)
 *     glm::mat4 model = glm::mat4_cast(q);
 *
 *     // Upload to GPU uniform (small data, happens every frame)
 *     glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
 *     //                 ^^^^^^^^  ^             ^^^^^^^^^^^^^^^^^^^^
 *     //                 Location  Count         Pointer to CPU data
 * }
 * ```
 *
 * **Hardware view (integrated GPU)**:
 * ```
 * CPU (L1 cache): model matrix = [1.0, 0.0, 0.0, 0.0, ...]
 *                                  ↓
 *                         Copy to GPU region in RAM
 *                                  ↓
 * GPU memory region: model = [1.0, 0.0, 0.0, 0.0, ...]
 *                                  ↓
 *                         GPU reads via memory controller
 *                                  ↓
 * GPU L3 cache: model matrix cached for shader access
 *
 * This happens EVERY frame because the quaternion changes!
 * But it's only 64 bytes, so it's fast (memory controller handles it).
 * ```
 *
 * ---
 *
 * ### Step 4: Issue Draw Call (CPU → GPU Command)
 *
 * **File**: `src/render/renderer.cpp` - Renderer::draw()
 *
 * ```cpp
 * void Renderer::draw(const SimulationState& state) {
 *     // ... setup code ...
 *
 *     // Send draw command to GPU
 *     glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
 *     //              ^^^^^^^^^^^^  ^^  ^^^^^^^^^^^^^^^  ^
 *     //              Draw triangles|   Data type        Offset
 *     //                         Count
 * }
 * ```
 *
 * **Hardware view**:
 * ```
 * CPU: Writes command to GPU command queue in memory:
 *      Command: DRAW_INDEXED
 *      Mode: TRIANGLES
 *      Count: 36
 *      VBO: [handle to VRAM buffer]
 *      EBO: [handle to VRAM index buffer]
 *      Shader: [handle to compiled shader program]
 *      Uniforms: [pointers to uniform data in VRAM]
 *
 * GPU: Reads command from queue (DMA - Direct Memory Access)
 *      "Ah, I need to draw 36 vertices as triangles. Let's go!"
 * ```
 *
 * ---
 *
 * ### Step 5: GPU Vertex Processing (Parallel)
 *
 * The GPU now processes vertices **in parallel** on thousands of cores.
 *
 * **File**: `shaders/cube.vert` (Vertex Shader)
 *
 * ```glsl
 * #version 330 core
 * layout (location = 0) in vec3 aPos;    // Input from VBO
 * layout (location = 1) in vec3 aColor;  // Input from VBO
 *
 * uniform mat4 model;  // From CPU via glUniformMatrix4fv()
 * uniform mat4 view;
 * uniform mat4 proj;
 *
 * out vec3 fragColor;  // Output to fragment shader
 *
 * void main() {
 *     // This runs ONCE PER VERTEX on the GPU
 *     gl_Position = proj * view * model * vec4(aPos, 1.0);
 *     fragColor = aColor;
 * }
 * ```
 *
 * **Hardware view (integrated GPU)**:
 * ```
 * Integrated GPU has 96 execution units (example: Intel UHD Graphics 630)
 * Each execution unit can handle 7 threads simultaneously.
 * Total: 96 × 7 = 672 parallel threads
 *
 * We have 36 vertices to process.
 *
 * Vertex Shader Execution:
 *   EU 0, thread 0:  Vertex 0  → position = proj * view * model * (-0.5, -0.5, -0.5)
 *   EU 0, thread 1:  Vertex 1  → position = proj * view * model * ( 0.5, -0.5, -0.5)
 *   EU 1, thread 0:  Vertex 2  → position = proj * view * model * ( 0.5,  0.5, -0.5)
 *   ...
 *   EU 5, thread 1:  Vertex 35 → position = proj * view * model * (...)
 *
 * All 36 vertices processed in PARALLEL across multiple execution units!
 *
 * Each execution unit:
 *   1. Reads vertex position from system RAM (GPU's reserved region)
 *   2. Reads uniform matrices from system RAM (cached in GPU L3)
 *   3. Executes matrix multiplications (GPU ALU - FMA units)
 *   4. Writes output position to internal GPU registers
 * ```
 *
 * **Why is this fast?**
 * - GPU has many execution units (96 EUs vs 8 CPU cores)
 * - Each EU handles 7 threads simultaneously (SIMD architecture)
 * - All doing the **exact same operation** (matrix multiply)
 * - No branching, no complex logic
 * - Specialized for parallel floating-point math
 *
 * ---
 *
 * ### Step 6: GPU Rasterization (Fixed Function Hardware)
 *
 * After vertex processing, the GPU has a list of triangles in screen space.
 *
 * **Rasterization** converts triangles into pixels.
 *
 * ```
 * Input:  Triangle with vertices at (100, 100), (200, 100), (150, 200)
 * Output: List of all pixels inside the triangle
 *
 * Example: (100, 100), (101, 100), (102, 100), ..., (150, 150), ...
 *
 * For a 1920x1080 screen, this might be millions of pixels!
 * ```
 *
 * **Hardware view**:
 * ```
 * GPU Rasterizer (fixed-function unit):
 *   ├─ Receives 12 triangles (36 vertices / 3)
 *   ├─ For each triangle:
 *   │   ├─ Clip to screen bounds
 *   │   ├─ Find all pixels inside triangle (edge equations)
 *   │   └─ Interpolate vertex attributes (color, UV, etc.)
 *   └─ Output: ~500,000 fragments (pixels) to process
 * ```
 *
 * ---
 *
 * ### Step 7: GPU Fragment Processing (Parallel)
 *
 * For every pixel inside every triangle, run the **fragment shader**.
 *
 * **File**: `shaders/cube.frag` (Fragment Shader)
 *
 * ```glsl
 * #version 330 core
 * in vec3 fragColor;   // Interpolated from vertex shader
 * out vec4 finalColor; // Output to framebuffer
 *
 * void main() {
 *     // This runs ONCE PER PIXEL on the GPU
 *     finalColor = vec4(fragColor, 1.0);
 * }
 * ```
 *
 * **Hardware view (integrated GPU)**:
 * ```
 * Fragment Shader Execution (500,000 pixels):
 *   EU 0, thread 0:  Pixel (100, 100) → color = (1.0, 0.0, 0.0, 1.0)
 *   EU 0, thread 1:  Pixel (101, 100) → color = (1.0, 0.0, 0.0, 1.0)
 *   EU 0, thread 2:  Pixel (102, 100) → color = (0.98, 0.02, 0.0, 1.0)  ← interpolated!
 *   ...
 *   EU 95, thread 6: Pixel (543, 789) → color = (0.5, 0.5, 0.0, 1.0)
 *
 * All running in PARALLEL across 96 execution units × 7 threads each!
 *
 * Output goes to Framebuffer in system RAM (GPU's reserved region):
 *   RAM Address 0x80000000: [pixel data: RGBA RGBA RGBA ...]
 *
 * Framebuffer is in system RAM, not dedicated VRAM.
 * ```
 *
 * ---
 *
 * ### Step 8: ImGui Rendering (CPU generates, GPU renders)
 *
 * **File**: `src/gui/panels/telemetry_panel.cpp` - TelemetryPanel::draw()
 *
 * ```cpp
 * void TelemetryPanel::draw(SimulationState& state, Camera&) {
 *     // This runs on CPU
 *     ImGui::Begin("Telemetry");
 *     ImGui::Text("Quaternion: %.3f, %.3f, %.3f, %.3f",
 *                 state.quaternion.w, state.quaternion.x,
 *                 state.quaternion.y, state.quaternion.z);
 *     ImGui::End();
 * }
 * ```
 *
 * **What happens internally**:
 *
 * ```cpp
 * // ImGui (CPU side) generates draw commands:
 * DrawList = [
 *     DrawRect(x: 10, y: 10, w: 300, h: 400, color: 0x000000F0),  // Window background
 *     DrawText(x: 20, y: 30, text: "Quaternion: 1.000, 0.000, ...", color: 0xFFFFFFFF),
 *     // ... more draw commands
 * ];
 *
 * // Then ImGui_ImplOpenGL3_RenderDrawData() (CPU → GPU):
 * for (cmd in DrawList) {
 *     // Upload vertex data to GPU
 *     glBufferData(GL_ARRAY_BUFFER, cmd.vertices, GL_STREAM_DRAW);
 *
 *     // Draw
 *     glDrawElements(GL_TRIANGLES, cmd.count, ...);
 * }
 * ```
 *
 * **Hardware view**:
 * ```
 * CPU:
 *   ├─ ImGui calculates layout (system RAM)
 *   ├─ Generates vertex data: rectangles as triangles
 *   │   Text "Q" → 2 triangles (using font texture)
 *   │   Window background → 2 triangles
 *   └─ Sends to GPU via OpenGL
 *
 * GPU:
 *   ├─ Vertex shader: Transform UI vertices to screen space
 *   ├─ Fragment shader: Sample font texture, output pixel color
 *   └─ Write to framebuffer on top of 3D scene
 * ```
 *
 * ---
 *
 * ### Step 9: Display to Screen (GPU → Monitor)
 *
 * **File**: `src/app/application.cpp` - Application::tick()
 *
 * ```cpp
 * void Application::tick() {
 *     // ... render 3D ...
 *     // ... render UI ...
 *
 *     // Swap front and back buffers
 *     glfwSwapBuffers(window);
 * }
 * ```
 *
 * **Hardware view (integrated GPU)**:
 * ```
 * GPU has TWO framebuffers in system RAM:
 *   ├─ Front buffer: Currently being scanned out to monitor
 *   └─ Back buffer:  Currently being rendered to
 *
 * glfwSwapBuffers():
 *   ├─ Wait for V-Sync (monitor refresh, 60 Hz = 16.67ms)
 *   ├─ Swap pointers: back ↔ front (just pointer swap, no data copy!)
 *   └─ Display engine now scans out from new front buffer
 *
 * Display Engine (on CPU die):
 *   ├─ Reads pixel data from front buffer in system RAM
 *   │   Via memory controller (shared with CPU/GPU memory access)
 *   ├─ Converts to display signal (DisplayPort/HDMI)
 *   └─ Sends to monitor
 *
 * Monitor:
 *   ├─ Receives digital signal (DisplayPort/HDMI)
 *   └─ Updates physical pixels on screen (LCD backlight, LED, OLED)
 *
 * Refresh rate: 60 Hz = scan 1920×1080 pixels 60 times per second
 * Bandwidth needed: 1920 × 1080 × 4 bytes × 60 Hz = ~500 MB/s from RAM
 * ```
 *
 * ---
 *
 * ## Summary: The Complete Journey
 *
 * ```
 * ┌─────────────────────────────────────────────────────────────────┐
 * │ CPU Die (Intel/AMD with integrated graphics)                   │
 * │                                                                 │
 * │  ┌──────────────────────┐         ┌──────────────────────┐     │
 * │  │ CPU Cores (8 cores)  │         │ iGPU (96 EUs)        │     │
 * │  │  ├─ Your C++ code    │         │  ├─ Vertex Shader    │     │
 * │  │  ├─ ImGui logic      │         │  ├─ Rasterizer       │     │
 * │  │  └─ OpenGL API calls │         │  └─ Fragment Shader  │     │
 * │  └──────────┬───────────┘         └──────────┬───────────┘     │
 * │             │                                │                 │
 * │             └────────────┬───────────────────┘                 │
 * │                          │                                     │
 * │                  ┌───────▼────────┐                            │
 * │                  │ Memory         │                            │
 * │                  │ Controller     │                            │
 * │                  └───────┬────────┘                            │
 * └──────────────────────────┼──────────────────────────────────────┘
 *                            │
 *                   ┌────────▼────────┐
 *                   │ System RAM      │ 16 GB DDR4 (~50 GB/s)
 *                   │  ├─ CPU region  │ 14 GB for applications
 *                   │  ├─ GPU region  │ 2 GB reserved for graphics
 *                   │  └─ Framebuffer │ 1920×1080×4×2 = 16 MB
 *                   └────────┬────────┘
 *                            │
 *                   ┌────────▼────────┐
 *                   │ Display Engine  │ (on CPU die)
 *                   │  Reads from RAM │
 *                   └────────┬────────┘
 *                            │ DisplayPort/HDMI
 *                            ↓
 *                   ┌─────────────────┐
 *                   │ Monitor (60 Hz) │
 *                   └─────────────────┘
 * ```
 *
 * ---
 *
 * ## Key Takeaways
 *
 * 1. **Integrated GPU: Two processors on one die**
 *    - CPU cores and GPU execution units share the same chip
 *    - Both access the same system RAM (no separate VRAM)
 *    - Memory is partitioned by the driver
 *    - Data still needs to be copied to GPU-accessible region
 *
 * 2. **OpenGL is a command API, not a library**
 *    - Queues commands for the GPU driver
 *    - Driver translates to GPU-specific machine code
 *    - Execution happens asynchronously on GPU execution units
 *
 * 3. **ImGui is a CPU library**
 *    - Runs 100% on CPU cores
 *    - Generates vertex data and draw commands
 *    - Needs a backend (imgui_impl_opengl3.cpp) to render via OpenGL
 *
 * 4. **Shaders are GPU programs**
 *    - Written in GLSL (OpenGL Shading Language)
 *    - Compiled by driver to GPU machine code
 *    - Run in parallel across execution units (96 EUs × 7 threads = 672 parallel)
 *
 * 5. **Memory and bandwidth matter**
 *    - System RAM bandwidth: ~50 GB/s (shared between CPU and GPU!)
 *    - GPU execution units: 96 EUs with SIMD (good for parallel math)
 *    - Memory controller coordinates CPU, GPU, and display engine access
 *    - Upload once in init(), reuse every frame to minimize transfers
 *
 * ---
 *
 * ## Mental Model
 *
 * Think of it like a restaurant:
 *
 * - **You (application code)**: Customer placing an order
 * - **OpenGL**: Waiter taking your order to the kitchen
 * - **GPU Driver**: Kitchen manager organizing cooks
 * - **GPU**: Kitchen with 2560 cooks working in parallel
 * - **VRAM**: Kitchen's ingredient storage (fast access)
 * - **System RAM**: Grocery store (slower, must transport ingredients)
 * - **Monitor**: Serving the finished dish to the customer
 *
 * You don't cook the food yourself - you describe what you want,
 * and the kitchen (GPU) does the actual work in parallel!
 *
 * ---
 *
 * ## See Also
 *
 * - @ref opengl_tutorial - OpenGL concepts in detail
 * - @ref imgui_tutorial - ImGui immediate mode GUI
 * - @ref opengl_imgui_integration - How they work together
 * - @ref architecture - Overall application architecture
 */
