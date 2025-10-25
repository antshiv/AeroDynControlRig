/**
 * @page opengl_imgui_integration OpenGL + ImGui Integration
 * @brief How OpenGL and ImGui work together in our application
 *
 * @tableofcontents
 */

# OpenGL + ImGui Integration

## The Challenge

We want to:
1. Render a 3D scene with OpenGL (rotating cube)
2. Display interactive UI panels with ImGui
3. Have them coexist in the same window

**Problem**: OpenGL renders to the whole screen. How do we overlay ImGui?

**Solution**: **Framebuffer Objects (FBO)** + ImGui textures!

---

## Architecture Overview

@dot "Rendering Architecture"
digraph RenderingArchitecture {
    rankdir=TB;
    node [shape=box, style="rounded,filled", fontname="Arial"];

    // Frame start
    tick [label="Application::tick()", fillcolor=lightgreen, shape=box3d];

    // OpenGL rendering
    subgraph cluster_opengl {
        label="OpenGL 3D Rendering";
        style=filled;
        fillcolor=lightblue;

        bind_fbo [label="Bind FBO\n(off-screen)"];
        render_3d [label="Render 3D Scene\n(cube + axes)"];
        unbind_fbo [label="Unbind FBO\n(back to screen)"];
    }

    // Texture result
    texture [label="Render Texture\n(contains 3D scene)", shape=cylinder, fillcolor=lightyellow];

    // ImGui rendering
    subgraph cluster_imgui {
        label="ImGui UI Rendering";
        style=filled;
        fillcolor=lavender;

        imgui_newframe [label="ImGui::NewFrame()"];
        imgui_viewport [label="ImGui::Image(texture)"];
        imgui_panels [label="Draw UI Panels"];
        imgui_render [label="ImGui::Render()"];
        imgui_drawdata [label="Render Draw Data"];
    }

    // Output
    screen [label="Screen\n(final frame)", shape=box3d, fillcolor=lightgreen];

    // Flow
    tick -> bind_fbo;
    bind_fbo -> render_3d;
    render_3d -> unbind_fbo;
    unbind_fbo -> texture;

    texture -> imgui_newframe;
    imgui_newframe -> imgui_viewport;
    imgui_viewport -> imgui_panels;
    imgui_panels -> imgui_render;
    imgui_render -> imgui_drawdata;
    imgui_drawdata -> screen;

    // Swap buffers
    screen -> swap [label="glfwSwapBuffers()"];
    swap [label="Display Frame", shape=ellipse, fillcolor=gold];
}
@enddot

---

## Step-by-Step Integration

### Step 1: Initialize Both Systems

From `application.cpp init()`:

```cpp
bool Application::init() {
    // 1. Initialize GLFW and create window
    glfwInit();
    window = glfwCreateWindow(800, 600, "App", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // 2. Initialize GLEW (OpenGL functions)
    glewInit();

    // 3. Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 4. Initialize ImGui backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);   // GLFW input
    ImGui_ImplOpenGL3_Init("#version 330");       // OpenGL rendering

    // 5. Initialize OpenGL renderers
    renderer.init();        // 3D scene renderer
    axisRenderer.init();    // Axis gizmo

    return true;
}
```

**Order matters!** OpenGL context must exist before ImGui backends initialize.

---

### Step 2: Create Off-Screen Render Target

From `application.cpp ensureRenderTarget()`:

```cpp
bool Application::ensureRenderTarget(int width, int height) {
    // Only recreate if size changed
    if (sceneWidth == width && sceneHeight == height && fbo != 0) {
        return true;
    }

    // Cleanup old FBO if exists
    destroyRenderTarget();

    sceneWidth = width;
    sceneHeight = height;

    // 1. Create framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 2. Create color texture
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // Important: Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 3. Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, renderTexture, 0);

    // 4. Create depth buffer (for 3D depth testing)
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                          width, height);

    // 5. Attach depth buffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer);

    // 6. Check if complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete!" << std::endl;
        return false;
    }

    // 7. Unbind (back to default framebuffer = screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}
```

**Key Points**:
- **renderTexture**: OpenGL texture that will contain the 3D scene
- **depthBuffer**: Needed for 3D depth testing (Z-buffer)
- **FBO**: Container that links texture + depth buffer

