# Documentation Guide

This directory contains documentation source files for the AeroDynControlRig project.

## Generating Documentation

### Prerequisites

Install Doxygen and Graphviz:

```bash
# Ubuntu/Debian
sudo apt install doxygen graphviz

# macOS
brew install doxygen graphviz

# Check installation
doxygen --version
dot -V
```

### Build Documentation

From the project root directory:

```bash
# Generate HTML documentation
doxygen Doxyfile

# Output will be in:
# - docs/doxygen/html/index.html  (HTML documentation)
# - docs/doxygen/latex/           (LaTeX documentation)
```

### View Documentation

Open in your browser:

```bash
# Linux
xdg-open docs/doxygen/html/index.html

# macOS
open docs/doxygen/html/index.html

# Windows
start docs/doxygen/html/index.html

# Or just navigate to it manually
firefox docs/doxygen/html/index.html
```

## Documentation Structure

| File | Purpose |
|------|---------|
| `mainpage.md` | Main landing page with quick start guide |
| `architecture.dox` | Complete system architecture with visual SVG diagrams |
| `architecture.md` | Comprehensive architecture guide with diagrams |
| `README.md` | This file - documentation build instructions |
| `images/` | SVG diagrams for visual documentation |

## Documentation Features

The generated documentation includes:

### 1. Interactive Class Diagrams

Graphviz generates visual class hierarchies:

- **Inheritance diagrams**: See which classes inherit from Module and Panel
- **Collaboration diagrams**: See which classes use each other
- **Include dependency graphs**: See header file dependencies

Example: Click on `Application` class to see:
- All members and methods
- Inheritance tree
- Collaboration diagram showing it uses Renderer, Camera, PanelManager, etc.

### 2. Call Graphs

For each function, see:
- **Call graph**: What functions does this function call?
- **Caller graph**: What functions call this function?

Example: `Application::tick()` call graph shows it calls:
- `module.update()` for each module
- `renderer.renderFrame3D()`
- `panelManager.drawAll()`
- And more...

### 3. Source Code Browser

Click "Files" → "File List" to:
- Browse all source files
- See syntax-highlighted code
- Jump to definitions with cross-references

### 4. Visual SVG Diagrams

The architecture documentation (`architecture.dox`) includes beautiful SVG diagrams:

- **Complete Rendering Pipeline** (`images/pipeline_overview.svg`): 9-stage journey from keyboard to monitor
  - Input Capture → CPU Math → GPU Upload → Vertex Shader → Rasterization → Fragment Shader → Framebuffer → Display → Vision
  - Shows timing breakdown (total: ~75ms latency)
  - Color-coded stages: CPU (blue), GPU (teal), Display/Output (orange)

- **OpenGL Buffer Objects** (`images/vao_vbo_ebo.svg`): VAO, VBO, and EBO relationships
  - VBO = The Data (vertex positions, colors)
  - VAO = The Recipe (layout instructions)
  - EBO = The Connections (triangle indices)
  - 4-step workflow showing how they work together

- **Rotation Transformations** (`images/quaternion_to_matrix.svg`): Quaternion → DCM → OpenGL Matrix
  - Why three representations are needed
  - Conversion pipeline with code examples
  - Trade-offs for each representation

The architecture guide also includes ASCII-art diagrams showing:

- **Frame execution sequence**: What happens each frame in what order
- **Module data flow**: How simulation state flows through modules
- **Initialization sequence**: Critical order of subsystem startup

### 5. Search Functionality

Use the search box (top-right) to find:
- Classes, functions, members
- Documentation text
- Code snippets

## Key Pages to Start With

After generating docs, navigate to:

1. **Main Page** (`index.html`)
   - Quick start guide
   - Architecture overview
   - Links to essential classes

2. **Complete System Architecture** (click "Complete System Architecture" in Related Pages)
   - **The Complete Pipeline**: Keyboard → Monitor (11 stages with timing)
   - **OpenGL Buffer Objects**: VAO, VBO, EBO with visual diagrams
   - **Rotation Transformations**: Quaternion → DCM → OpenGL Matrix
   - **Shader Pipeline**: Vertex and Fragment shader explanations
   - **Memory Layout**: CPU RAM vs GPU VRAM breakdown
   - **ImGui Integration**: How immediate-mode UI generates OpenGL calls
   - **Performance Characteristics**: Bottlenecks and optimizations
   - **Hardware Pipeline**: GPU architecture (shader cores, rasterizer, VRAM)
   - **Debugging Tips**: Common issues and solutions

3. **Architecture Guide** (click "Architecture Guide" in Related Pages)
   - OpenGL concepts (VAO, VBO, shaders, FBO)
   - ImGui immediate mode paradigm
   - Frame-by-frame execution flow
   - Class interaction diagrams
   - Data flow through simulation

4. **Class List** (click "Classes" → "Class List")
   - See all 23+ classes
   - Each has detailed documentation
   - Inheritance and collaboration diagrams

5. **Class Hierarchy** (click "Classes" → "Class Hierarchy")
   - Visual tree of class relationships
   - Module hierarchy
   - Panel hierarchy

6. **File List** (click "Files" → "File List")
   - Browse source code
   - See file dependencies
   - Jump to definitions

