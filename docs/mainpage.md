# AeroDynControlRig Documentation

Welcome to the AeroDynControlRig API documentation!

## What is AeroDynControlRig?

AeroDynControlRig is a **desktop visualization and testbed** for flight-control stack development. It provides a fast feedback loop for control algorithm development by combining:

- **Real-time 3D visualization** using OpenGL 3.3
- **Interactive dashboard** built with Dear ImGui
- **Modular simulation architecture** for easy extension
- **Quaternion-based attitude dynamics**
- **Sensor simulation and state estimation**

## Quick Start

1. **Build the project**:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   cmake --build build
   ./build/AeroDynControlRig
   ```

2. **Explore the UI**:
   - Viewport: 3D cube rotating based on attitude
   - Control Panel: Pause/resume, adjust simulation speed
   - Telemetry: View quaternion, Euler angles, rates
   - Dynamics: Configure and plot first-order system

3. **Understand the code**:
   - See @ref architecture.md for detailed diagrams and explanations
   - Browse the class list to see all 23+ classes with full documentation

## Architecture Overview

```
Application
    ├─ GLFW Window + OpenGL Context
    ├─ SimulationState (shared data)
    │   └─ Updated by Modules
    │       ├─ QuaternionDemoModule (attitude)
    │       ├─ SensorSimulatorModule (IMU)
    │       ├─ ComplementaryEstimatorModule (fusion)
    │       ├─ RotorTelemetryModule (thrust/power)
    │       └─ FirstOrderDynamicsModule (test system)
    ├─ Renderer (3D graphics → off-screen FBO)
    │   ├─ Cube geometry
    │   ├─ Axis gizmo
    │   └─ Camera control
    └─ PanelManager (ImGui UI)
        ├─ ControlPanel
        ├─ TelemetryPanel
        ├─ DynamicsPanel
        ├─ EstimatorPanel
        ├─ RotorPanel
        ├─ SensorPanel
        └─ PowerPanel