---

### Step 3: Render 3D Scene to Texture

From `application.cpp renderSceneToTexture()`:

```cpp
ImTextureID Application::renderSceneToTexture(const ImVec2& size) {
    int width = static_cast<int>(size.x);
    int height = static_cast<int>(size.y);

    // Ensure FBO exists and is correct size
    if (!ensureRenderTarget(width, height)) {
        return nullptr;
    }

    // 1. Bind FBO (render to texture instead of screen)
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 2. Set viewport to texture size
    glViewport(0, 0, width, height);

    // 3. Clear texture (black background + clear depth)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // 5. Set up matrices
    renderer.setModelMatrix(glm::value_ptr(simulationState.model_matrix));
    renderer.setViewMatrix(camera.getViewMatrix());

    float aspect = width / static_cast<float>(height);
    renderer.setProjectionMatrix(camera.getProjectionMatrix(aspect));

    // 6. Render 3D geometry
    renderer.renderFrame3D(transform);
    axisRenderer.render3D(transform);

    // 7. Unbind FBO (back to screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 8. Return texture ID for ImGui to use
    return (ImTextureID)(intptr_t)renderTexture;
}
```

**What happens**:
1. OpenGL renders to `renderTexture` instead of screen
2. All geometry (cube + axes) drawn to this texture
3. Texture now contains a picture of the 3D scene

---

### Step 4: Display Texture in ImGui

From `application.cpp renderDashboardLayout()`:

```cpp
void Application::renderDashboardLayout(ImGuiIO& io) {
    // ... docking setup ...

    ImGui::Begin("3D Viewport");

    // Get available space in window
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    // Render 3D scene to texture
    ImTextureID sceneTexture = renderSceneToTexture(viewportSize);

    if (sceneTexture) {
        // Display texture as ImGui image
        ImGui::Image(
            sceneTexture,        // OpenGL texture ID
            viewportSize,        // Size to display
            ImVec2(0.0f, 1.0f),  // UV top-left (flipped Y)
            ImVec2(1.0f, 0.0f)   // UV bottom-right (flipped Y)
        );

        // Handle mouse interaction with viewport
        if (ImGui::IsItemHovered()) {
            // Mouse is over 3D viewport
            // Can handle camera controls here
        }
    }

    ImGui::End();
}
```

**UV Coordinates**: Flipped because OpenGL textures have Y=0 at bottom, but ImGui has Y=0 at top.

---

### Step 5: Render UI Panels

```cpp
void Application::renderDashboardLayout(ImGuiIO& io) {
    // Viewport window (showing 3D texture)
    ImGui::Begin("3D Viewport");
    ImTextureID tex = renderSceneToTexture(ImGui::GetContentRegionAvail());
    ImGui::Image(tex, ImGui::GetContentRegionAvail(), ...);
    ImGui::End();

    // Control panel (ImGui widgets)
    ImGui::Begin("Control Panel");
    if (ImGui::Button("Pause")) {
        simulationState.control.paused = !simulationState.control.paused;
    }
    ImGui::SliderFloat("Speed", &simulationState.control.time_scale, 0.1f, 5.0f);
    ImGui::End();

    // Telemetry panel
    ImGui::Begin("Telemetry");
    ImGui::Text("Quat: [%.3f, %.3f, %.3f, %.3f]",
                simulationState.quaternion[0],
                simulationState.quaternion[1],
                simulationState.quaternion[2],
                simulationState.quaternion[3]);
    ImGui::End();

    // ... more panels ...
}
```

---

### Step 6: Render ImGui Draw Data

From `application.cpp tick()`:

```cpp
void Application::tick() {
    // ... update simulation ...

    // 1. Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. Build UI (calls renderDashboardLayout)
    if (simulationState.control.use_legacy_ui) {
        renderLegacyLayout();
    } else {
        renderDashboardLayout(io);
    }

    // 3. Finalize ImGui rendering
    ImGui::Render();

    // 4. Render ImGui draw commands with OpenGL
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 5. Swap buffers (show frame)
    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

**What ImGui_ImplOpenGL3_RenderDrawData does**:
1. Iterates through ImGui draw commands
2. Uses its own OpenGL shader
3. Renders quads/triangles for UI
4. Applies textures (fonts, images)
5. Renders on top of anything already on screen

---

## Complete Frame Breakdown

Let's trace one frame in detail:

```
┌─────────────────────────────────────────────────────────────────┐
│ Frame N                                                         │
└─────────────────────────────────────────────────────────────────┘

