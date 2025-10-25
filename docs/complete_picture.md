/**
 * @page complete_picture The Complete Picture: From Your Intent to Pixels on Screen
 *
 * @tableofcontents
 *
 * ## The Big Picture: What You Want vs What Actually Happens
 *
 * ### What You Think (High-Level Intent)
 *
 * ```
 * "I want to create a window with resizable, dockable panels
 *  that show telemetry data with nice fonts and respond to
 *  mouse clicks and drags."
 * ```
 *
 * ### What Actually Happens (3 Translation Layers)
 *
 * @dot "The Three Translation Layers"
 * digraph TranslationLayers {
 *     rankdir=TB;
 *     node [shape=box, style="rounded,filled", fontname="Arial"];
 *
 *     intent [label="YOUR INTENT\n\nPanels, fonts, mouse events,\nresizing, docking", fillcolor="#E8F5E9"];
 *     imgui [label="LAYER 1: ImGui\n\nTranslates intent to geometry\n(rectangles, text as triangles)", fillcolor="#FFF3E0"];
 *     opengl [label="LAYER 2: OpenGL\n\nTranslates geometry to GPU commands\n(VBO, VAO, draw calls)", fillcolor="#E3F2FD"];
 *     gpu [label="LAYER 3: GPU\n\nExecutes commands,\nfills framebuffer with RGBA", fillcolor="#FCE4EC"];
 *     monitor [label="MONITOR\n\nDisplays framebuffer", fillcolor="#F3E5F5"];
 *
 *     intent -> imgui [label="ImGui API"];
 *     imgui -> opengl [label="OpenGL API"];
 *     opengl -> gpu [label="GPU Driver"];
 *     gpu -> monitor [label="HDMI"];
 * }
 * @enddot
 *
 * ---
 *
 * ## Layer 0: Your Code (Human Intent)
 *
 * **What you write** (`src/gui/panels/telemetry_panel.cpp`):
 *
 * ```cpp
 * void TelemetryPanel::draw(SimulationState& state, Camera& camera) {
 *     // YOUR INTENT: "Show a window with telemetry data"
 *     ImGui::Begin("Telemetry");
 *
 *     // YOUR INTENT: "Display quaternion values in a nice format"
 *     ImGui::Text("Quaternion: %.3f, %.3f, %.3f, %.3f",
 *                 state.quaternion.w, state.quaternion.x,
 *                 state.quaternion.y, state.quaternion.z);
 *
 *     // YOUR INTENT: "Button that does something when clicked"
 *     if (ImGui::Button("Reset")) {
 *         state.quaternion = {1.0, 0.0, 0.0, 0.0};
 *     }
 *
 *     ImGui::End();
 * }
 * ```
 *
 * **What you're saying:**
 * - "Make a window titled 'Telemetry'"
 * - "Show some text with these values"
 * - "Make a button, tell me if it's clicked"
 *
 * **You DON'T specify:**
 * - Where each pixel goes
 * - What triangles to draw
 * - How to detect mouse clicks on the button
 * - What color each pixel should be
 *
 * ---
 *
 * ## Layer 1: ImGui (Intent → Geometry)
 *
 * **ImGui's job:** Convert your high-level intent into **geometry** (triangles, vertices, colors).
 *
 * @dot "ImGui Translation"
 * digraph ImGuiTranslation {
 *     rankdir=LR;
 *     node [shape=box, style="rounded,filled", fontname="Arial"];
 *
 *     subgraph cluster_input {
 *         label="Your Code";
 *         style=filled;
 *         color=lightgrey;
 *         begin [label="ImGui::Begin(\"Telemetry\")", fillcolor=white];
 *         text [label="ImGui::Text(\"Quaternion: ...\")", fillcolor=white];
 *         button [label="ImGui::Button(\"Reset\")", fillcolor=white];
 *         end [label="ImGui::End()", fillcolor=white];
 *     }
 *
 *     subgraph cluster_output {
 *         label="ImGui's Output (Geometry)";
 *         style=filled;
 *         color=lightblue;
 *         window_bg [label="Window background:\n2 triangles (rect)\n200×400 pixels", fillcolor=white];
 *         title_text [label="Title \"Telemetry\":\n60 triangles\n(10 letters × 6 tri/letter)", fillcolor=white];
 *         quat_text [label="Quaternion text:\n180 triangles\n(30 chars × 6 tri/char)", fillcolor=white];
 *         button_bg [label="Button background:\n2 triangles (rect)\n100×30 pixels", fillcolor=white];
 *         button_text [label="Button \"Reset\":\n30 triangles\n(5 letters × 6 tri/letter)", fillcolor=white];
 *     }
 *
 *     begin -> window_bg;
 *     begin -> title_text;
 *     text -> quat_text;
 *     button -> button_bg;
 *     button -> button_text;
 * }
 * @enddot
 *
 * ### ImGui Generates Vertices
 *
 * **For the window background:**
 * ```cpp
 * // ImGui internally creates:
 * ImDrawVert vertices[4] = {
 *     {pos: {10, 10},   uv: {0,0}, col: 0xF0000000},  // Top-left
 *     {pos: {210, 10},  uv: {1,0}, col: 0xF0000000},  // Top-right
 *     {pos: {210, 410}, uv: {1,1}, col: 0xF0000000},  // Bottom-right
 *     {pos: {10, 410},  uv: {0,1}, col: 0xF0000000},  // Bottom-left
 * };
 * ImDrawIdx indices[6] = {0, 1, 2, 0, 2, 3};  // 2 triangles
 * ```
 *
 * **For the text "Quaternion: 1.000, 0.000, 0.000, 0.000":**
 * ```cpp
 * // Each character = 1 quad = 2 triangles = 6 vertices
 * // 30 characters × 6 vertices = 180 vertices
 * ImDrawVert text_vertices[180];
 * // Each vertex has:
 * //   - Position: where on screen
 * //   - UV: which part of font texture to sample
 * //   - Color: text color
 * ```
 *
 * **For mouse click detection:**
 * ```cpp
 * // ImGui checks:
 * ImVec2 mouse_pos = ImGui::GetMousePos();  // e.g., (125, 350)
 * ImVec2 button_min = {100, 340};
 * ImVec2 button_max = {200, 370};
 *
 * if (mouse_pos.x >= button_min.x && mouse_pos.x <= button_max.x &&
 *     mouse_pos.y >= button_min.y && mouse_pos.y <= button_max.y &&
 *     ImGui::IsMouseClicked(0)) {
 *     return true;  // Button was clicked!
 * }
 * ```
 *
 * ### ImGui's Output
 *
 * **ImGui produces a DrawList:**
 * ```
 * ImDrawData {
 *     total_vertices: 274 (window + title + text + button + button text)
 *     total_indices: 822 (274 vertices / 3 = 91 triangles × 3 = 273... rounded)
 *
 *     DrawList 0: {
 *         VtxBuffer: [vertex0, vertex1, ..., vertex273]
 *         IdxBuffer: [0, 1, 2, 3, 4, 5, ..., 820, 821]
 *         CmdBuffer: [
 *             {texture: font_atlas, idx_count: 822, clip_rect: (0,0,1920,1080)}
 *         ]
 *     }
 * }
 * ```
 *
 * **This is still CPU-side data!** Just an array of vertices and indices.
 *
 * ---
 *
 * ## Layer 2: OpenGL (Geometry → GPU Commands)
 *
 * **OpenGL's job:** Translate ImGui's geometry into **GPU commands** (VBO, VAO, draw calls).
 *
 * @dot "OpenGL Translation"
 * digraph OpenGLTranslation {
 *     rankdir=TB;
 *     node [shape=box, style="rounded,filled", fontname="Arial"];
 *
 *     imgui_data [label="ImGui DrawList\n\n274 vertices\n822 indices", fillcolor="#FFF3E0"];
 *
 *     subgraph cluster_opengl {
 *         label="OpenGL Backend\n(imgui_impl_opengl3.cpp)";
 *         style=filled;
 *         color=lightblue;
 *
 *         vbo_upload [label="glBufferData(VBO)\n\nUpload 274 vertices\nto GPU memory", fillcolor=white];
 *         ebo_upload [label="glBufferData(EBO)\n\nUpload 822 indices\nto GPU memory", fillcolor=white];
 *         vao_setup [label="glVertexAttribPointer\n\nDescribe format:\npos=2 floats, uv=2 floats,\ncol=4 bytes", fillcolor=white];
 *         draw_call [label="glDrawElements()\n\nDraw 274 triangles", fillcolor=white];
 *     }
 *
 *     gpu_cmd [label="GPU Command Queue\n\nDRAW_INDEXED\nvbo_id=42, ebo_id=17\ncount=822", fillcolor="#E3F2FD"];
 *
 *     imgui_data -> vbo_upload;
 *     imgui_data -> ebo_upload;
 *     vbo_upload -> vao_setup;
 *     ebo_upload -> vao_setup;
 *     vao_setup -> draw_call;
 *     draw_call -> gpu_cmd;
 * }
 * @enddot
 *
 * ### OpenGL Backend Code
 *
 * **File:** `external/imgui/backends/imgui_impl_opengl3.cpp`
 *
 * ```cpp
 * void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data) {
 *     for (int n = 0; n < draw_data->CmdListsCount; n++) {
 *         const ImDrawList* cmd_list = draw_data->CmdLists[n];
 *
 *         // STEP 1: Upload vertices to GPU (VBO)
 *         glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
 *         glBufferData(GL_ARRAY_BUFFER,
 *                      cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
 *                      cmd_list->VtxBuffer.Data,  // ImGui's vertex array
 *                      GL_STREAM_DRAW);  // Changes every frame
 *
 *         // STEP 2: Upload indices to GPU (EBO)
 *         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
 *         glBufferData(GL_ELEMENT_ARRAY_BUFFER,
 *                      cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
 *                      cmd_list->IdxBuffer.Data,  // ImGui's index array
 *                      GL_STREAM_DRAW);
 *
 *         // STEP 3: Describe format (VAO - already set up once)
 *         // Position: 2 floats at offset 0
 *         // UV: 2 floats at offset 8
 *         // Color: 4 bytes at offset 16
 *
 *         // STEP 4: Issue draw command
 *         for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
 *             const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
 *
 *             glBindTexture(GL_TEXTURE_2D, pcmd->TextureId);  // Font atlas
 *             glDrawElements(GL_TRIANGLES,
 *                            pcmd->ElemCount,  // 822 indices
 *                            GL_UNSIGNED_SHORT,
 *                            (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
 *         }
 *     }
 * }
 * ```
 *
 * ### What OpenGL Actually Does
 *
 * **VBO (Vertex Buffer Object):**
 * - Allocates GPU memory (system RAM for integrated GPU)
 * - Copies vertex data from CPU to GPU memory region
 * - Just storage, like copying a file to a USB drive
 *
 * **EBO (Element Buffer Object):**
 * - Stores index data (which vertices form triangles)
 * - `indices = [0, 1, 2]` means "triangle from vertex 0, 1, 2"
 *
 * **VAO (Vertex Array Object):**
 * - Describes the format: "First 8 bytes = position, next 8 = UV, next 4 = color"
 * - Like a struct definition for the vertex data
 *
 * **glDrawElements():**
 * - Sends command to GPU: "Draw triangles using VBO/EBO/VAO"
 * - This is the actual rendering command!
 *
 * ### The "JSON-like File" You Mentioned
 *
 * **You're exactly right!** OpenGL creates a command structure:
 *
 * ```json
 * {
 *   "command": "DRAW_INDEXED_TRIANGLES",
 *   "vertex_buffer": {
 *     "id": 42,
 *     "format": {
 *       "position": {"type": "float", "count": 2, "offset": 0},
 *       "uv": {"type": "float", "count": 2, "offset": 8},
 *       "color": {"type": "ubyte", "count": 4, "offset": 16}
 *     },
 *     "data": [
 *       {"position": [10, 10], "uv": [0, 0], "color": [240, 0, 0, 0]},
 *       {"position": [210, 10], "uv": [1, 0], "color": [240, 0, 0, 0]},
 *       ...
 *     ]
 *   },
 *   "index_buffer": {
 *     "id": 17,
 *     "data": [0, 1, 2, 0, 2, 3, ...]
 *   },
 *   "texture": {"id": 5, "type": "font_atlas"},
 *   "shader": {"id": 3, "type": "imgui_shader"},
 *   "count": 822
 * }
 * ```
 *
 * **GPU driver reads this "JSON" and executes it.**
 *
 * ---
 *
 * ## Layer 3: GPU (Commands → Pixels)
 *
 * **GPU's job:** Execute OpenGL commands and **fill framebuffer with RGBA values**.
 *
 * @dot "GPU Execution"
 * digraph GPUExecution {
 *     rankdir=TB;
 *     node [shape=box, style="rounded,filled", fontname="Arial"];
 *
 *     cmd [label="GPU Command\n\nDRAW_INDEXED\n822 indices\n274 triangles", fillcolor="#E3F2FD"];
 *
 *     subgraph cluster_gpu {
 *         label="GPU Execution Units";
 *         style=filled;
 *         color=pink;
 *
 *         vertex_shader [label="Vertex Shader\n\nRuns 274 times\n(once per vertex)\n\nTransforms positions", fillcolor=white];
 *         rasterizer [label="Rasterizer\n\nFinds ~50,000 pixels\ninside 274 triangles", fillcolor=white];
 *         fragment_shader [label="Fragment Shader\n\nRuns 50,000 times\n(once per pixel)\n\nSamples font texture\nOutputs RGBA", fillcolor=white];
 *     }
 *
 *     framebuffer [label="Framebuffer\n\n50,000 pixels updated\nwith RGBA values", fillcolor="#FCE4EC"];
 *
 *     cmd -> vertex_shader;
 *     vertex_shader -> rasterizer;
 *     rasterizer -> fragment_shader;
 *     fragment_shader -> framebuffer;
 * }
 * @enddot
 *
 * ### GPU Pipeline Steps
 *
 * **Step 1: Vertex Shader** (runs 274 times, once per vertex)
 *
 * ```glsl
 * // ImGui's vertex shader
 * layout (location = 0) in vec2 Position;
 * layout (location = 1) in vec2 UV;
 * layout (location = 2) in vec4 Color;
 *
 * uniform mat4 ProjMtx;  // Orthographic projection
 *
 * out vec2 Frag_UV;
 * out vec4 Frag_Color;
 *
 * void main() {
 *     // Transform position to screen space
 *     gl_Position = ProjMtx * vec4(Position.xy, 0, 1);
 *     Frag_UV = UV;
 *     Frag_Color = Color;
 * }
 * ```
 *
 * **What happens:**
 * - Vertex 0: position (10, 10) → screen position
 * - Vertex 1: position (210, 10) → screen position
 * - ... 272 more vertices
 *
 * **Step 2: Rasterizer** (GPU hardware)
 *
 * ```
 * For each of 274 triangles:
 *   - Find ALL pixels inside the triangle
 *   - Interpolate UV and Color for each pixel
 *
 * Example triangle: vertices at (10, 10), (210, 10), (210, 410)
 *   → Covers ~40,000 pixels
 *
 * Output: List of 50,000 pixels to process
 * ```
 *
 * **Step 3: Fragment Shader** (runs 50,000 times, once per pixel)
 *
 * ```glsl
 * // ImGui's fragment shader
 * in vec2 Frag_UV;
 * in vec4 Frag_Color;
 *
 * uniform sampler2D Texture;  // Font atlas
 *
 * out vec4 Out_Color;
 *
 * void main() {
 *     // Sample texture at UV coordinate
 *     vec4 tex_color = texture(Texture, Frag_UV.st);
 *
 *     // Multiply by vertex color
 *     Out_Color = Frag_Color * tex_color;
 *
 *     // GPU writes this to framebuffer[pixel_x][pixel_y]
 * }
 * ```
 *
 * **What happens:**
 * - Pixel at (125, 350): Sample texture, output RGBA → framebuffer[350][125]
 * - Pixel at (126, 350): Sample texture, output RGBA → framebuffer[350][126]
 * - ... 49,998 more pixels
 *
 * ### Result: Framebuffer Filled
 *
 * ```
 * framebuffer[10][10] = {240, 0, 0, 0}     ← Window background
 * framebuffer[11][10] = {240, 0, 0, 0}     ← Window background
 * ...
 * framebuffer[30][15] = {255, 255, 255, 255}  ← Letter 'T' in "Telemetry"
 * framebuffer[31][15] = {255, 255, 255, 255}  ← Letter 'T'
 * ...
 * framebuffer[125][350] = {100, 150, 255, 255}  ← Button background
 * ...
 * ```
 *
 * **Now the framebuffer has pixel colors!**
 *
 * ---
 *
 * ## Layer 4: Display Engine → Monitor
 *
 * @dot "Display Path"
 * digraph DisplayPath {
 *     rankdir=LR;
 *     node [shape=box, style="rounded,filled", fontname="Arial"];
 *
 *     framebuffer [label="Framebuffer\n(in RAM)\n\n1920×1080 pixels\n8 MB of RGBA data", fillcolor="#FCE4EC"];
 *     display_engine [label="Display Engine\n(on CPU die)\n\nReads at 60 Hz\nConverts to signal", fillcolor="#F3E5F5"];
 *     hdmi [label="HDMI Cable\n\nCarries digital\npixel stream", fillcolor=white, shape=ellipse];
 *     monitor [label="Monitor\n\nLights up LEDs/LCD\nbased on RGBA", fillcolor="#E8EAF6"];
 *
 *     framebuffer -> display_engine [label="Memory\nread"];
 *     display_engine -> hdmi [label="Signal"];
 *     hdmi -> monitor [label="Pixels"];
 * }
 * @enddot
 *
 * **Every 16.67ms (60 Hz):**
 * 1. Display engine reads entire framebuffer (2,073,600 pixels)
 * 2. Converts to HDMI signal
 * 3. Sends to monitor
 * 4. Monitor updates physical pixels
 *
 * ---
 *
 * ## The Complete Flow (All Layers)
 *
 * @dot "Complete System Flow"
 * digraph CompleteFlow {
 *     rankdir=TB;
 *     node [shape=box, style="rounded,filled", fontname="Arial", fontsize=10];
 *
 *     subgraph cluster_app {
 *         label="Your Application Code";
 *         style=filled;
 *         color=lightgreen;
 *         app_code [label="ImGui::Begin(\"Telemetry\")\nImGui::Text(...)\nImGui::Button(...)", fillcolor=white];
 *     }
 *
 *     subgraph cluster_imgui {
 *         label="ImGui Library (CPU)";
 *         style=filled;
 *         color=lightyellow;
 *         imgui_layout [label="Calculate layout\n(where things go)", fillcolor=white];
 *         imgui_input [label="Check mouse/keyboard\n(is button clicked?)", fillcolor=white];
 *         imgui_geometry [label="Generate geometry\n(vertices, indices)", fillcolor=white];
 *     }
 *
 *     subgraph cluster_opengl {
 *         label="OpenGL Backend (CPU)";
 *         style=filled;
 *         color=lightblue;
 *         opengl_upload [label="Upload to VBO/EBO", fillcolor=white];
 *         opengl_vao [label="Set VAO format", fillcolor=white];
 *         opengl_draw [label="glDrawElements()", fillcolor=white];
 *     }
 *
 *     subgraph cluster_gpu {
 *         label="GPU Hardware";
 *         style=filled;
 *         color=pink;
 *         gpu_vertex [label="Vertex Shader\n(position transform)", fillcolor=white];
 *         gpu_raster [label="Rasterizer\n(find pixels)", fillcolor=white];
 *         gpu_fragment [label="Fragment Shader\n(compute RGBA)", fillcolor=white];
 *         gpu_fb [label="Write to framebuffer", fillcolor=white];
 *     }
 *
 *     display [label="Display Engine → Monitor", fillcolor=lightgray];
 *
 *     app_code -> imgui_layout;
 *     imgui_layout -> imgui_input;
 *     imgui_input -> imgui_geometry;
 *     imgui_geometry -> opengl_upload;
 *     opengl_upload -> opengl_vao;
 *     opengl_vao -> opengl_draw;
 *     opengl_draw -> gpu_vertex;
 *     gpu_vertex -> gpu_raster;
 *     gpu_raster -> gpu_fragment;
 *     gpu_fragment -> gpu_fb;
 *     gpu_fb -> display;
 * }
 * @enddot
 *
 * ---
 *
 * ## Summary: Your Understanding is Correct!
 *
 * ### You Said:
 *
 * > "We need to tell the OS we are going to render some data.
 * > That data is going to have a main window with transparent background
 * > and resizable, dockable panels with mouse events."
 *
 * **✓ CORRECT!** This is handled by:
 * - **GLFW**: Creates the OS window
 * - **ImGui**: Handles panels, docking, mouse events, layout
 *
 * ### You Said:
 *
 * > "Now we need to convert all this to actual pixels the monitor can see.
 * > ImGui gives panels and mouse events and makes it easy,
 * > but it's not OS/hardware/GPU compatible yet."
 *
 * **✓ CORRECT!** ImGui produces geometry (vertex arrays), not pixels.
 *
 * ### You Said:
 *
 * > "Then OpenGL comes and VAO, VBO, and creates one big JSON-like file.
 * > GPU parses vertex/color and creates the canvas to send to HDMI."
 *
 * **✓ EXACTLY RIGHT!** OpenGL creates a command structure (like JSON),
 * GPU executes it and fills framebuffer, display engine sends to monitor.
 *
 * ---
 *
 * ## The Roles
 *
 * | Component | Role | Analogy |
 * |-----------|------|---------|
 * | **Your Code** | Express intent | Restaurant customer ordering |
 * | **ImGui** | Intent → Geometry | Waiter writing order |
 * | **OpenGL** | Geometry → GPU commands | Kitchen manager organizing |
 * | **GPU** | Commands → Pixels | Chefs cooking in parallel |
 * | **Framebuffer** | Store pixels | Serving platter |
 * | **Display Engine** | Pixels → Signal | Server delivering food |
 * | **Monitor** | Display pixels | Customer eating |
 *
 * **You never touch pixels directly. You express intent, libraries translate it down to hardware.**
 *
 * ---
 *
 * ## See Also
 *
 * - @ref pixel_truth - Everything is just computing RGBA
 * - @ref display_fundamentals - How pixels get to monitor
 * - @ref hardware_pipeline - CPU/GPU hardware details
 * - @ref opengl_tutorial - OpenGL API deep dive
 * - @ref imgui_tutorial - ImGui API deep dive
 * - @ref opengl_imgui_integration - How they work together
 */