```

## Essential Reading

| Guide | Description |
|-------|-------------|
| @ref complete_picture | **START HERE!** The complete flow: Your code → ImGui → OpenGL → GPU → Monitor (with beautiful diagrams!) |
| @ref pixel_truth | The fundamental truth: Everything is just computing RGBA for pixels |
| @ref display_fundamentals | How pixels get on screen - from framebuffers to monitors |
| @ref hardware_pipeline | Deep dive: RAM → Cache → GPU → Screen. Understand the hardware! |
| @ref architecture | Comprehensive guide with diagrams explaining OpenGL, ImGui, and how all classes interact |
| @ref opengl_tutorial | Complete OpenGL tutorial with code examples from our renderer |
| @ref imgui_tutorial | Complete ImGui tutorial with code examples from our panels |
| @ref opengl_imgui_integration | How OpenGL and ImGui work together in our application |
| @ref Application | Main orchestrator - initialization sequence and main loop |
| @ref SimulationState | Central data structure - what data flows where |
| @ref Module | Base interface for simulation modules |
| @ref Panel | Base interface for UI panels |

## 🌟 New: Web Developer? You Already Know This!

If you understand HTML, CSS, and JavaScript, you already understand this codebase! We've created a **complete mapping** between web concepts and our OpenGL/ImGui architecture:

### Quick Translation Guide

| Web Concept | AeroDynControlRig Equivalent | Example |
|-------------|------------------------------|---------|
| `<div class="card">` | `ui::BeginCard()` | Container element |
| `style="color: red"` | `ImGui::PushStyleColor()` | Inline styling |
| CSS custom properties (`:root { --primary: #007bff; }`) | `ui::Palette` struct | Design tokens |
| `<button onclick="handleClick()">` | `if (ImGui::Button(...))` | Event handling |
| `<script>` component pattern | `Panel` class | Reusable UI component |
| Global `window.appState` | `SimulationState` | Global state object |

**Example - Same UI, Different Syntax:**

Web (HTML + CSS + JS):
```html
<div class="card">
  <h2>Rotor Dynamics <span class="badge-success">+2.5%</span></h2>
  <p class="text-muted">Rotor RPM</p>
  <h1>2140 RPM</h1>
</div>

<style>
.card { padding: 16px; background: white; border-radius: 8px; }
.text-muted { color: #6c757d; }
.badge-success { color: #28a745; }
</style>
```

AeroDynControlRig (C++ + ImGui):
```cpp
ui::BeginCard("Rotor", options);
    ui::CardHeader("Rotor Dynamics", "+2.5%", &palette.success);
    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
    ImGui::TextUnformatted("Rotor RPM");
    ImGui::PopStyleColor();
    ImGui::PushFont(fonts.heading);
    ImGui::Text("2140 RPM");
    ImGui::PopFont();
ui::EndCard();
```

---

## Key Concepts

### OpenGL Rendering Pipeline

The application uses **off-screen rendering** with Framebuffer Objects (FBO):

1. **Render 3D scene** (cube + axes) to an off-screen texture
2. **Display texture** in an ImGui Image() widget
3. **Render ImGui UI** on top (panels, controls)
4. **Swap buffers** to show the complete frame

See @ref architecture for detailed pipeline diagrams.

### ImGui Immediate Mode

Unlike traditional GUIs where you create widgets once and manage their state:

```cpp
// Traditional (retained mode)
Button* btn = new Button("Click");
if (btn->wasClicked()) { ... }
```

ImGui uses **immediate mode** - you re-declare the UI every frame:

```cpp
// ImGui (immediate mode)
if (ImGui::Button("Click")) {
    // Just clicked!
}
```

ImGui handles all state tracking internally. This makes UI code simple and declarative.

### Module System

Modules implement the @ref Module interface:

- `initialize(SimulationState& state)` - Called once at startup
- `update(double dt, SimulationState& state)` - Called each frame

All modules read from and write to the shared @ref SimulationState, enabling loose coupling.

### Panel System

Panels implement the @ref Panel interface:

- `name()` - Returns panel display name
- `draw(SimulationState& state, Camera& camera)` - Renders ImGui UI

Panels can read from state (for display) and write to state (for user controls).

## Learning Path

If you're new to OpenGL and ImGui, follow this path:

1. **Read the Architecture Guide**: @ref architecture
   - Understand the frame execution flow
   - See how OpenGL and ImGui interact
   - Learn the coordinate transformation pipeline

2. **Explore the code**:
   - Start with Application::init() to see initialization
   - Follow Application::tick() to see the main loop
   - Read QuaternionDemoModule to see a simple module

3. **Make changes**:
   - Add a new panel (copy TelemetryPanel, modify draw())
   - Create a new module (copy FirstOrderDynamicsModule)
   - Modify shader colors in `shaders/` directory

4. **External resources**:
   - [LearnOpenGL](https://learnopengl.com/) - OpenGL tutorials
   - [ImGui Demo](https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp) - Widget examples
   - [GLM Documentation](https://glm.g-truc.net/) - Math library reference

## Class Categories

### Core Infrastructure
- @ref Application - Main application class
- @ref SimulationState - Shared data structure
- @ref Module - Base class for simulation modules
- @ref Panel - Base class for UI panels
- @ref PanelManager - Panel registry and orchestrator

### Rendering
- @ref Renderer - Main 3D scene renderer (OpenGL)
- @ref AxisRenderer - Coordinate axis gizmo
- @ref Camera - First-person camera with mouse look

### Simulation Modules
- @ref QuaternionDemoModule - Attitude dynamics (quaternion integration)
- @ref SensorSimulatorModule - IMU sensor generation (gyro + accel)
- @ref ComplementaryEstimatorModule - Sensor fusion (complementary filter)
- @ref FirstOrderDynamicsModule - Test system (LTI dynamics)
- @ref RotorTelemetryModule - Rotor thrust/torque/power calculations

### UI Panels
- @ref ControlPanel - Simulation playback controls
- @ref TelemetryPanel - Flight state display
- @ref DynamicsPanel - First-order system config and plotting
- @ref EstimatorPanel - State estimator diagnostics
- @ref RotorPanel - Rotor performance metrics
- @ref SensorPanel - IMU sensor readouts
- @ref PowerPanel - Electrical power monitoring

## Development Roadmap

Current stage: **Stage 0 - Core Visual Sandbox**

| Stage | Goal | Status |
|-------|------|--------|
| 0 | Core Visual Sandbox | ✅ In Progress |
| 1 | Dynamics Explorer | Planned |
| 2 | Controller Workbench | Planned |
| 3 | Estimation Diagnostics | Planned |
| 4 | INS Integration | Planned |
| 5 | High-Fidelity Simulation | Planned |

See [README.md](../README.md) for detailed roadmap.

## Getting Help

- **Class documentation**: Click on any class name to see detailed API docs
- **Visual diagrams**: All classes have inheritance and collaboration diagrams
- **Call graphs**: Functions show caller/callee relationships (requires Graphviz)
- **Architecture guide**: @ref architecture has comprehensive diagrams

## Contributing

This project uses:
- C++17 standard
- CMake 3.10+ build system
- Doxygen for documentation (you're reading it!)
- Git submodules for external dependencies

To add a new module:
1. Create header in `src/modules/your_module.h`
2. Inherit from Module and implement initialize() and update()
3. Register in Application::initializeModules()

To add a new panel:
1. Create header in `src/gui/panels/your_panel.h`
2. Inherit from Panel and implement name() and draw()
3. Register in Application::initializePanels()

---

**Happy coding!** 🚀

For a deep dive into the architecture, see @ref architecture.
