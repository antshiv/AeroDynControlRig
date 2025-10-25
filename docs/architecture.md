/**
 * @page architecture Architecture Guide
 * @brief Comprehensive guide to OpenGL, ImGui, and system architecture
 *
 * @tableofcontents
 */

# AeroDynControlRig Architecture Guide

## Table of Contents
1. [System Overview](#system-overview)
2. [OpenGL Rendering Pipeline](#opengl-rendering-pipeline)
3. [ImGui Integration](#imgui-integration)
4. [Frame Execution Flow](#frame-execution-flow)
5. [Class Interactions](#class-interactions)
6. [Data Flow](#data-flow)

---

## System Overview

AeroDynControlRig is a desktop application combining real-time 3D rendering (OpenGL) with an interactive dashboard (ImGui) for flight control algorithm development.

### High-Level Architecture

@dot "System Architecture"
digraph SystemArchitecture {
    rankdir=TB;
    node [shape=box, style="rounded,filled", fillcolor=lightblue, fontname="Arial"];
    edge [fontname="Arial"];

    app [label="Application\n(Main Orchestrator)", fillcolor=lightgreen, shape=box3d];
    glfw [label="GLFW Window\n+ OpenGL Context", fillcolor=lightyellow];
    state [label="SimulationState\n(Shared Data)", fillcolor=lightcoral];

    subgraph cluster_modules {
        label="Simulation Modules";
        style=filled;
        fillcolor=lightgray;
        quat [label="QuaternionDemo"];
        sensor [label="SensorSimulator"];
        est [label="Estimator"];
        rotor [label="RotorTelemetry"];
        dyn [label="Dynamics"];
    }

    subgraph cluster_render {
        label="Rendering";
        style=filled;
        fillcolor=lightcyan;
        renderer [label="Renderer"];
        axis [label="AxisRenderer"];
        camera [label="Camera"];
    }

    subgraph cluster_ui {
        label="UI Panels";
        style=filled;
        fillcolor=lavender;
        panels [label="7 ImGui Panels"];
    }

    app -> glfw [label="creates"];
    app -> state [label="owns"];
    app -> quat;
    app -> sensor;
    app -> est;
    app -> rotor;
    app -> dyn;
    app -> renderer;
    app -> axis;
    app -> camera;
    app -> panels;

    quat -> state [label="updates", color=blue, style=bold];
    sensor -> state [label="updates", color=blue, style=bold];
    est -> state [label="updates", color=blue, style=bold];
    rotor -> state [label="updates", color=blue, style=bold];
    dyn -> state [label="updates", color=blue, style=bold];

    renderer -> state [label="reads", color=green, style=dashed];
    panels -> state [label="reads/writes", color=red, style=dotted];
}
@enddot

### Technology Stack

| Component | Technology | Purpose |
|-----------|-----------|---------|
| Windowing | GLFW 3 | Cross-platform window creation, input handling |
| Graphics | OpenGL 3.3 | GPU-accelerated 3D rendering |
| Math | GLM | Vector/matrix math for graphics |
| UI Framework | Dear ImGui | Immediate-mode GUI for dashboards |
| Attitude Math | Custom C library | Quaternion/Euler angle conversions |

---

## OpenGL Rendering Pipeline

### The Graphics Hardware Pipeline

```
CPU Side                           GPU Side
────────                           ────────

┌─────────────────┐
│ Application.cpp │
│  tick()         │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ renderScene     │
│ ToTexture()     │
└────────┬────────┘
         │
         ▼                         ┌──────────────────┐
┌─────────────────┐                │  Framebuffer     │
│ Bind FBO        │───────────────►│  (Off-screen)    │
│ (Off-screen)    │                └──────────────────┘
└────────┬────────┘
         │
         ▼                         ┌──────────────────┐
┌─────────────────┐                │  Vertex Shader   │
│ renderer.       │───────────────►│  (Transform      │
│ renderFrame3D() │                │   vertices)      │
└────────┬────────┘                └────────┬─────────┘
         │                                  │
         │ Set Uniforms:                    ▼
         │ - model matrix            ┌──────────────────┐
         │ - view matrix             │  Rasterizer      │
         │ - projection matrix       │  (Generate       │
         │                           │   fragments)     │
         ▼                           └────────┬─────────┘
┌─────────────────┐                          │
│ glDrawElements()│                          ▼
│ (Draw cube)     │                  ┌──────────────────┐
└─────────────────┘                  │  Fragment Shader │
                                     │  (Color pixels)  │
                                     └────────┬─────────┘
         ┌────────────────────────────────────┘
         │
         ▼
┌─────────────────────┐
│ Texture attached    │
│ to FBO              │
│ (renderTexture)     │
└────────┬────────────┘
         │
         ▼
┌─────────────────────┐
│ ImGui displays      │
│ texture in          │
│ Image() widget      │
└─────────────────────┘
```

### Key OpenGL Concepts

#### 1. Vertex Buffer Objects (VBO)
```cpp
// Stores vertex data in GPU memory
float vertices[] = {
    // Position        Color
    -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Vertex 0: red
     0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Vertex 1: green
    // ... more vertices
};
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```

#### 2. Vertex Array Objects (VAO)
```cpp
// Stores the "format" of vertex data
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
// ↑ Attribute 0 (position): 3 floats, starting at offset 0

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
// ↑ Attribute 1 (color): 3 floats, starting at offset 3
```

#### 3. Element Buffer Objects (EBO)
```cpp
// Indices for drawing triangles (avoids duplicate vertices)
unsigned int indices[] = {
    0, 1, 2,  // First triangle
    2, 3, 0   // Second triangle
};
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
```

#### 4. Shaders (GLSL)
```glsl
// Vertex Shader (runs per vertex)
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;      // Object → World
uniform mat4 view;       // World → Camera
uniform mat4 projection; // Camera → Screen

out vec3 ourColor;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}

// Fragment Shader (runs per pixel)
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0);
}
```

#### 5. Framebuffer Objects (FBO)
```cpp
// Render to texture instead of screen
glGenFramebuffers(1, &fbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

// Attach color texture
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       GL_TEXTURE_2D, renderTexture, 0);

// Attach depth buffer
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                          GL_RENDERBUFFER, depthBuffer);

// Now all rendering goes to this FBO instead of the window
```

### Coordinate Transformations

```
Local Space          World Space         Camera Space        Clip Space
(Model coords)       (Scene coords)      (View coords)       (Screen coords)

   Cube                 Cube                 Cube                Screen
┌────────┐          ┌────────┐           ┌────────┐          ┌────────┐
│   /\   │          │   /\   │           │   /\   │          │   /\   │
│  /  \  │  model   │  /  \  │   view    │  /  \  │  proj    │  /  \  │
│ /____\ │  ────►   │ /____\ │   ────►   │ /____\ │  ────►   │ /____\ │
│        │  matrix  │        │  matrix   │        │  matrix  │  -1→+1 │
└────────┘          └────────┘           └────────┘          └────────┘

    │                    │                    │                    │
    └────────────────────┴────────────────────┴────────────────────┘
                              gl_Position = projection * view * model * vertex
```

---

## ImGui Integration

### Immediate Mode GUI Paradigm

**Traditional GUI (Retained Mode):**
```cpp
Button* btn = new Button("Click me");
btn->setPosition(10, 10);
// Button persists, tracks its own state
if (btn->wasClicked()) { /* ... */ }
```

**ImGui (Immediate Mode):**
```cpp
// Re-declare UI every frame - ImGui handles state internally
if (ImGui::Button("Click me")) {
    // Button was just clicked
}
```

### ImGui Rendering Flow

```
┌─────────────────────────────────────────────────────────────┐
│ Application::tick()                                         │
└────────┬────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│ ImGui::NewFrame()                                           │
│ - Reset draw lists                                          │
│ - Update input state (mouse, keyboard)                      │
│ - Begin new frame                                           │
└────────┬────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│ Application::renderDashboardLayout()                        │
│                                                             │
│   ImGui::Begin("Control Panel");                           │
│   ├─ if (ImGui::Button("Pause"))                           │
│   │     state.control.paused = !state.control.paused;      │
│   ├─ ImGui::SliderFloat("Speed", &state.control.timescale);│
│   └─ ImGui::End();                                          │
│                                                             │
│   PanelManager::drawAll(state, camera)                     │
│   ├─ TelemetryPanel::draw(state, camera)                   │
│   ├─ RotorPanel::draw(state, camera)                       │
│   └─ ... (all 7 panels)                                    │
└────────┬────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│ ImGui::Render()                                             │
│ - Finalize draw commands                                    │
│ - Optimize vertex buffers                                   │
│ - Generate ImDrawData                                       │
└────────┬────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│ ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())      │
│ - Upload UI geometry to GPU                                 │
│ - Bind ImGui's shader program                               │
│ - Draw all UI quads/triangles                               │
│ - Use texture atlas for fonts/icons                         │
└─────────────────────────────────────────────────────────────┘
```

### ImGui + OpenGL Interaction

```
┌─────────────────────────────────────────────────────────────┐
│                     Single Frame                             │
└─────────────────────────────────────────────────────────────┘

1. Render 3D Scene to Texture (Off-screen FBO)
   ┌──────────────────────────────────────┐
   │ glBindFramebuffer(GL_FRAMEBUFFER, fbo);│
   │ renderer.renderFrame3D(transform);    │
   │ axisRenderer.render3D(transform);     │
   └──────────────────┬───────────────────┘
                      │
                      ▼
              ┌───────────────┐
              │ renderTexture │ ← 3D scene now in this texture
              └───────┬───────┘
                      │
2. ImGui Frame                │
   ┌──────────────────────────▼───────────┐
   │ ImGui::NewFrame()                    │
   │                                      │
   │ ImGui::Begin("Viewport");            │
   │ ImGui::Image(renderTexture, size);   │ ← Display 3D scene
   │ ImGui::End();                        │
   │                                      │
   │ ImGui::Begin("Control Panel");       │
   │ if (ImGui::Button("Pause")) { ... }  │
   │ ImGui::End();                        │
   │                                      │
   │ ImGui::Render();                     │
   └──────────────────┬───────────────────┘
                      │
3. Render ImGui to Screen (Default framebuffer)
   ┌──────────────────▼───────────────────┐
   │ glBindFramebuffer(GL_FRAMEBUFFER, 0);│ ← Back to screen
   │ ImGui_ImplOpenGL3_RenderDrawData();  │
   └──────────────────┬───────────────────┘
                      │
4. Swap Buffers              │
   ┌──────────────────▼───────────────────┐
   │ glfwSwapBuffers(window);             │ ← Show frame
   └──────────────────────────────────────┘
```

---

## Frame Execution Flow

### Main Loop Sequence Diagram

```
┌──────┐     ┌────────────┐     ┌─────────┐     ┌────────┐     ┌──────────┐
│ main │     │Application │     │ Modules │     │Renderer│     │  ImGui   │
└──┬───┘     └─────┬──────┘     └────┬────┘     └───┬────┘     └────┬─────┘
   │               │                  │              │               │
   │ init()        │                  │              │               │
   ├──────────────►│                  │              │               │
   │               │ initializeModules│              │               │
   │               ├─────────────────►│              │               │
   │               │                  │              │               │
   │               │ renderer.init()  │              │               │
   │               ├─────────────────────────────────►│               │
   │               │                  │              │               │
   │               │ ImGui::CreateContext()           │               │
   │               ├──────────────────────────────────────────────────►│
   │               │                  │              │               │
   │ while(running)│                  │              │               │
   │ ┌─────────────┼──────────────────┼──────────────┼───────────────┼───┐
   │ │ tick()      │                  │              │               │   │
   │ ├─────────────┤                  │              │               │   │
   │ │             │ Update dt        │              │               │   │
   │ │             │                  │              │               │   │
   │ │             │ if (!paused)     │              │               │   │
   │ │             │ ┌────────────────┼──────────────┼───────────────┼───┤
   │ │             │ │ module.update()│              │               │   │
   │ │             ├─┼───────────────►│              │               │   │
   │ │             │ │                │ QuaternionDemo│               │   │
   │ │             │ │                ├──► (updates quaternion)       │   │
   │ │             │ │                │ SensorSimulator               │   │
   │ │             │ │                ├──► (computes gyro/accel)      │   │
   │ │             │ │                │ Estimator                     │   │
   │ │             │ │                ├──► (fuses sensor data)        │   │
   │ │             │ │                │ RotorTelemetry                │   │
   │ │             │ │                └──► (calculates thrust/power)  │   │
   │ │             │ └────────────────┼──────────────┼───────────────┼───┤
   │ │             │                  │              │               │   │
   │ │             │ render3D()       │              │               │   │
   │ │             ├──────────────────┼──────────────┼───────────────┼───┤
   │ │             │ Bind FBO         │              │               │   │
   │ │             │                  │              │               │   │
   │ │             │ setModelMatrix(state.model_matrix)              │   │
   │ │             ├─────────────────────────────────►│               │   │
   │ │             │ renderFrame3D()  │              │               │   │
   │ │             ├─────────────────────────────────►│               │   │
   │ │             │                  │              ├─► glDrawElements│   │
   │ │             │                  │              │   (draw cube)  │   │
   │ │             │                  │              │               │   │
   │ │             │ Unbind FBO       │              │               │   │
   │ │             │ (back to screen) │              │               │   │
   │ │             │                  │              │               │   │
   │ │             │ ImGui::NewFrame()│              │               │   │
   │ │             ├──────────────────────────────────────────────────►│   │
   │ │             │                  │              │               │   │
   │ │             │ renderDashboardLayout()          │               │   │
   │ │             │ ┌────────────────┼──────────────┼───────────────┼───┤
   │ │             │ │ ImGui::Image(renderTexture) ◄─────3D viewport │   │
   │ │             │ │                │              │               │   │
   │ │             │ │ panelManager.drawAll(state, camera)           │   │
   │ │             │ │  ├─ ControlPanel::draw()    ◄─────read/write  │   │
   │ │             │ │  ├─ TelemetryPanel::draw()  ◄─────read state  │   │
   │ │             │ │  └─ ... (7 panels total)                      │   │
   │ │             │ └────────────────┼──────────────┼───────────────┼───┤
   │ │             │                  │              │               │   │
   │ │             │ ImGui::Render()  │              │               │   │
   │ │             ├──────────────────────────────────────────────────►│   │
   │ │             │                  │              │               │   │
   │ │             │ ImGui_ImplOpenGL3_RenderDrawData()               │   │
   │ │             ├──────────────────────────────────────────────────►│   │
   │ │             │                  │              │               ├──►│
   │ │             │                  │              │               │Draw│
   │ │             │                  │              │               │UI  │
   │ │             │                  │              │               │    │
   │ │             │ glfwSwapBuffers()│              │               │   │
   │ │             ├─────────────────────────────────────────────────────►│
   │ │             │                  │              │               │Show│
   │ │             │                  │              │               │    │
   │ └─────────────┼──────────────────┼──────────────┼───────────────┼───┘
   │ Loop          │                  │              │               │
   └───────────────┼──────────────────┼──────────────┼───────────────┘
```

### Timing Diagram

```
Time ────────────────────────────────────────────────────────────►

Frame N          Frame N+1         Frame N+2
│                │                 │
├─ Update (dt)   ├─ Update (dt)    ├─ Update (dt)
│  ├─ Modules    │  ├─ Modules     │  ├─ Modules
│  └─ Physics    │  └─ Physics     │  └─ Physics
│                │                 │
├─ Render 3D     ├─ Render 3D      ├─ Render 3D
│  to FBO        │  to FBO         │  to FBO
│                │                 │
├─ Build ImGui   ├─ Build ImGui    ├─ Build ImGui
│  UI            │  UI             │  UI
│                │                 │
├─ Render ImGui  ├─ Render ImGui   ├─ Render ImGui
│  to screen     │  to screen      │  to screen
│                │                 │
└─ Swap Buffers  └─ Swap Buffers   └─ Swap Buffers
   (Display)        (Display)         (Display)

   ◄────16ms────►  ◄────16ms────►    (60 FPS)
```

---

## Class Interactions

### Module System

```
┌────────────────────────────────────────────────────────────┐
│                    SimulationState                         │
│  (Shared data structure - single source of truth)          │
│                                                            │
│  .quaternion                                               │
│  .euler                                                    │
│  .angular_rate_deg_per_sec                                 │
│  .sensor { gyro_rad_s, accel_mps2 }                        │
│  .estimator { quaternion, euler }                          │
│  .rotor { rpm[4], thrust[4], power }                       │
│  .dynamics_state { input, output }                         │
│  .control { paused, time_scale }                           │
└────────────────┬───────────────────────────────────────────┘
                 │
       ┌─────────┼─────────┐
       │         │         │
       ▼         ▼         ▼
┌──────────┐ ┌──────────┐ ┌──────────────┐
│Quaternion│ │  Sensor  │ │Complementary │
│  Demo    │ │Simulator │ │  Estimator   │
│  Module  │ │  Module  │ │   Module     │
└────┬─────┘ └────┬─────┘ └──────┬───────┘
     │            │               │
     │ Reads:     │ Reads:        │ Reads:
     │ - angular  │ - quaternion  │ - sensor.gyro
     │   _rate    │ - angular_rate│ - sensor.accel
     │            │               │
     │ Writes:    │ Writes:       │ Writes:
     │ - quaternion│ - sensor.gyro │ - estimator.quaternion
     │ - euler    │ - sensor.accel│ - estimator.euler
     │ - model_   │               │
     │   matrix   │               │
     └────────────┴───────────────┴────────────┐
                                               │
                                               ▼
                                    ┌─────────────────┐
                                    │  PanelManager   │
                                    │  drawAll()      │
                                    └────────┬────────┘
                                             │
                     ┌───────────────────────┼───────────────────┐
                     │                       │                   │
                     ▼                       ▼                   ▼
              ┌─────────────┐        ┌─────────────┐    ┌─────────────┐
              │ Telemetry   │        │  Estimator  │    │   Sensor    │
              │   Panel     │        │   Panel     │    │   Panel     │
              │ (displays   │        │ (displays   │    │ (displays   │
              │  quaternion,│        │  estimator. │    │  sensor.    │
              │  euler,     │        │  quaternion,│    │  gyro,      │
              │  rates)     │        │  euler)     │    │  accel)     │
              └─────────────┘        └─────────────┘    └─────────────┘
                     │                       │                   │
                     └───────────────────────┴───────────────────┘
                                     │
                                     │ All panels READ from
                                     │ SimulationState
                                     │
                                     │ ControlPanel WRITES to
                                     │ state.control.paused,
                                     │ state.control.time_scale, etc.
```

### Renderer Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                      Application                            │
└─────┬───────────────────────────┬───────────────────────────┘
      │                           │
      │                           │
      ▼                           ▼
┌─────────────┐           ┌─────────────┐
│  Renderer   │           │AxisRenderer │
│             │           │             │
│ Cube        │           │ RGB Axes    │
│ Geometry    │           │ Gizmo       │
└─────┬───────┘           └──────┬──────┘
      │                          │
      │ Uses                     │ Uses
      │                          │
      ▼                          ▼
┌─────────────────────────────────────────┐
│            Transform                     │
│  (GLM matrix utilities)                 │
└─────────────┬───────────────────────────┘
              │
              │ Provides matrices
              │
              ▼
┌─────────────────────────────────────────┐
│            Camera                       │
│  - position                             │
│  - front, up, right                     │
│  - yaw, pitch, zoom                     │
│                                         │
│  getViewMatrix()                        │
│  getProjectionMatrix()                  │
└─────────────────────────────────────────┘
```

### Panel System

```
┌────────────────────────────────────────────────────────────┐
│                    Panel (Interface)                       │
│  virtual const char* name() const = 0;                     │
│  virtual void draw(SimulationState&, Camera&) = 0;         │
└───────────────────────┬────────────────────────────────────┘
                        │
                        │ Implemented by
                        │
        ┌───────────────┼───────────────┬────────────────┐
        │               │               │                │
        ▼               ▼               ▼                ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│ControlPanel  │ │ Telemetry    │ │  Dynamics    │ │   Rotor      │
│              │ │   Panel      │ │   Panel      │ │   Panel      │
│ Buttons:     │ │              │ │              │ │              │
│ - Pause      │ │ Displays:    │ │ Has:         │ │ Displays:    │
│ - Resume     │ │ - Quaternion │ │ - PlotLines  │ │ - RPM[4]     │
│              │ │ - Euler      │ │ - History    │ │ - Thrust[4]  │
│ Sliders:     │ │ - Rates      │ │   buffers    │ │ - Power      │
│ - Time scale │ │ - Time       │ │              │ │              │
└──────────────┘ └──────────────┘ └──────────────┘ └──────────────┘
```

---

## Data Flow

### State Updates

```
User Input (Keyboard/Mouse)
    │
    │ GLFW callbacks
    │
    ▼
┌─────────────────────────────────┐
│   Application::keyCallback()    │
│   Application::mouseCallback()  │
└─────────────┬───────────────────┘
              │
              │ Updates
              │
              ▼
┌─────────────────────────────────┐
│          Camera                 │
│  - position                     │
│  - yaw, pitch                   │
└─────────────┬───────────────────┘
              │
              │ Used by
              │
              ▼
┌─────────────────────────────────┐
│  Renderer::setViewMatrix()      │
│  (Rendering uses camera view)   │
└─────────────────────────────────┘


User Input (ImGui widgets)
    │
    │ ImGui::Button(), ImGui::SliderFloat(), etc.
    │
    ▼
┌─────────────────────────────────┐
│   Panel::draw()                 │
│   if (ImGui::Button("Pause"))   │
│     state.control.paused = true;│
└─────────────┬───────────────────┘
              │
              │ Modifies
              │
              ▼
┌─────────────────────────────────┐
│      SimulationState            │
│  .control.paused                │
│  .control.time_scale            │
│  .dynamics_config.*             │
└─────────────┬───────────────────┘
              │
              │ Read by
              │
              ▼
┌─────────────────────────────────┐
│  Application::tick()            │
│  if (!state.control.paused)     │
│    module.update(dt, state);    │
└─────────────────────────────────┘
```

### Simulation Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    Simulation Tick                          │
└─────────────────────────────────────────────────────────────┘

Step 1: Attitude Integration
┌──────────────────────┐
│ QuaternionDemoModule │
│                      │
│ Read:  angular_rate  │
│ Write: quaternion    │
│        euler         │
│        model_matrix  │
└──────────┬───────────┘
           │
           ▼
Step 2: Sensor Simulation
┌──────────────────────┐
│ SensorSimulatorModule│
│                      │
│ Read:  quaternion    │
│        angular_rate  │
│ Write: sensor.gyro   │
│        sensor.accel  │
└──────────┬───────────┘
           │
           ▼
Step 3: State Estimation
┌──────────────────────────┐
│ ComplementaryEstimator   │
│         Module           │
│                          │
│ Read:  sensor.gyro       │
│        sensor.accel      │
│ Write: estimator.quat    │
│        estimator.euler   │
└──────────┬───────────────┘
           │
           ▼
Step 4: Rotor Calculations
┌──────────────────────┐
│ RotorTelemetryModule │
│                      │
│ Read:  rotor_config  │
│ Write: rotor.rpm[]   │
│        rotor.thrust[]│
│        rotor.power   │
└──────────┬───────────┘
           │
           ▼
Step 5: Power Integration
┌──────────────────────┐
│ PowerModule          │
│ (implicit in rotor)  │
│                      │
│ Read:  rotor.power   │
│ Write: power.current │
│        power.energy  │
└──────────────────────┘
```

### Rendering Data Flow

```
SimulationState.model_matrix
           │
           │ Set by QuaternionDemoModule
           │
           ▼
┌──────────────────────────┐
│ Renderer::setModelMatrix │
└──────────┬───────────────┘
           │
           │ Combined with
           │
           ▼
Camera.getViewMatrix()
Camera.getProjectionMatrix()
           │
           │ Sent to GPU as uniforms
           │
           ▼
┌──────────────────────────┐
│ Vertex Shader            │
│ gl_Position =            │
│   projection * view      │
│   * model * vertex       │
└──────────┬───────────────┘
           │
           │ GPU processes geometry
           │
           ▼
┌──────────────────────────┐
│ Fragment Shader          │
│ (Colors pixels)          │
└──────────┬───────────────┘
           │
           │ Output to
           │
           ▼
┌──────────────────────────┐
│ Framebuffer (FBO)        │
│ renderTexture            │
└──────────┬───────────────┘
           │
           │ Displayed in
           │
           ▼
┌──────────────────────────┐
│ ImGui::Image()           │
│ (Viewport panel)         │
└──────────────────────────┘
```

---

## Quick Reference: Key Files

| File | Purpose | Key Concepts |
|------|---------|--------------|
| `main.cpp` | Entry point | Creates Application, runs main loop |
| `application.h/cpp` | Orchestrator | GLFW, OpenGL context, main tick() |
| `simulation_state.h` | Data hub | All simulation data in one struct |
| `module.h` | Interface | Base class for physics modules |
| `panel.h` | Interface | Base class for UI panels |
| `renderer.h/cpp` | 3D graphics | VAO/VBO setup, shader loading, draw calls |
| `camera.h/cpp` | View control | Mouse look, WASD movement, matrices |
| `panel_manager.h/cpp` | UI registry | Owns and draws all panels |

---

## Debugging Tips

### OpenGL Debugging
```cpp
// Check for OpenGL errors after suspicious calls
GLenum err = glGetError();
if (err != GL_NO_ERROR) {
    std::cerr << "OpenGL error: " << err << std::endl;
}

// Verify shader compilation
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "Shader compilation failed: " << infoLog << std::endl;
}
```

### ImGui Debugging
```cpp
// Show demo window with all widgets and examples
ImGui::ShowDemoWindow();

// Show metrics window for performance
ImGui::ShowMetricsWindow();

// Inspect current state
std::cout << "Paused: " << state.control.paused << std::endl;
std::cout << "Quaternion: ["
          << state.quaternion[0] << ", "
          << state.quaternion[1] << ", "
          << state.quaternion[2] << ", "
          << state.quaternion[3] << "]" << std::endl;
```

---

## Next Steps for Learning

1. **OpenGL Tutorial**: https://learnopengl.com/
   - Start with "Hello Triangle" to understand VAO/VBO/EBO
   - "Transformations" chapter for model/view/projection matrices
   - "Framebuffers" for off-screen rendering

2. **ImGui Examples**:
   - `imgui/examples/example_glfw_opengl3/` in the ImGui repository
   - Browse `imgui_demo.cpp` for widget examples

3. **Experiment**:
   - Modify shader colors in `shaders/` directory
   - Add a new panel by inheriting from `Panel`
   - Create a new module by inheriting from `Module`
   - Change camera speed in `camera.h` (movementSpeed)

4. **Visualization**:
   - Run `doxygen Doxyfile` to generate class diagrams
   - Use `ImGui::ShowMetricsWindow()` to see render stats
   - Add `ImGui::PlotLines()` to visualize simulation data

---

*This guide is intended to be viewed in Doxygen. For best results, enable `HAVE_DOT = YES` in your Doxyfile to generate class relationship diagrams.*
