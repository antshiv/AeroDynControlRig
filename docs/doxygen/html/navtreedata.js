/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "AeroDynControlRig", "index.html", [
    [ "AeroDynControlRig Documentation", "index.html", [
      [ "What is AeroDynControlRig?", "index.html#autotoc_md223", null ],
      [ "Quick Start", "index.html#autotoc_md224", null ],
      [ "Architecture Overview", "index.html#autotoc_md225", null ],
      [ "Essential Reading", "index.html#autotoc_md226", null ],
      [ "🌟 New: Web Developer? You Already Know This!", "index.html#autotoc_md227", [
        [ "Quick Translation Guide", "index.html#autotoc_md228", null ]
      ] ],
      [ "Key Concepts", "index.html#autotoc_md230", [
        [ "OpenGL Rendering Pipeline", "index.html#autotoc_md231", null ],
        [ "ImGui Immediate Mode", "index.html#autotoc_md232", null ],
        [ "Module System", "index.html#autotoc_md233", null ],
        [ "Panel System", "index.html#autotoc_md234", null ]
      ] ],
      [ "Learning Path", "index.html#autotoc_md235", null ],
      [ "Class Categories", "index.html#autotoc_md236", [
        [ "Core Infrastructure", "index.html#autotoc_md237", null ],
        [ "Rendering", "index.html#autotoc_md238", null ],
        [ "Simulation Modules", "index.html#autotoc_md239", null ],
        [ "UI Panels", "index.html#autotoc_md240", null ]
      ] ],
      [ "Development Roadmap", "index.html#autotoc_md241", null ],
      [ "Getting Help", "index.html#autotoc_md242", null ],
      [ "Contributing", "index.html#autotoc_md243", null ]
    ] ],
    [ "architecture", "md_docs_architecture.html", null ],
    [ "Architecture Guide", "architecture.html", [
      [ "AeroDynControlRig Architecture Guide", "architecture.html#autotoc_md0", [
        [ "Table of Contents", "architecture.html#autotoc_md1", null ],
        [ "System Overview", "architecture.html#autotoc_md3", [
          [ "High-Level Architecture", "architecture.html#autotoc_md4", null ],
          [ "Technology Stack", "architecture.html#autotoc_md5", null ]
        ] ],
        [ "OpenGL Rendering Pipeline", "architecture.html#autotoc_md7", [
          [ "The Graphics Hardware Pipeline", "architecture.html#autotoc_md8", null ],
          [ "Key OpenGL Concepts", "architecture.html#autotoc_md9", [
            [ "1. Vertex Buffer Objects (VBO)", "architecture.html#autotoc_md10", null ],
            [ "2. Vertex Array Objects (VAO)", "architecture.html#autotoc_md11", null ],
            [ "3. Element Buffer Objects (EBO)", "architecture.html#autotoc_md12", null ],
            [ "4. Shaders (GLSL)", "architecture.html#autotoc_md13", null ],
            [ "5. Framebuffer Objects (FBO)", "architecture.html#autotoc_md14", null ]
          ] ],
          [ "Coordinate Transformations", "architecture.html#autotoc_md15", null ]
        ] ],
        [ "ImGui Integration", "architecture.html#autotoc_md17", [
          [ "Immediate Mode GUI Paradigm", "architecture.html#autotoc_md18", null ],
          [ "ImGui Rendering Flow", "architecture.html#autotoc_md19", null ],
          [ "ImGui + OpenGL Interaction", "architecture.html#autotoc_md20", null ]
        ] ],
        [ "Frame Execution Flow", "architecture.html#autotoc_md22", [
          [ "Main Loop Sequence Diagram", "architecture.html#autotoc_md23", null ],
          [ "Timing Diagram", "architecture.html#autotoc_md24", null ]
        ] ],
        [ "Class Interactions", "architecture.html#autotoc_md26", [
          [ "Module System", "architecture.html#autotoc_md27", null ],
          [ "Renderer Hierarchy", "architecture.html#autotoc_md28", null ],
          [ "Panel System", "architecture.html#autotoc_md29", null ]
        ] ],
        [ "Data Flow", "architecture.html#autotoc_md31", [
          [ "State Updates", "architecture.html#autotoc_md32", null ],
          [ "Simulation Data Flow", "architecture.html#autotoc_md33", null ],
          [ "Rendering Data Flow", "architecture.html#autotoc_md34", null ]
        ] ],
        [ "Quick Reference: Key Files", "architecture.html#autotoc_md36", null ],
        [ "Debugging Tips", "architecture.html#autotoc_md38", [
          [ "OpenGL Debugging", "architecture.html#autotoc_md39", null ],
          [ "ImGui Debugging", "architecture.html#autotoc_md40", null ]
        ] ],
        [ "Next Steps for Learning", "architecture.html#autotoc_md42", null ]
      ] ]
    ] ],
    [ "complete_picture", "md_docs_complete_picture.html", null ],
    [ "The Complete Picture: From Your Intent to Pixels on Screen", "complete_picture.html", null ],
    [ "display_fundamentals", "md_docs_display_fundamentals.html", null ],
    [ "Display Fundamentals: Pixels, Framebuffers, and the OS", "display_fundamentals.html", null ],
    [ "Documentation Update Summary", "md_docs_DOCUMENTATION_UPDATE.html", [
      [ "What Was Created", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md45", [
        [ "1. <strong>Web Analogy Guide</strong> (<tt>docs/web_analogy.md</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md46", null ],
        [ "2. <strong>GPU Pipeline Guide</strong> (<tt>docs/gpu_pipeline.md</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md47", null ],
        [ "3. <strong>Doxygen Module Groups</strong> (<tt>docs/groups.dox</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md48", null ],
        [ "4. <strong>Enhanced Header Documentation</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md49", [
          [ "<tt>src/gui/widgets/card.h</tt>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md50", null ],
          [ "<tt>src/gui/style.h</tt>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md51", null ]
        ] ],
        [ "5. <strong>Updated Main Documentation</strong> (<tt>docs/mainpage.md</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md52", null ]
      ] ],
      [ "How to View the Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md53", [
        [ "Option 1: Generate HTML Docs (Recommended)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md54", null ],
        [ "Option 2: Read Markdown Directly", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md55", null ]
      ] ],
      [ "Key Features", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md56", [
        [ "1. Web Analogy Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md57", null ],
        [ "2. Hardware Pipeline Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md58", null ],
        [ "3. Cross-Referenced Examples", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md59", null ]
      ] ],
      [ "File Changes Summary", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md60", null ],
      [ "What This Achieves", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md61", [
        [ "1. <strong>Lowers Barrier to Entry</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md62", null ],
        [ "2. <strong>Explains the \"Why\"</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md63", null ],
        [ "3. <strong>Demystifies the Hardware</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md64", null ],
        [ "4. <strong>Makes Design Decisions Clear</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md65", null ]
      ] ],
      [ "Example Documentation Snippets", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md66", [
        [ "Color Palette Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md67", null ],
        [ "Font Loading Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md68", null ]
      ] ],
      [ "How to Use This Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md69", [
        [ "For New Contributors", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md70", null ],
        [ "For Yourself (Future Reference)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md71", null ],
        [ "For Teaching Others", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md72", null ]
      ] ],
      [ "Next Steps (Optional)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md73", null ],
      [ "Generating the Docs", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md74", null ],
      [ "Documentation Quality", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md75", [
        [ "Coverage", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md76", null ],
        [ "Depth", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md77", null ],
        [ "Accessibility", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md78", null ]
      ] ],
      [ "Summary", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md80", null ]
    ] ],
    [ "GPU Pipeline: From RAM to Screen", "gpu_pipeline.html", [
      [ "Overview", "gpu_pipeline.html#autotoc_md81", null ],
      [ "The Complete Pipeline", "gpu_pipeline.html#autotoc_md83", null ],
      [ "Stage 1: CPU Side - Your C++ Code", "gpu_pipeline.html#autotoc_md85", [
        [ "1.1 Application Update Loop", "gpu_pipeline.html#autotoc_md86", null ],
        [ "1.2 Module Updates (Pure CPU Work)", "gpu_pipeline.html#autotoc_md88", null ],
        [ "1.3 ImGui Vertex Generation", "gpu_pipeline.html#autotoc_md90", null ],
        [ "1.4 OpenGL Command Submission", "gpu_pipeline.html#autotoc_md92", null ]
      ] ],
      [ "Stage 2: PCIe Bus Transfer", "gpu_pipeline.html#autotoc_md94", [
        [ "2.1 CPU → GPU Communication", "gpu_pipeline.html#autotoc_md95", null ]
      ] ],
      [ "Stage 3: GPU Side - Graphics Card", "gpu_pipeline.html#autotoc_md97", [
        [ "3.1 GPU Architecture Overview", "gpu_pipeline.html#autotoc_md98", null ],
        [ "3.2 GPU Memory Hierarchy", "gpu_pipeline.html#autotoc_md100", null ],
        [ "3.3 Vertex Shader Execution", "gpu_pipeline.html#autotoc_md102", null ],
        [ "3.4 Rasterization (Fixed-Function Hardware)", "gpu_pipeline.html#autotoc_md104", null ],
        [ "3.5 Fragment Shader Execution", "gpu_pipeline.html#autotoc_md106", null ],
        [ "3.6 Framebuffer Operations", "gpu_pipeline.html#autotoc_md108", null ]
      ] ],
      [ "Stage 4: Display Output", "gpu_pipeline.html#autotoc_md110", [
        [ "4.1 Swap Buffers", "gpu_pipeline.html#autotoc_md111", null ],
        [ "4.2 Display Controller Scanout", "gpu_pipeline.html#autotoc_md113", null ]
      ] ],
      [ "Complete Frame Timing Breakdown", "gpu_pipeline.html#autotoc_md115", null ],
      [ "Memory Bandwidth Analysis", "gpu_pipeline.html#autotoc_md117", [
        [ "CPU Side (System RAM)", "gpu_pipeline.html#autotoc_md118", null ],
        [ "PCIe Bus (CPU ↔ GPU)", "gpu_pipeline.html#autotoc_md119", null ],
        [ "GPU Side (VRAM)", "gpu_pipeline.html#autotoc_md120", null ]
      ] ],
      [ "Optimization Insights", "gpu_pipeline.html#autotoc_md122", [
        [ "Where Time Is Actually Spent", "gpu_pipeline.html#autotoc_md123", null ],
        [ "If You Wanted to Optimize", "gpu_pipeline.html#autotoc_md124", null ]
      ] ],
      [ "Key Takeaways", "gpu_pipeline.html#autotoc_md126", null ],
      [ "Mapping to AeroDynControlRig Code", "gpu_pipeline.html#autotoc_md128", null ],
      [ "Related Documentation", "gpu_pipeline.html#autotoc_md130", null ],
      [ "Further Reading", "gpu_pipeline.html#autotoc_md132", null ]
    ] ],
    [ "UI Architecture Overview", "ui_architecture.html", [
      [ "Introduction", "ui_architecture.html#ui_arch_intro", null ],
      [ "Data Flow", "ui_architecture.html#ui_arch_data", null ],
      [ "Styling System", "ui_architecture.html#ui_arch_styling", null ],
      [ "Component System", "ui_architecture.html#ui_arch_components", null ],
      [ "Panel System", "ui_architecture.html#ui_arch_panels", null ],
      [ "Layout System", "ui_architecture.html#ui_arch_layout", null ]
    ] ],
    [ "hardware_pipeline", "md_docs_hardware_pipeline.html", null ],
    [ "Hardware Pipeline: RAM → CPU → GPU → Screen", "hardware_pipeline.html", null ],
    [ "imgui_tutorial", "md_docs_imgui_tutorial.html", null ],
    [ "ImGui Deep Dive Tutorial", "imgui_tutorial.html", [
      [ "ImGui Deep Dive Tutorial", "imgui_tutorial.html#autotoc_md139", [
        [ "What is ImGui?", "imgui_tutorial.html#autotoc_md140", [
          [ "Why ImGui for Our Application?", "imgui_tutorial.html#autotoc_md141", null ]
        ] ],
        [ "The Big Idea: Immediate Mode", "imgui_tutorial.html#autotoc_md143", [
          [ "Traditional GUI (Retained Mode)", "imgui_tutorial.html#autotoc_md144", null ],
          [ "ImGui (Immediate Mode)", "imgui_tutorial.html#autotoc_md145", null ]
        ] ],
        [ "Setup and Initialization", "imgui_tutorial.html#autotoc_md147", [
          [ "From <tt>application.cpp init()</tt>", "imgui_tutorial.html#autotoc_md148", null ],
          [ "What Each Backend Does", "imgui_tutorial.html#autotoc_md149", null ]
        ] ],
        [ "Frame Structure", "imgui_tutorial.html#autotoc_md151", null ],
        [ "Core Widgets", "imgui_tutorial.html#autotoc_md153", [
          [ "Buttons", "imgui_tutorial.html#autotoc_md154", null ],
          [ "Text", "imgui_tutorial.html#autotoc_md155", null ],
          [ "Sliders", "imgui_tutorial.html#autotoc_md156", null ],
          [ "Input Fields", "imgui_tutorial.html#autotoc_md157", null ],
          [ "Checkboxes", "imgui_tutorial.html#autotoc_md158", null ],
          [ "Combo Boxes (Dropdowns)", "imgui_tutorial.html#autotoc_md159", null ],
          [ "Radio Buttons", "imgui_tutorial.html#autotoc_md160", null ]
        ] ],
        [ "Layout and Spacing", "imgui_tutorial.html#autotoc_md162", [
          [ "Same Line", "imgui_tutorial.html#autotoc_md163", null ],
          [ "Spacing", "imgui_tutorial.html#autotoc_md164", null ],
          [ "Columns", "imgui_tutorial.html#autotoc_md165", null ],
          [ "Groups", "imgui_tutorial.html#autotoc_md166", null ]
        ] ],
        [ "Windows and Panels", "imgui_tutorial.html#autotoc_md168", [
          [ "Basic Window", "imgui_tutorial.html#autotoc_md169", null ],
          [ "Window Flags", "imgui_tutorial.html#autotoc_md170", null ],
          [ "Child Windows (Scrollable Regions)", "imgui_tutorial.html#autotoc_md171", null ]
        ] ],
        [ "Advanced Widgets", "imgui_tutorial.html#autotoc_md173", [
          [ "Plotting", "imgui_tutorial.html#autotoc_md174", null ],
          [ "Progress Bars", "imgui_tutorial.html#autotoc_md175", null ],
          [ "Color Pickers", "imgui_tutorial.html#autotoc_md176", null ],
          [ "Images", "imgui_tutorial.html#autotoc_md177", null ],
          [ "Trees", "imgui_tutorial.html#autotoc_md178", null ],
          [ "Tabs", "imgui_tutorial.html#autotoc_md179", null ]
        ] ],
        [ "Styling and Theming", "imgui_tutorial.html#autotoc_md181", [
          [ "Colors", "imgui_tutorial.html#autotoc_md182", null ],
          [ "Sizes and Spacing", "imgui_tutorial.html#autotoc_md183", null ],
          [ "Fonts", "imgui_tutorial.html#autotoc_md184", null ]
        ] ],
        [ "Docking", "imgui_tutorial.html#autotoc_md186", [
          [ "Enable Docking", "imgui_tutorial.html#autotoc_md187", null ],
          [ "Dockspace Example", "imgui_tutorial.html#autotoc_md188", null ]
        ] ],
        [ "Input Handling", "imgui_tutorial.html#autotoc_md190", [
          [ "Mouse", "imgui_tutorial.html#autotoc_md191", null ],
          [ "Keyboard", "imgui_tutorial.html#autotoc_md192", null ],
          [ "Capturing Input", "imgui_tutorial.html#autotoc_md193", null ]
        ] ],
        [ "Custom Rendering", "imgui_tutorial.html#autotoc_md195", [
          [ "Draw Lists", "imgui_tutorial.html#autotoc_md196", null ]
        ] ],
        [ "Panel Manager Pattern", "imgui_tutorial.html#autotoc_md198", [
          [ "Base Panel Interface", "imgui_tutorial.html#autotoc_md199", null ],
          [ "Implementing a Panel", "imgui_tutorial.html#autotoc_md200", null ],
          [ "Panel Manager", "imgui_tutorial.html#autotoc_md201", null ],
          [ "Usage", "imgui_tutorial.html#autotoc_md202", null ]
        ] ],
        [ "Common Patterns", "imgui_tutorial.html#autotoc_md204", [
          [ "Read-Only Display", "imgui_tutorial.html#autotoc_md205", null ],
          [ "Conditional UI", "imgui_tutorial.html#autotoc_md206", null ],
          [ "Tooltips", "imgui_tutorial.html#autotoc_md207", null ]
        ] ],
        [ "Performance Tips", "imgui_tutorial.html#autotoc_md209", [
          [ "1. Minimize State Changes", "imgui_tutorial.html#autotoc_md210", null ],
          [ "2. Collapse Unused Panels", "imgui_tutorial.html#autotoc_md211", null ],
          [ "3. Limit PlotLines Data", "imgui_tutorial.html#autotoc_md212", null ]
        ] ],
        [ "Debugging ImGui", "imgui_tutorial.html#autotoc_md214", [
          [ "Demo Window", "imgui_tutorial.html#autotoc_md215", null ],
          [ "Metrics Window", "imgui_tutorial.html#autotoc_md216", null ],
          [ "Debug Log", "imgui_tutorial.html#autotoc_md217", null ]
        ] ],
        [ "Next Steps", "imgui_tutorial.html#autotoc_md219", null ],
        [ "Resources", "imgui_tutorial.html#autotoc_md220", null ]
      ] ]
    ] ],
    [ "How to Navigate the Documentation", "md_docs_NAVIGATION.html", [
      [ "Quick Access to Architecture Guide", "md_docs_NAVIGATION.html#autotoc_md246", [
        [ "Method 1: From the Main Page (Easiest)", "md_docs_NAVIGATION.html#autotoc_md247", null ],
        [ "Method 2: From the Top Navigation", "md_docs_NAVIGATION.html#autotoc_md248", null ],
        [ "Method 3: Direct Link", "md_docs_NAVIGATION.html#autotoc_md249", null ]
      ] ],
      [ "What You'll Find in the Architecture Guide", "md_docs_NAVIGATION.html#autotoc_md251", [
        [ "Section 1: System Overview", "md_docs_NAVIGATION.html#autotoc_md252", null ],
        [ "Section 2: OpenGL Rendering Pipeline", "md_docs_NAVIGATION.html#autotoc_md253", null ],
        [ "Section 3: ImGui Integration", "md_docs_NAVIGATION.html#autotoc_md254", null ],
        [ "Section 4: Frame Execution Flow", "md_docs_NAVIGATION.html#autotoc_md255", null ],
        [ "Section 5: Class Interactions", "md_docs_NAVIGATION.html#autotoc_md256", null ],
        [ "Section 6: Data Flow", "md_docs_NAVIGATION.html#autotoc_md257", null ],
        [ "Section 7: Quick Reference", "md_docs_NAVIGATION.html#autotoc_md258", null ]
      ] ],
      [ "Navigation Tips", "md_docs_NAVIGATION.html#autotoc_md260", [
        [ "Finding Specific Information", "md_docs_NAVIGATION.html#autotoc_md261", null ],
        [ "Using the Table of Contents", "md_docs_NAVIGATION.html#autotoc_md262", null ],
        [ "Viewing Diagrams", "md_docs_NAVIGATION.html#autotoc_md263", null ]
      ] ],
      [ "Other Useful Pages", "md_docs_NAVIGATION.html#autotoc_md265", [
        [ "Main Page (<tt>index.html</tt>)", "md_docs_NAVIGATION.html#autotoc_md266", null ],
        [ "Class List (<tt>classes.html</tt>)", "md_docs_NAVIGATION.html#autotoc_md267", null ],
        [ "Class Hierarchy (<tt>hierarchy.html</tt>)", "md_docs_NAVIGATION.html#autotoc_md268", null ],
        [ "Files → File List", "md_docs_NAVIGATION.html#autotoc_md269", null ]
      ] ],
      [ "Diagram Examples in Architecture Guide", "md_docs_NAVIGATION.html#autotoc_md271", [
        [ "OpenGL Pipeline Diagram", "md_docs_NAVIGATION.html#autotoc_md272", null ],
        [ "ImGui Frame Rendering", "md_docs_NAVIGATION.html#autotoc_md273", null ],
        [ "Module Data Flow", "md_docs_NAVIGATION.html#autotoc_md274", null ],
        [ "Sequence Diagrams", "md_docs_NAVIGATION.html#autotoc_md275", null ]
      ] ],
      [ "For First-Time Readers", "md_docs_NAVIGATION.html#autotoc_md277", null ],
      [ "Common Questions", "md_docs_NAVIGATION.html#autotoc_md279", null ],
      [ "Shortcut: View Docs Script", "md_docs_NAVIGATION.html#autotoc_md281", null ]
    ] ],
    [ "opengl_imgui_integration", "md_docs_opengl_imgui_integration.html", null ],
    [ "OpenGL + ImGui Integration", "opengl_imgui_integration.html", [
      [ "OpenGL + ImGui Integration", "opengl_imgui_integration.html#autotoc_md283", [
        [ "The Challenge", "opengl_imgui_integration.html#autotoc_md284", null ],
        [ "Architecture Overview", "opengl_imgui_integration.html#autotoc_md286", null ],
        [ "Step-by-Step Integration", "opengl_imgui_integration.html#autotoc_md288", [
          [ "Step 1: Initialize Both Systems", "opengl_imgui_integration.html#autotoc_md289", null ],
          [ "Step 2: Create Off-Screen Render Target", "opengl_imgui_integration.html#autotoc_md291", null ],
          [ "Step 3: Render 3D Scene to Texture", "opengl_imgui_integration.html#autotoc_md293", null ],
          [ "Step 4: Display Texture in ImGui", "opengl_imgui_integration.html#autotoc_md295", null ],
          [ "Step 5: Render UI Panels", "opengl_imgui_integration.html#autotoc_md297", null ],
          [ "Step 6: Render ImGui Draw Data", "opengl_imgui_integration.html#autotoc_md299", null ]
        ] ],
        [ "Complete Frame Breakdown", "opengl_imgui_integration.html#autotoc_md301", null ],
        [ "Handling Input", "opengl_imgui_integration.html#autotoc_md303", [
          [ "Problem: Who Gets the Mouse?", "opengl_imgui_integration.html#autotoc_md304", null ],
          [ "Solution: Input Capture Flags", "opengl_imgui_integration.html#autotoc_md305", null ]
        ] ],
        [ "Viewport-Specific Input", "opengl_imgui_integration.html#autotoc_md307", null ],
        [ "Common Pitfalls", "opengl_imgui_integration.html#autotoc_md309", [
          [ "1. Texture Appears Upside-Down", "opengl_imgui_integration.html#autotoc_md310", null ],
          [ "2. Viewport Appears Black", "opengl_imgui_integration.html#autotoc_md311", null ],
          [ "3. UI Doesn't Respond to Input", "opengl_imgui_integration.html#autotoc_md312", null ],
          [ "4. Framebuffer Leak", "opengl_imgui_integration.html#autotoc_md313", null ]
        ] ],
        [ "Performance Considerations", "opengl_imgui_integration.html#autotoc_md315", [
          [ "1. Minimize FBO Recreations", "opengl_imgui_integration.html#autotoc_md316", null ],
          [ "2. Render Scene Only When Visible", "opengl_imgui_integration.html#autotoc_md317", null ],
          [ "3. Use Appropriate Texture Format", "opengl_imgui_integration.html#autotoc_md318", null ],
          [ "4. Limit Panel Updates", "opengl_imgui_integration.html#autotoc_md319", null ]
        ] ],
        [ "Advanced: Multiple Viewports", "opengl_imgui_integration.html#autotoc_md321", null ],
        [ "Debugging Integration Issues", "opengl_imgui_integration.html#autotoc_md323", [
          [ "1. Visualize FBO Contents", "opengl_imgui_integration.html#autotoc_md324", null ],
          [ "2. Check OpenGL State", "opengl_imgui_integration.html#autotoc_md325", null ],
          [ "3. Use RenderDoc", "opengl_imgui_integration.html#autotoc_md326", null ]
        ] ],
        [ "Summary", "opengl_imgui_integration.html#autotoc_md328", null ]
      ] ]
    ] ],
    [ "opengl_tutorial", "md_docs_opengl_tutorial.html", null ],
    [ "OpenGL Deep Dive Tutorial", "opengl_tutorial.html", [
      [ "OpenGL Deep Dive Tutorial", "opengl_tutorial.html#autotoc_md330", [
        [ "What is OpenGL?", "opengl_tutorial.html#autotoc_md331", [
          [ "Why Use OpenGL?", "opengl_tutorial.html#autotoc_md332", null ],
          [ "OpenGL in Our Application", "opengl_tutorial.html#autotoc_md333", null ]
        ] ],
        [ "The Mental Model: CPU ↔ GPU Pipeline", "opengl_tutorial.html#autotoc_md335", null ],
        [ "Core Concept 1: Vertices", "opengl_tutorial.html#autotoc_md337", [
          [ "What is a Vertex?", "opengl_tutorial.html#autotoc_md338", null ],
          [ "Example from <tt>renderer.cpp</tt>", "opengl_tutorial.html#autotoc_md339", null ]
        ] ],
        [ "Core Concept 2: VBO (Vertex Buffer Object)", "opengl_tutorial.html#autotoc_md341", [
          [ "What is a VBO?", "opengl_tutorial.html#autotoc_md342", null ],
          [ "Why Use VBOs?", "opengl_tutorial.html#autotoc_md343", null ],
          [ "Creating a VBO in Our Code", "opengl_tutorial.html#autotoc_md344", null ],
          [ "Buffer Usage Hints", "opengl_tutorial.html#autotoc_md345", null ]
        ] ],
        [ "Core Concept 3: VAO (Vertex Array Object)", "opengl_tutorial.html#autotoc_md347", [
          [ "What is a VAO?", "opengl_tutorial.html#autotoc_md348", null ],
          [ "Analogy", "opengl_tutorial.html#autotoc_md349", null ],
          [ "Creating a VAO in Our Code", "opengl_tutorial.html#autotoc_md350", null ],
          [ "Understanding Stride and Offset", "opengl_tutorial.html#autotoc_md351", null ]
        ] ],
        [ "Core Concept 4: EBO (Element Buffer Object)", "opengl_tutorial.html#autotoc_md353", [
          [ "What is an EBO?", "opengl_tutorial.html#autotoc_md354", null ],
          [ "The Problem: Duplicate Vertices", "opengl_tutorial.html#autotoc_md355", null ],
          [ "The Solution: Indexed Drawing", "opengl_tutorial.html#autotoc_md356", null ],
          [ "Creating an EBO", "opengl_tutorial.html#autotoc_md357", null ],
          [ "Drawing with Indices", "opengl_tutorial.html#autotoc_md358", null ]
        ] ],
        [ "Core Concept 5: Shaders", "opengl_tutorial.html#autotoc_md360", [
          [ "What are Shaders?", "opengl_tutorial.html#autotoc_md361", null ],
          [ "Two Required Shaders", "opengl_tutorial.html#autotoc_md362", null ],
          [ "Vertex Shader Example (from <tt>shaders/vertex_shader.glsl</tt>)", "opengl_tutorial.html#autotoc_md363", null ],
          [ "Fragment Shader Example (from <tt>shaders/fragment_shader.glsl</tt>)", "opengl_tutorial.html#autotoc_md364", null ],
          [ "Compiling Shaders in C++", "opengl_tutorial.html#autotoc_md365", null ]
        ] ],
        [ "Core Concept 6: Uniforms", "opengl_tutorial.html#autotoc_md367", [
          [ "What are Uniforms?", "opengl_tutorial.html#autotoc_md368", null ],
          [ "Setting Uniforms in Our Code", "opengl_tutorial.html#autotoc_md369", null ],
          [ "Uniform Types", "opengl_tutorial.html#autotoc_md370", null ]
        ] ],
        [ "Core Concept 7: Coordinate Transformations", "opengl_tutorial.html#autotoc_md372", [
          [ "The Four Coordinate Spaces", "opengl_tutorial.html#autotoc_md373", null ],
          [ "1. Local Space (Model Space)", "opengl_tutorial.html#autotoc_md374", null ],
          [ "2. World Space", "opengl_tutorial.html#autotoc_md375", null ],
          [ "3. View Space (Camera Space)", "opengl_tutorial.html#autotoc_md376", null ],
          [ "4. Clip Space (NDC - Normalized Device Coordinates)", "opengl_tutorial.html#autotoc_md377", null ]
        ] ],
        [ "Core Concept 8: FBO (Framebuffer Object)", "opengl_tutorial.html#autotoc_md379", [
          [ "What is an FBO?", "opengl_tutorial.html#autotoc_md380", null ],
          [ "Why Use FBOs?", "opengl_tutorial.html#autotoc_md381", null ],
          [ "Creating an FBO in Our Code", "opengl_tutorial.html#autotoc_md382", null ],
          [ "Rendering to an FBO", "opengl_tutorial.html#autotoc_md383", null ]
        ] ],
        [ "Complete Rendering Loop", "opengl_tutorial.html#autotoc_md385", null ],
        [ "Common Pitfalls and Solutions", "opengl_tutorial.html#autotoc_md387", [
          [ "Problem: Black Screen", "opengl_tutorial.html#autotoc_md388", null ],
          [ "Problem: Vertices in Wrong Position", "opengl_tutorial.html#autotoc_md389", null ],
          [ "Problem: Memory Leaks", "opengl_tutorial.html#autotoc_md390", null ]
        ] ],
        [ "Performance Tips", "opengl_tutorial.html#autotoc_md392", [
          [ "1. Minimize State Changes", "opengl_tutorial.html#autotoc_md393", null ],
          [ "2. Use Instancing for Repeated Objects", "opengl_tutorial.html#autotoc_md394", null ],
          [ "3. Batch Similar Objects", "opengl_tutorial.html#autotoc_md395", null ]
        ] ],
        [ "Debugging Tools", "opengl_tutorial.html#autotoc_md397", [
          [ "1. glGetError()", "opengl_tutorial.html#autotoc_md398", null ],
          [ "2. Shader Info Log", "opengl_tutorial.html#autotoc_md399", null ],
          [ "3. RenderDoc", "opengl_tutorial.html#autotoc_md400", null ]
        ] ],
        [ "Next Steps", "opengl_tutorial.html#autotoc_md402", null ],
        [ "Resources", "opengl_tutorial.html#autotoc_md403", null ]
      ] ]
    ] ],
    [ "pixel_truth", "md_docs_pixel_truth.html", null ],
    [ "The Pixel Truth: Everything is Just Computing RGBA", "pixel_truth.html", null ],
    [ "Documentation Guide", "md_docs_README.html", [
      [ "Generating Documentation", "md_docs_README.html#autotoc_md406", [
        [ "Prerequisites", "md_docs_README.html#autotoc_md407", null ],
        [ "Build Documentation", "md_docs_README.html#autotoc_md408", null ],
        [ "View Documentation", "md_docs_README.html#autotoc_md409", null ]
      ] ],
      [ "Documentation Structure", "md_docs_README.html#autotoc_md410", null ],
      [ "Documentation Features", "md_docs_README.html#autotoc_md411", [
        [ "1. Interactive Class Diagrams", "md_docs_README.html#autotoc_md412", null ],
        [ "2. Call Graphs", "md_docs_README.html#autotoc_md413", null ],
        [ "3. Source Code Browser", "md_docs_README.html#autotoc_md414", null ],
        [ "4. Visual Diagrams in Comments", "md_docs_README.html#autotoc_md415", null ],
        [ "5. Search Functionality", "md_docs_README.html#autotoc_md416", null ]
      ] ],
      [ "Key Pages to Start With", "md_docs_README.html#autotoc_md417", null ],
      [ "Diagram Types", "md_docs_README.html#autotoc_md418", [
        [ "Class Diagrams", "md_docs_README.html#autotoc_md419", null ],
        [ "Collaboration Diagrams", "md_docs_README.html#autotoc_md420", null ],
        [ "Call Graphs", "md_docs_README.html#autotoc_md421", null ]
      ] ],
      [ "Tips for Navigating", "md_docs_README.html#autotoc_md422", [
        [ "For Beginners", "md_docs_README.html#autotoc_md423", null ],
        [ "For Understanding Data Flow", "md_docs_README.html#autotoc_md424", null ],
        [ "For Understanding Rendering", "md_docs_README.html#autotoc_md425", null ],
        [ "For Adding Features", "md_docs_README.html#autotoc_md426", null ]
      ] ],
      [ "Customizing Documentation", "md_docs_README.html#autotoc_md427", null ],
      [ "Updating Documentation", "md_docs_README.html#autotoc_md428", null ],
      [ "Troubleshooting", "md_docs_README.html#autotoc_md429", [
        [ "\"Doxygen not found\"", "md_docs_README.html#autotoc_md430", null ],
        [ "\"Graphs not generating\"", "md_docs_README.html#autotoc_md431", null ],
        [ "\"Call graphs missing\"", "md_docs_README.html#autotoc_md432", null ],
        [ "\"Class diagrams missing\"", "md_docs_README.html#autotoc_md433", null ],
        [ "\"Documentation incomplete\"", "md_docs_README.html#autotoc_md434", null ]
      ] ],
      [ "Resources", "md_docs_README.html#autotoc_md435", null ]
    ] ],
    [ "Web Development Analogy", "web_analogy.html", [
      [ "Overview", "web_analogy.html#autotoc_md437", null ],
      [ "The Three Pillars: HTML ↔ CSS ↔ JavaScript", "web_analogy.html#autotoc_md439", null ],
      [ "1. HTML = Structure (ImGui Widgets)", "web_analogy.html#autotoc_md441", [
        [ "Web Markup", "web_analogy.html#autotoc_md442", null ],
        [ "AeroDynControlRig Equivalent", "web_analogy.html#autotoc_md443", null ],
        [ "Key Equivalences", "web_analogy.html#autotoc_md444", null ],
        [ "Custom Components (Like React Components)", "web_analogy.html#autotoc_md445", null ]
      ] ],
      [ "2. CSS = Styling (Theme System)", "web_analogy.html#autotoc_md447", [
        [ "Web Stylesheet", "web_analogy.html#autotoc_md448", null ],
        [ "AeroDynControlRig Equivalent", "web_analogy.html#autotoc_md449", null ],
        [ "CSS-Like Utilities", "web_analogy.html#autotoc_md450", null ],
        [ "Typography System", "web_analogy.html#autotoc_md451", null ]
      ] ],
      [ "3. JavaScript = Interactivity (Immediate Mode)", "web_analogy.html#autotoc_md453", [
        [ "Web Event Handling", "web_analogy.html#autotoc_md454", null ],
        [ "AeroDynControlRig Equivalent (Immediate Mode)", "web_analogy.html#autotoc_md455", null ],
        [ "Data Binding (Like Vue/React)", "web_analogy.html#autotoc_md456", null ]
      ] ],
      [ "4. Component Architecture (React-Like Panels)", "web_analogy.html#autotoc_md458", [
        [ "React Component", "web_analogy.html#autotoc_md459", null ],
        [ "AeroDynControlRig Panel", "web_analogy.html#autotoc_md460", null ],
        [ "Panel Registration (Like React Router)", "web_analogy.html#autotoc_md461", null ]
      ] ],
      [ "5. Layout System (Flexbox/Grid-Like)", "web_analogy.html#autotoc_md463", [
        [ "Web Flexbox", "web_analogy.html#autotoc_md464", null ],
        [ "ImGui Equivalent", "web_analogy.html#autotoc_md465", null ],
        [ "Dockspace = CSS Grid", "web_analogy.html#autotoc_md466", null ]
      ] ],
      [ "6. State Management (Vuex/Redux-Like)", "web_analogy.html#autotoc_md468", [
        [ "Redux Store", "web_analogy.html#autotoc_md469", null ],
        [ "SimulationState (Global State)", "web_analogy.html#autotoc_md470", null ]
      ] ],
      [ "7. Comparison Table", "web_analogy.html#autotoc_md472", null ],
      [ "8. Code-to-Code Translation Examples", "web_analogy.html#autotoc_md474", [
        [ "Example 1: Button with Conditional Text", "web_analogy.html#autotoc_md475", null ],
        [ "Example 2: Styled Container with Header", "web_analogy.html#autotoc_md476", null ],
        [ "Example 3: Color Badge Based on Condition", "web_analogy.html#autotoc_md477", null ]
      ] ],
      [ "9. Best Practices (Web → ImGui)", "web_analogy.html#autotoc_md479", [
        [ "✅ DO: Think in Components", "web_analogy.html#autotoc_md480", null ],
        [ "✅ DO: Use Design Tokens", "web_analogy.html#autotoc_md481", null ],
        [ "✅ DO: Separate Logic from Presentation", "web_analogy.html#autotoc_md482", null ],
        [ "❌ DON'T: Mix Styles Inconsistently", "web_analogy.html#autotoc_md483", null ]
      ] ],
      [ "10. Summary", "web_analogy.html#autotoc_md485", null ],
      [ "Related Documentation", "web_analogy.html#autotoc_md487", null ]
    ] ],
    [ "AeroDynControlRig", "md_README.html", [
      [ "Overview", "md_README.html#autotoc_md489", null ],
      [ "Directory Layout", "md_README.html#autotoc_md490", null ],
      [ "Getting Started", "md_README.html#autotoc_md491", null ],
      [ "Current Features", "md_README.html#autotoc_md492", null ],
      [ "Roadmap", "md_README.html#autotoc_md493", null ],
      [ "Long-Term Goals", "md_README.html#autotoc_md494", null ],
      [ "Ecosystem Map", "md_README.html#autotoc_md495", null ],
      [ "UI Preview", "md_README.html#autotoc_md496", null ]
    ] ],
    [ "Modules", "modules.html", "modules" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"euler_8c.html#ad774b88fd06267fc2a237871fc635362",
"md_docs_NAVIGATION.html#autotoc_md269",
"structSimulationState_1_1PowerHistory.html#a0fbf98c90c75967a6cac1952c390c725"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';