1. glfwPollEvents()
   ├─► Process user input (mouse, keyboard)
   └─► GLFW callbacks update camera state

2. Update Simulation (if not paused)
   ├─► QuaternionDemoModule::update()
   │   └─► Updates simulationState.quaternion
   ├─► SensorSimulatorModule::update()
   │   └─► Updates simulationState.sensor
   └─► ... other modules

3. Render 3D Scene to Texture
   ├─► glBindFramebuffer(fbo)  ← Now rendering to texture
   ├─► glClear(...)            ← Clear texture
   ├─► glUseProgram(...)       ← Use 3D shader
   ├─► Set uniforms (model/view/projection)
   ├─► glBindVertexArray(vao)  ← Select cube geometry
   ├─► glDrawElements(...)     ← Draw cube
   ├─► Draw axis gizmo
   └─► glBindFramebuffer(0)    ← Back to screen

4. Build ImGui UI
   ├─► ImGui::NewFrame()
   ├─► ImGui::Begin("Viewport")
   │   ├─► ImGui::Image(renderTexture)  ← Show 3D scene
   │   └─► ImGui::End()
   ├─► ImGui::Begin("Control Panel")
   │   ├─► ImGui::Button("Pause")
   │   ├─► ImGui::SliderFloat(...)
   │   └─► ImGui::End()
   ├─► ... draw all panels
   └─► ImGui::Render()  ← Generate draw commands

5. Render ImGui to Screen
   ├─► glBindFramebuffer(0)  ← Render to screen
   ├─► ImGui_ImplOpenGL3_RenderDrawData()
   │   ├─► Use ImGui's shader
   │   ├─► Draw rectangles (panels, buttons, text)
   │   ├─► Apply textures (fonts, our renderTexture)
   │   └─► All rendered on top of screen

6. Present Frame
   ├─► glfwSwapBuffers()
   └─► Screen shows: 3D scene in viewport + UI panels

Loop back to step 1
```

---

## Handling Input

### Problem: Who Gets the Mouse?

When you click on a button, should the 3D camera move?

**No!** ImGui needs the click.

### Solution: Input Capture Flags

```cpp
// From application.cpp mouseCallback()

void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    // Check if ImGui wants the mouse
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;  // Don't process in 3D scene
    }

    // Process mouse for camera control
    app->camera.processMouseMovement(xOffset, yOffset);
}
```

**ImGui sets these flags**:
- `io.WantCaptureMouse` - Mouse is over UI
- `io.WantCaptureKeyboard` - Typing in text field
- `io.WantTextInput` - Text input active

**Always check these before processing input in your 3D scene!**

---

## Viewport-Specific Input

Our app uses a clever technique for viewport interaction:

```cpp
// From renderDashboardLayout()

// After ImGui::Image()
bool viewport_hovered = ImGui::IsItemHovered();
bool viewport_active = ImGui::IsItemActive();

if (viewport_active || viewport_hovered) {
    // Mouse is over the 3D viewport

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

        // Rotate camera
        camera.processMouseMovement(drag.x * sensitivity, -drag.y * sensitivity);

        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
        ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

        // Pan camera
        camera.pan(-drag.x * sensitivity, drag.y * sensitivity);

        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    }
}
```

**Result**: Camera only moves when dragging inside the viewport!

---

## Common Pitfalls

### 1. Texture Appears Upside-Down

**Problem**: OpenGL textures have Y=0 at bottom, ImGui has Y=0 at top.

**Solution**: Flip UV coordinates in `ImGui::Image()`:

```cpp
// Wrong
ImGui::Image(tex, size, ImVec2(0, 0), ImVec2(1, 1));