## Diagram Types

### Class Diagrams

Shows inheritance and composition:

```
       Module
         ▲
         │ inherits
         │
    ┌────┴────┬────────┬─────────────┐
    │         │        │             │
Quaternion Sensor  Estimator  FirstOrder
  Demo     Sim                Dynamics
```

### Collaboration Diagrams

Shows class usage relationships (uses, contains, depends on):

```
Application ──uses──► Renderer
           └──owns──► PanelManager ──owns──► Panel[]
           └──owns──► Module[]
           └──uses──► SimulationState
```

### Call Graphs

Shows function call relationships:

```
Application::tick()
    ├─► module.update()
    ├─► renderer.renderFrame3D()
    │   └─► glDrawElements()
    └─► panelManager.drawAll()
        └─► panel.draw()
```

## Tips for Navigating

### For Beginners

1. Start with **Main Page** → Read the Quick Start
2. Read **Complete System Architecture** → Understand the rendering pipeline
   - See how keyboard input becomes pixels on your monitor
   - View beautiful SVG diagrams showing VAO/VBO/EBO relationships
   - Understand quaternion → DCM → OpenGL matrix conversion
   - Learn the 11-stage pipeline with timing breakdown
3. Read **Architecture Guide** → Understand OpenGL and ImGui basics
4. Browse **Class List** → See what classes exist
5. Pick a simple class like `TelemetryPanel`
   - Read its documentation
   - Click the collaboration diagram
   - See that it reads from `SimulationState`
6. Read `Application::init()` documentation
   - See the initialization sequence diagram
   - Understand why order matters

### For Understanding Data Flow

1. Start with `SimulationState` documentation
   - See all the data members
   - Notice nested structs for organization
2. Look at `QuaternionDemoModule`
   - See what it reads (angular_rate)
   - See what it writes (quaternion, euler, model_matrix)
3. Look at `TelemetryPanel`
   - See what it reads (quaternion, euler)
   - Notice it only reads, never writes
4. Check the architecture guide for complete data flow diagram

### For Understanding Rendering

1. Read **Complete System Architecture** → The Complete Pipeline
   - Understand the 11-stage journey from keyboard to vision
   - See timing breakdown (Input: 1ms, GPU: 1ms, Display: 16.7ms, etc.)
   - View SVG diagrams showing the complete data flow
2. Read **Complete System Architecture** → OpenGL Buffer Objects
   - Understand VAO (the recipe), VBO (the data), EBO (the connections)
   - See how they work together with visual diagrams
3. Read **Complete System Architecture** → Rotation Transformations
   - Understand why we need quaternion → DCM → OpenGL matrix
   - See the trade-offs for each representation
4. Look at `Renderer` class documentation
   - See the coordinate transformation explanation
   - Understand model/view/projection matrices
5. Look at `Application::renderSceneToTexture()`
   - See how off-screen rendering works
6. Read about Framebuffer Objects (FBO) in architecture guide

### For Adding Features

1. **To add a module**:
   - Read `Module` base class documentation
   - Look at `FirstOrderDynamicsModule` as an example
   - See `Application::initializeModules()` for registration

2. **To add a panel**:
   - Read `Panel` base class documentation
   - Look at `ControlPanel` as an example
   - See `Application::initializePanels()` for registration

## Customizing Documentation

Edit `Doxyfile` to change:

- `PROJECT_NAME`: Change project title
- `PROJECT_BRIEF`: Change tagline
- `OUTPUT_DIRECTORY`: Change where docs are generated
- `HAVE_DOT`: Enable/disable Graphviz diagrams
- `CALL_GRAPH`: Enable/disable call graphs
- `EXTRACT_PRIVATE`: Show/hide private members

After editing, regenerate with `doxygen Doxyfile`.

## Updating Documentation

When you modify code:

1. Update Doxygen comments in headers
2. Rebuild documentation: `doxygen Doxyfile`
3. Refresh browser to see changes

Doxygen comments use special syntax:

```cpp
/**
 * @brief Short one-line description
 *
 * Longer detailed description can span
 * multiple paragraphs.
 *
 * @param foo Description of parameter foo
 * @param bar Description of parameter bar
 * @return Description of return value
 *
 * @see RelatedClass
 * @note Important note
 * @warning Warning about usage
 */
void myFunction(int foo, float bar);
```

## Troubleshooting

### "Doxygen not found"
Install Doxygen: `sudo apt install doxygen` (Linux) or `brew install doxygen` (macOS)

### "Graphs not generating"
Install Graphviz: `sudo apt install graphviz` (Linux) or `brew install graphviz` (macOS)

### "Call graphs missing"
Set `CALL_GRAPH = YES` and `CALLER_GRAPH = YES` in Doxyfile

### "Class diagrams missing"
Set `HAVE_DOT = YES` and `CLASS_GRAPH = YES` in Doxyfile

### "Documentation incomplete"
Ensure `EXTRACT_ALL = YES` in Doxyfile to document even undocumented code

## Resources

- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Doxygen Comment Syntax](https://www.doxygen.nl/manual/docblocks.html)
- [Graphviz Documentation](https://graphviz.org/documentation/)

---

**Happy documenting!** 📚
