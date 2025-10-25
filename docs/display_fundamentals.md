/**
 * @page display_fundamentals Display Fundamentals: Pixels, Framebuffers, and the OS
 *
 * @tableofcontents
 *
 * Understanding graphics requires understanding the **fundamental problem**:
 * How do we put colored pixels on a monitor?
 *
 * This guide explains the complete picture from the lowest level (physical pixels)
 * to the highest level (your application drawing a rectangle).
 *
 * ---
 *
 * ## Step 0: The Physical Monitor (The Target)
 *
 * ### What is a Monitor?
 *
 * A monitor is a **grid of physical pixels** (light-emitting elements).
 *
 * ```
 * 1920×1080 monitor = 2,073,600 individual pixels arranged in a grid:
 *
 *   Column:  0      1      2      3  ...  1919
 *          ┌──────┬──────┬──────┬──────────┐
 * Row 0:   │Pixel │Pixel │Pixel │...       │
 *          ├──────┼──────┼──────┼──────────┤
 * Row 1:   │Pixel │Pixel │Pixel │...       │
 *          ├──────┼──────┼──────┼──────────┤
 * Row 2:   │Pixel │Pixel │Pixel │...       │
 *          │  ...   ...    ...              │
 * Row 1079:│Pixel │Pixel │Pixel │...       │
 *          └──────┴──────┴──────┴──────────┘
 * ```
 *
 * Each pixel has **3 sub-pixels**: Red, Green, Blue (RGB).
 *
 * Each sub-pixel can have brightness 0-255:
 * - (255, 0, 0) = bright red
 * - (0, 255, 0) = bright green
 * - (255, 255, 255) = white
 * - (0, 0, 0) = black
 *
 * ### How Does the Monitor Get Pixel Data?
 *
 * The monitor is **completely dumb**. It just displays whatever pixel values it receives.
 *
 * ```
 * DisplayPort/HDMI cable sends a stream of pixel values 60 times per second:
 *
 * Frame 1 (16.67ms duration):
 *   Pixel[0,0] = (255, 0, 0)      ← Red
 *   Pixel[0,1] = (255, 0, 0)      ← Red
 *   Pixel[0,2] = (255, 0, 0)      ← Red
 *   ... 2,073,597 more pixels
 *
 * Frame 2 (next 16.67ms):
 *   Pixel[0,0] = (0, 255, 0)      ← Green (changed!)
 *   Pixel[0,1] = (0, 255, 0)      ← Green
 *   Pixel[0,2] = (0, 255, 0)      ← Green
 *   ... 2,073,597 more pixels
 * ```
 *
 * **The monitor has no memory of what it displayed before.**
 * You must send all 2+ million pixel values EVERY frame (60 times per second)!
 *
 * ---
 *
 * ## Step 1: The Framebuffer (2D Canvas in Memory)
 *
 * ### What is a Framebuffer?
 *
 * A **framebuffer** is a **2D array in RAM** that holds pixel values for one frame.
 *
 * ```
 * For 1920×1080 display:
 *
 * Framebuffer in memory (system RAM for integrated GPU):
 *   Size = 1920 × 1080 × 4 bytes per pixel = 8,294,400 bytes (~8 MB)
 *
 *   Memory layout (linear array):
 *   Address 0x80000000: [R, G, B, A] ← Pixel (0, 0)
 *   Address 0x80000004: [R, G, B, A] ← Pixel (1, 0)
 *   Address 0x80000008: [R, G, B, A] ← Pixel (2, 0)
 *   ...
 *   Address 0x807E8000: [R, G, B, A] ← Pixel (1919, 1079) - last pixel
 * ```
 *
 * **This is the 2D canvas you asked about!**
 * - One framebuffer per monitor
 * - Stores RGBA for every pixel
 * - Lives in system RAM (for integrated GPU) or VRAM (for discrete GPU)
 *
 * ### Who Writes to the Framebuffer?
 *
 * **The GPU execution units write to it, pixel by pixel.**
 *
 * When the GPU runs your fragment shader, it writes the output color to the framebuffer:
 *
 * ```glsl
 * // Fragment shader runs ONCE per pixel
 * void main() {
 *     // Calculate color for this pixel
 *     vec4 color = vec4(1.0, 0.0, 0.0, 1.0);  // Red
 *
 *     // Output goes to framebuffer at this pixel's position
 *     FragColor = color;
 *     // GPU writes this to: framebuffer[pixel_x, pixel_y] = (255, 0, 0, 255)
 * }
 * ```
 *
 * **The GPU fills the framebuffer pixel-by-pixel** based on your shaders!
 *
 * ### How Does the Monitor Get the Framebuffer?
 *
 * **Display Engine** (hardware on the CPU die) scans the framebuffer and sends it to the monitor:
 *
 * ```
 * Display Engine (60 Hz refresh):
 *   Every 16.67ms:
 *     1. Read framebuffer from RAM (all 8 MB)
 *     2. Convert to DisplayPort/HDMI signal
 *     3. Send to monitor pixel-by-pixel:
 *        Row 0: send pixels 0-1919
 *        Row 1: send pixels 0-1919
 *        ...
 *        Row 1079: send pixels 0-1919
 *     4. Monitor displays the frame
 *     5. Repeat
 * ```
 *
 * **The monitor just receives a stream of pixels - it has no idea what they represent!**
 *
 * ---
 *
 * ## Step 2: How to Put One Pixel on Screen
 *
 * **You DON'T write code to set individual pixels** (usually).
 *
 * Instead:
 * 1. **You draw geometry** (triangles with vertices)
 * 2. **GPU rasterizes** (converts triangles to pixels)
 * 3. **GPU runs fragment shader** for each pixel
 * 4. **Fragment shader outputs color** to framebuffer
 *
 * ### Example: Drawing One Red Pixel at (500, 300)
 *
 * **Method 1: Draw a tiny triangle covering that pixel**
 *
 * ```cpp
 * // Create a triangle that covers pixel (500, 300)
 * float vertices[] = {
 *     500.0f, 300.0f,  // Vertex 0
 *     501.0f, 300.0f,  // Vertex 1
 *     500.0f, 301.0f,  // Vertex 2
 * };
 *
 * // Upload to GPU
 * glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 *
 * // Draw
 * glDrawArrays(GL_TRIANGLES, 0, 3);
 * ```
 *
 * **What happens**:
 * ```
 * 1. GPU receives 3 vertices: (500, 300), (501, 300), (500, 301)
 * 2. Rasterizer finds pixels inside triangle: just (500, 300)
 * 3. Fragment shader runs ONCE for pixel (500, 300)
 * 4. Shader outputs red: (1.0, 0.0, 0.0, 1.0)
 * 5. GPU writes to framebuffer[500, 300] = (255, 0, 0, 255)
 * 6. Display engine sends this to monitor
 * 7. Monitor lights up pixel at row 300, column 500 in red
 * ```
 *
 * **Method 2: Use a library like ImGui**
 *
 * ```cpp
 * // ImGui generates the triangle vertices for you
 * ImDrawList* draw_list = ImGui::GetWindowDrawList();
 * draw_list->AddRectFilled(ImVec2(500, 300), ImVec2(501, 301), IM_COL32(255, 0, 0, 255));
 *
 * // ImGui backend uploads to GPU and draws
 * ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
 * ```
 *
 * **You NEVER write**: `framebuffer[500][300] = red;` directly.
 *
 * You always go through: **Geometry → Rasterization → Fragment Shader → Framebuffer**
 *
 * ---
 *
 * ## Step 3: Drawing a Rectangle (High-Level to Low-Level)
 *
 * Let's trace: "Draw a green rectangle at (100, 100) with size 200×150"
 *
 * ### Your Code (High-Level)
 *
 * ```cpp
 * // Option 1: Pure OpenGL
 * float vertices[] = {
 *     // Rectangle = 2 triangles = 6 vertices
 *     100.0f, 100.0f,  0.0f, 1.0f, 0.0f,  // Bottom-left, green
 *     300.0f, 100.0f,  0.0f, 1.0f, 0.0f,  // Bottom-right, green
 *     300.0f, 250.0f,  0.0f, 1.0f, 0.0f,  // Top-right, green
 *     100.0f, 100.0f,  0.0f, 1.0f, 0.0f,  // Bottom-left, green
 *     300.0f, 250.0f,  0.0f, 1.0f, 0.0f,  // Top-right, green
 *     100.0f, 250.0f,  0.0f, 1.0f, 0.0f,  // Top-left, green
 * };
 *
 * glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 * glDrawArrays(GL_TRIANGLES, 0, 6);
 * ```
 *
 * ```cpp
 * // Option 2: ImGui (easier)
 * ImGui::GetBackgroundDrawList()->AddRectFilled(
 *     ImVec2(100, 100),
 *     ImVec2(300, 250),
 *     IM_COL32(0, 255, 0, 255)  // Green
 * );
 * ```
 *
 * ### GPU Processing (Low-Level)
 *
 * ```
 * Step 1: Vertex Shader
 *   Process 6 vertices (2 triangles):
 *     Vertex 0: (100, 100) → screen position
 *     Vertex 1: (300, 100) → screen position
 *     ... 4 more vertices
 *
 * Step 2: Rasterizer
 *   Find ALL pixels inside the 2 triangles:
 *     Triangle 1: (100,100), (300,100), (300,250)
 *       → Covers pixels: (100,100), (101,100), (102,100), ..., (299,249)
 *     Triangle 2: (100,100), (300,250), (100,250)
 *       → Covers pixels: (100,100), (101,100), ..., (100,249)
 *
 *   Total pixels to color: 200 × 150 = 30,000 pixels
 *
 * Step 3: Fragment Shader (runs 30,000 times in parallel!)
 *   For each pixel (e.g., pixel at 150, 175):
 *     vec4 color = vec4(0.0, 1.0, 0.0, 1.0);  // Green
 *     FragColor = color;
 *     // GPU writes: framebuffer[150, 175] = (0, 255, 0, 255)
 *
 * Step 4: Framebuffer Now Contains
 *   framebuffer[100][100] = (0, 255, 0, 255)  ← Green
 *   framebuffer[101][100] = (0, 255, 0, 255)  ← Green
 *   framebuffer[102][100] = (0, 255, 0, 255)  ← Green
 *   ... 29,997 more green pixels
 *
 * Step 5: Display Engine
 *   Scans framebuffer at 60 Hz
 *   Sends all 2,073,600 pixels to monitor via HDMI
 *   Monitor displays the green rectangle
 * ```
 *
 * **You give high-level commands (rectangle vertices).**
 * **GPU fills in all the pixels automatically.**
 *
 * ---
 *
 * ## Step 4: The Operating System's Role (Window Manager)
 *
 * You asked: "How does the OS handle multiple windows and track mouse clicks?"
 *
 * ### The Window Manager (OS Component)
 *
 * On Linux: X11, Wayland
 * On Windows: Desktop Window Manager (DWM)
 * On macOS: Quartz Compositor
 *
 * **The window manager is a program that:**
 * 1. Maintains a list of all application windows
 * 2. Decides where each window is positioned on screen
 * 3. Composites (combines) all windows into ONE final framebuffer
 * 4. Routes input events (mouse, keyboard) to the correct application
 *
 * ### Example: Two Applications Running
 *
 * ```
 * Application A: Web browser window at (0, 0), size 800×600
 * Application B: Our AeroDynControlRig at (400, 200), size 1200×800
 *
 * Each application has its OWN framebuffer (off-screen):
 *   Browser framebuffer: 800×600 pixels
 *   Our app framebuffer:  1200×800 pixels
 *
 * Window Manager (compositor):
 *   1. Reads browser framebuffer (800×600)
 *   2. Reads our app framebuffer (1200×800)
 *   3. Composites them into FINAL framebuffer (1920×1080):
 *
 *      Final framebuffer:
 *        Copy browser pixels to region (0, 0) → (800, 600)
 *        Copy our app pixels to region (400, 200) → (1600, 1000)
 *        Fill rest with desktop background
 *
 *   4. Display engine sends FINAL framebuffer to monitor
 * ```
 *
 * **Each application renders to its OWN off-screen framebuffer.**
 * **Window manager combines them into the final screen framebuffer.**
 *
 * ### How Our Application Works (Off-Screen Rendering)
 *
 * **File**: `src/app/application.cpp`
 *
 * ```cpp
 * void Application::init() {
 *     // Create our OWN framebuffer (off-screen, not the screen!)
 *     glGenFramebuffers(1, &fbo);
 *     glBindFramebuffer(GL_FRAMEBUFFER, fbo);
 *
 *     // Create texture to hold rendered image (1200×800)
 *     glGenTextures(1, &fboTexture);
 *     glBindTexture(GL_TEXTURE_2D, fboTexture);
 *     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1200, 800, ...);
 *
 *     // Attach texture to framebuffer
 *     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
 * }
 *
 * void Application::render3D() {
 *     // Render to OUR framebuffer (not screen!)
 *     glBindFramebuffer(GL_FRAMEBUFFER, fbo);
 *     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 *
 *     // Draw 3D cube
 *     renderer.draw(state);
 *     // GPU writes pixels to OUR framebuffer texture (1200×800)
 *
 *     // Unbind (go back to screen framebuffer)
 *     glBindFramebuffer(GL_FRAMEBUFFER, 0);
 * }
 *
 * void Application::renderUI() {
 *     // Now render to SCREEN framebuffer (managed by GLFW/OS)
 *     ImGui::Begin("Viewport");
 *     ImGui::Image((void*)(intptr_t)fboTexture, ImVec2(1200, 800));
 *     // This displays our off-screen framebuffer as a texture in the UI
 *     ImGui::End();
 *
 *     // Render ImGui panels
 *     for (auto& panel : panels) {
 *         panel->draw(state, camera);
 *     }
 *
 *     // ImGui generates geometry, GPU writes to screen framebuffer
 *     ImGui::Render();
 *     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
 * }
 *
 * void Application::tick() {
 *     render3D();   // → Our off-screen framebuffer
 *     renderUI();   // → Screen framebuffer (shown in OS window)
 *
 *     glfwSwapBuffers(window);
 *     // OS window manager reads our window's framebuffer
 *     // Composites it with other windows
 *     // Sends final result to monitor
 * }
 * ```
 *
 * ---
 *
 * ## Step 5: Mouse Click Tracking (OS + Application)
 *
 * ### Who Tracks Mouse Position?
 *
 * **Hardware → OS Kernel → Window Manager → Application**
 *
 * ```
 * 1. Mouse Hardware (USB device)
 *    Sends: "I moved +5 pixels right, +3 pixels down"
 *
 * 2. OS Kernel (USB driver)
 *    Maintains global cursor position:
 *      cursor_x = 734
 *      cursor_y = 412
 *    Updates: cursor_x += 5, cursor_y += 3
 *    New position: (739, 415)
 *
 * 3. Window Manager (compositor)
 *    Knows all window positions:
 *      Browser window: (0, 0) → (800, 600)
 *      Our app window:  (400, 200) → (1600, 1000)
 *
 *    Cursor is at (739, 415) globally.
 *
 *    Which window is that in?
 *      739 is between 400 and 1600 ✓
 *      415 is between 200 and 1000 ✓
 *      → Cursor is in our app window!
 *
 *    Convert to window-local coordinates:
 *      local_x = 739 - 400 = 339
 *      local_y = 415 - 200 = 215
 *
 *    If mouse is clicked:
 *      Send event to our app: "Mouse clicked at (339, 215)"
 *
 * 4. Our Application (receives event via GLFW)
 *    void Application::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
 *        double xpos, ypos;
 *        glfwGetCursorPos(window, &xpos, &ypos);
 *        // xpos = 339, ypos = 215 (window-local coordinates)
 *
 *        // ImGui checks if click is on a button
 *        ImGuiIO& io = ImGui::GetIO();
 *        io.MousePos = ImVec2(xpos, ypos);
 *
 *        if (ImGui::IsMouseClicked(0)) {
 *            // ImGui internally checks: is cursor over a button?
 *            // Button bounds: (50, 200) → (150, 230)
 *            // Cursor: (339, 215) → NOT in button
 *        }
 *    }
 * ```
 *
 * ### Where is Mouse Position Stored?
 *
 * - **Kernel**: Global screen coordinates (updated by USB driver)
 * - **Window Manager**: Global coordinates + window hit testing
 * - **Application (GLFW)**: Window-local coordinates
 * - **ImGui**: Application-local coordinates + UI element hit testing
 *
 * **NOT in VRAM!** Mouse position is in **system RAM** (kernel memory, application memory).
 *
 * ### Visual Cursor
 *
 * The cursor image (arrow) is drawn by the **window manager**:
 *
 * ```
 * Window Manager's compositor loop (60 Hz):
 *   1. Composite all application windows → final framebuffer
 *   2. Draw cursor image at (cursor_x, cursor_y) ON TOP
 *      cursor_pixels = load_image("arrow.png")
 *      for each pixel in cursor_pixels:
 *          framebuffer[cursor_x + offset_x][cursor_y + offset_y] = cursor_pixel
 *   3. Send framebuffer to display engine → monitor
 * ```
 *
 * That's why the cursor is **always on top** of everything!
 *
 * ---
 *
 * ## Step 6: Multi-Monitor Support
 *
 * ### One Framebuffer Per Monitor
 *
 * ```
 * System with 2 monitors:
 *   Monitor 1: 1920×1080 @ (0, 0)
 *   Monitor 2: 1920×1080 @ (1920, 0)  ← To the right of monitor 1
 *
 * Two framebuffers in RAM:
 *   Framebuffer 1: 1920×1080 × 4 bytes = 8 MB
 *   Framebuffer 2: 1920×1080 × 4 bytes = 8 MB
 *
 * Window Manager:
 *   Window A is at global position (500, 100) → on Monitor 1
 *   Window B is at global position (2200, 300) → on Monitor 2
 *
 *   Composite Window A into Framebuffer 1
 *   Composite Window B into Framebuffer 2
 *
 * Display Engine:
 *   Send Framebuffer 1 to Monitor 1 via HDMI port 1
 *   Send Framebuffer 2 to Monitor 2 via HDMI port 2
 * ```
 *
 * **Each monitor has its own framebuffer.**
 * **Window manager decides which window goes to which framebuffer.**
 *
 * ---
 *
 * ## Summary: The Complete Flow
 *
 * ```
 * ┌─────────────────────────────────────────────────────────────┐
 * │ YOUR APPLICATION                                            │
 * │  "Draw a green rectangle from (100,100) to (300,250)"      │
 * │                                                             │
 * │  Code:                                                      │
 * │    float vertices[] = { ... };  ← YOU create geometry      │
 * │    glBufferData(..., vertices);  ← Upload to GPU           │
 * │    glDrawArrays(...);            ← Tell GPU to draw        │
 * └───────────────────────┬─────────────────────────────────────┘
 *                         │
 *                         ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │ GPU (EXECUTION UNITS)                                       │
 * │  1. Vertex Shader: Process 6 vertices (2 triangles)        │
 * │  2. Rasterizer: Find 30,000 pixels inside triangles        │
 * │  3. Fragment Shader: Run 30,000 times, output green        │
 * │  4. Write to YOUR app's framebuffer:                       │
 * │     framebuffer[100][100] = (0, 255, 0, 255)               │
 * │     framebuffer[101][100] = (0, 255, 0, 255)               │
 * │     ... 29,998 more pixels                                 │
 * └───────────────────────┬─────────────────────────────────────┘
 *                         │
 *                         ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │ APPLICATION FRAMEBUFFER (in RAM)                            │
 * │  1200×800 texture with your rendered content               │
 * │  [Contains 3D cube + UI panels]                            │
 * └───────────────────────┬─────────────────────────────────────┘
 *                         │
 *                         ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │ WINDOW MANAGER (OS Compositor)                              │
 * │  1. Read your app's framebuffer                            │
 * │  2. Read other apps' framebuffers                          │
 * │  3. Composite into SCREEN framebuffer (1920×1080):         │
 * │     - Browser window at (0, 0)                             │
 * │     - Your app window at (400, 200)                        │
 * │     - Desktop background                                   │
 * │     - Mouse cursor at (cursor_x, cursor_y)                 │
 * └───────────────────────┬─────────────────────────────────────┘
 *                         │
 *                         ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │ SCREEN FRAMEBUFFER (in RAM)                                 │
 * │  1920×1080 × 4 bytes = 8 MB                                │
 * │  [Final composited image with all windows + cursor]        │
 * └───────────────────────┬─────────────────────────────────────┘
 *                         │
 *                         ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │ DISPLAY ENGINE (on CPU die)                                 │
 * │  Reads screen framebuffer at 60 Hz                         │
 * │  Sends 2,073,600 pixel values via HDMI/DisplayPort        │
 * └───────────────────────┬─────────────────────────────────────┘
 *                         │ HDMI cable
 *                         ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │ MONITOR (Dumb Display)                                      │
 * │  Receives pixel stream, lights up LEDs/LCD                 │
 * │  Has NO memory, NO logic, just displays what it receives   │
 * └─────────────────────────────────────────────────────────────┘
 * ```
 *
 * ---
 *
 * ## Key Insights (Answers to Your Questions)
 *
 * ### 1. "How to put a pixel on a monitor?"
 *
 * You DON'T write individual pixel values. Instead:
 * - **You**: Create geometry (vertices of triangles)
 * - **GPU**: Rasterizes (figures out which pixels are inside)
 * - **Fragment Shader**: Runs for each pixel, outputs color
 * - **GPU**: Writes color to framebuffer
 * - **Display Engine**: Sends framebuffer to monitor
 *
 * ### 2. "Who is making the framebuffer pixel by pixel?"
 *
 * **The GPU's fragment shader** runs millions of times (once per pixel) and writes to the framebuffer.
 *
 * You write the shader logic, GPU executes it in parallel.
 *
 * ### 3. "Does the application have to give each pixel value?"
 *
 * **NO!** You give:
 * - Geometry (triangle vertices)
 * - Shader logic (how to color pixels)
 *
 * GPU fills in all the pixels automatically.
 *
 * ### 4. "Is the OS in VRAM?"
 *
 * **NO!** The window manager runs on the **CPU** using **system RAM**.
 *
 * VRAM (or GPU memory region) only stores:
 * - Vertex buffers (geometry)
 * - Textures (images)
 * - Framebuffers (pixel arrays)
 *
 * ### 5. "Does VRAM have a 2D canvas per monitor?"
 *
 * **YES!** Each monitor has a framebuffer (2D pixel array) in RAM.
 *
 * For integrated GPU: in system RAM.
 * For discrete GPU: in VRAM.
 *
 * ### 6. "Who tracks mouse position?"
 *
 * - **Kernel**: Global screen coordinates (all monitors)
 * - **Window Manager**: Determines which window is under cursor
 * - **Application**: Receives window-local coordinates
 * - **ImGui**: Checks if cursor is over a UI element
 *
 * NOT stored in VRAM - stored in system RAM (kernel + application memory).
 *
 * ### 7. "Is it like JSON API for web?"
 *
 * **Exactly!** You write high-level commands:
 * ```cpp
 * // Like HTTP request: "POST /api/rectangle {x:100, y:100, color:'green'}"
 * glDrawArrays(GL_TRIANGLES, 0, 6);  // "GPU, draw these triangles"
 * ```
 *
 * GPU driver translates to low-level GPU machine code.
 * GPU executes and fills pixels.
 *
 * ### 8. "Monitor is just a dumb display?"
 *
 * **YES!** Monitor receives a pixel stream 60 times per second.
 * It has no idea what application created it.
 * It just lights up pixels as told.
 *
 * ### 9. "Something tracks applications, windows, screens?"
 *
 * **The Window Manager** (OS component):
 * - X11, Wayland (Linux)
 * - Desktop Window Manager (Windows)
 * - Quartz Compositor (macOS)
 *
 * Runs on CPU, uses system RAM, composites all windows into final framebuffer.
 *
 * ---
 *
 * ## See Also
 *
 * - @ref hardware_pipeline - Complete RAM → GPU → Screen flow
 * - @ref opengl_tutorial - OpenGL concepts (VBO, VAO, FBO)
 * - @ref opengl_imgui_integration - Off-screen rendering technique
 * - @ref architecture - Application architecture overview
 */