// Correct (flipped Y)
ImGui::Image(tex, size, ImVec2(0, 1), ImVec2(1, 0));
```

### 2. Viewport Appears Black

**Possible causes**:
- FBO not complete (check with `glCheckFramebufferStatus`)
- Forgot to clear color/depth buffers
- Viewport size is 0
- Texture filtering not set

**Debug**:
```cpp
std::cout << "FBO status: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
std::cout << "Viewport size: " << width << "x" << height << std::endl;
```

### 3. UI Doesn't Respond to Input

**Cause**: Not initializing GLFW backend with input callbacks:

```cpp
// Wrong
ImGui_ImplGlfw_InitForOpenGL(window, false);

// Correct
ImGui_ImplGlfw_InitForOpenGL(window, true);  // true = install callbacks
```

### 4. Framebuffer Leak

**Problem**: Creating new FBO every frame without deleting old ones.

**Solution**: Check if FBO exists before recreating:

```cpp
if (sceneWidth == width && sceneHeight == height && fbo != 0) {
    return true;  // Reuse existing FBO
}

// Delete old FBO before creating new one
destroyRenderTarget();
```

---

## Performance Considerations

### 1. Minimize FBO Recreations

FBO creation is expensive. Only recreate when size changes:

```cpp
if (sceneWidth != newWidth || sceneHeight != newHeight) {
    recreateFBO();  // Only when needed
}
```

### 2. Render Scene Only When Visible

```cpp
if (ImGui::Begin("Viewport", nullptr, flags)) {
    if (ImGui::IsWindowCollapsed()) {
        // Don't render if collapsed
        ImGui::End();
        return;
    }

    // Render scene
    // ...
}
ImGui::End();
```

### 3. Use Appropriate Texture Format

```cpp
// For regular rendering
glTexImage2D(..., GL_RGB, ...);  // No alpha = faster

// For transparency
glTexImage2D(..., GL_RGBA, ...);  // With alpha = slower
```

### 4. Limit Panel Updates

```cpp
// Update expensive data only occasionally
static float updateTimer = 0.0f;
updateTimer += dt;

if (updateTimer > 0.1f) {  // 10 FPS update
    updateExpensiveData();
    updateTimer = 0.0f;
}
```

---

## Advanced: Multiple Viewports

You can render different scenes to different textures:

```cpp
// Front view
ImTextureID frontView = renderSceneToTexture(camera_front, size);
ImGui::Begin("Front View");
ImGui::Image(frontView, size);
ImGui::End();

// Top view
ImTextureID topView = renderSceneToTexture(camera_top, size);
ImGui::Begin("Top View");
ImGui::Image(topView, size);
ImGui::End();
```

Each view needs its own FBO!

---

## Debugging Integration Issues

### 1. Visualize FBO Contents

Save FBO texture to image:

```cpp
std::vector<unsigned char> pixels(width * height * 3);
glBindTexture(GL_TEXTURE_2D, renderTexture);
glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
// Save pixels to image file
```

### 2. Check OpenGL State

```cpp
GLint currentFBO;
glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
std::cout << "Current FBO: " << currentFBO << std::endl;

GLint viewport[4];
glGetIntegerv(GL_VIEWPORT, viewport);
std::cout << "Viewport: " << viewport[2] << "x" << viewport[3] << std::endl;
```

### 3. Use RenderDoc

**RenderDoc** can capture frames and show you:
- All OpenGL calls
- All textures (including FBO contents)
- All shaders
- Draw call order

Download from: https://renderdoc.org/

---

## Summary

**The Integration Works By**:

1. **OpenGL renders 3D scene to a texture** (off-screen FBO)
2. **ImGui displays that texture** in a window (`ImGui::Image()`)
3. **ImGui adds UI on top** (panels, buttons, text)
4. **Both render to screen** in the final step
5. **Input is managed** with `io.WantCaptureMouse/Keyboard`

**Key Files**:
- `application.cpp` - Integration logic
- `renderer.cpp` - OpenGL 3D rendering
- `*_panel.cpp` - ImGui UI code

**Benefits**:
- ✅ 3D graphics and UI coexist
- ✅ Panels can be docked/resized
- ✅ Clean separation of concerns
- ✅ Easy to add new panels

---

**You now understand how OpenGL and ImGui integrate in our application!**

@see @ref opengl_tutorial for OpenGL details
@see @ref imgui_tutorial for ImGui details
@see @ref architecture for system architecture
