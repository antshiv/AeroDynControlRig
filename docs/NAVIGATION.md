# How to Navigate the Documentation

## Quick Access to Architecture Guide

After opening `docs/doxygen/html/index.html`, you can access the Architecture Guide in **three ways**:

### Method 1: From the Main Page (Easiest)

1. Open `docs/doxygen/html/index.html`
2. Scroll down to **"Essential Reading"** table
3. Click on **"Architecture Guide"** link in the first row
4. You'll see the complete guide with all diagrams!

### Method 2: From the Top Navigation

1. Open `docs/doxygen/html/index.html`
2. Click **"Related Pages"** in the top navigation bar
3. Click **"Architecture Guide"** from the list
4. Done!

### Method 3: Direct Link

Just open: `docs/doxygen/html/architecture.html` directly in your browser

---

## What You'll Find in the Architecture Guide

The Architecture Guide (`architecture.html`) contains **500+ lines** of detailed explanations with ASCII diagrams:

### Section 1: System Overview
- High-level architecture diagram
- Technology stack table
- Component relationships

### Section 2: OpenGL Rendering Pipeline
- **CPU to GPU data flow diagram**
- Vertex/Fragment shader pipeline
- Key OpenGL concepts explained:
  - VBO (Vertex Buffer Objects) with code examples
  - VAO (Vertex Array Objects) with code examples
  - EBO (Element Buffer Objects) with code examples
  - Shaders (GLSL vertex + fragment shader examples)
  - FBO (Framebuffer Objects) with code examples
- **Coordinate transformation diagram**: Local → World → Camera → Clip

### Section 3: ImGui Integration
- **Immediate mode vs retained mode comparison**
- ImGui rendering flow diagram (step-by-step)
- How ImGui and OpenGL interact each frame
- **4-step frame rendering diagram**

### Section 4: Frame Execution Flow
- **Main loop sequence diagram** showing:
  - What `main()` calls
  - What `Application::tick()` does
  - How modules update
  - How rendering happens
  - How UI renders
- **Timing diagram** showing 60 FPS frame breakdown

### Section 5: Class Interactions
- **Module system data flow**
- Renderer hierarchy
- Panel system organization
- State update flow diagrams

### Section 6: Data Flow
- **User input → Camera → Renderer flow**
- **User input → Panel → SimulationState → Modules flow**
- **5-step simulation tick diagram**
- **Rendering data flow**: state → GPU → screen

### Section 7: Quick Reference
- Key files and their purposes
- Debugging tips (OpenGL + ImGui)
- Next steps for learning
- External resource links

---

## Navigation Tips

### Finding Specific Information

| Want to learn about... | Go to section... |
|------------------------|------------------|
| What is a shader? | OpenGL Rendering Pipeline → Shaders |
| How does the cube rotate? | Coordinate Transformations |
| What is FBO? | OpenGL Rendering Pipeline → Framebuffer Objects |
| How does ImGui work? | ImGui Integration |
| What happens each frame? | Frame Execution Flow |
| How do modules communicate? | Class Interactions → Module System |
| How does user input work? | Data Flow |

### Using the Table of Contents

The Architecture Guide has a **clickable table of contents** at the top:

```
Table of Contents
1. System Overview
2. OpenGL Rendering Pipeline
3. ImGui Integration
4. Frame Execution Flow
5. Class Interactions
6. Data Flow
```

Click any section to jump directly to it!

### Viewing Diagrams

All ASCII diagrams are in monospace code blocks for proper alignment:

```
┌─────────────┐
│ Application │
└──────┬──────┘
       │
   ┌───┴───┐
   ▼       ▼
Renderer Panel
```

They render perfectly in the HTML documentation.

---

## Other Useful Pages

### Main Page (`index.html`)
- Quick start guide
- Architecture overview
- Learning path for beginners
- Links to all key classes

### Class List (`classes.html`)
- All 23+ documented classes
- Click any class name to see:
  - Full API documentation
  - Inheritance diagram
  - Collaboration diagram
  - Member list

### Class Hierarchy (`hierarchy.html`)
- Visual tree of class relationships
- See Module and Panel hierarchies
- Understand inheritance structure

### Files → File List
- Browse all source files
- See syntax-highlighted code
- Jump to definitions

---

## Diagram Examples in Architecture Guide

Here's what you'll see:

### OpenGL Pipeline Diagram
Shows data flowing from CPU (C++ code) → GPU (shaders) → Screen

### ImGui Frame Rendering
Shows all 4 steps:
1. Render 3D scene to texture (off-screen FBO)
2. ImGui frame setup
3. Render ImGui to screen
4. Swap buffers

### Module Data Flow
Shows how data flows through:
QuaternionDemo → SensorSimulator → Estimator → Panels

### Sequence Diagrams
Shows function calls:
```
main() → Application::init() → renderer.init() → shader compilation
```

---

## For First-Time Readers

**Start here:**

1. Open `index.html`
2. Read "Quick Start" section
3. Click **"Architecture Guide"** link
4. Read sections in this order:
   - System Overview (understand big picture)
   - OpenGL Rendering Pipeline (learn graphics basics)
   - ImGui Integration (learn UI basics)
   - Frame Execution Flow (see it all together)
5. Browse "Class List" to see all documented classes
6. Pick a simple class like `TelemetryPanel` and read its docs

**You'll understand:**
- ✅ What OpenGL is and how it works
- ✅ What ImGui is and why it's "immediate mode"
- ✅ How the application renders frames
- ✅ How modules communicate
- ✅ How to add your own features

---

## Common Questions

**Q: Where are the diagrams?**
A: In the Architecture Guide (`architecture.html`). Click "Related Pages" → "Architecture Guide"

**Q: How do I see class diagrams?**
A: Click any class name (e.g., `Application`) → scroll down to see inheritance/collaboration diagrams

**Q: Can I see function call graphs?**
A: Yes! Click on any function → look for "Call Graph" and "Caller Graph" sections

**Q: How do I search?**
A: Use the search box in the top-right corner of any page

**Q: Is there a printer-friendly version?**
A: The Architecture Guide is in `docs/architecture.md` (plain markdown) if you prefer reading the source

---

## Shortcut: View Docs Script

For convenience, use:

```bash
./view_docs.sh
```

This will:
1. Regenerate documentation (if needed)
2. Open `index.html` in your default browser
3. Display navigation tips

---

**Happy exploring!** 📚

The Architecture Guide is specifically designed to teach OpenGL and ImGui concepts to beginners while serving as a technical reference for experienced developers.
