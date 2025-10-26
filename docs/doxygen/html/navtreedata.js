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
      [ "What is AeroDynControlRig?", "index.html#autotoc_md246", null ],
      [ "Quick Start", "index.html#autotoc_md247", null ],
      [ "Architecture Overview", "index.html#autotoc_md248", null ],
      [ "Essential Reading", "index.html#autotoc_md249", null ],
      [ "🌟 New: Web Developer? You Already Know This!", "index.html#autotoc_md250", [
        [ "Quick Translation Guide", "index.html#autotoc_md251", null ]
      ] ],
      [ "Key Concepts", "index.html#autotoc_md253", [
        [ "OpenGL Rendering Pipeline", "index.html#autotoc_md254", null ],
        [ "ImGui Immediate Mode", "index.html#autotoc_md255", null ],
        [ "Module System", "index.html#autotoc_md256", null ],
        [ "Panel System", "index.html#autotoc_md257", null ]
      ] ],
      [ "Learning Path", "index.html#autotoc_md258", null ],
      [ "Class Categories", "index.html#autotoc_md259", [
        [ "Core Infrastructure", "index.html#autotoc_md260", null ],
        [ "Rendering", "index.html#autotoc_md261", null ],
        [ "Simulation Modules", "index.html#autotoc_md262", null ],
        [ "UI Panels", "index.html#autotoc_md263", null ]
      ] ],
      [ "Development Roadmap", "index.html#autotoc_md264", null ],
      [ "Getting Help", "index.html#autotoc_md265", null ],
      [ "Contributing", "index.html#autotoc_md266", null ]
    ] ],
    [ "Complete System Architecture", "architecture.html", [
      [ "Overview", "architecture.html#arch_overview", null ],
      [ "The Complete Pipeline", "architecture.html#arch_pipeline", null ],
      [ "OpenGL Buffer Objects", "architecture.html#arch_opengl_buffers", null ],
      [ "Rotation Transformations", "architecture.html#arch_transformations", null ],
      [ "Shader Pipeline", "architecture.html#arch_shaders", null ],
      [ "Memory Layout", "architecture.html#arch_memory", null ],
      [ "ImGui Integration", "architecture.html#arch_imgui", null ],
      [ "Module Architecture", "architecture.html#arch_modules", null ],
      [ "Data Flow Diagram", "architecture.html#arch_dataflow", null ],
      [ "Performance Characteristics", "architecture.html#arch_performance", null ],
      [ "Hardware Pipeline", "architecture.html#arch_hardware", null ],
      [ "Debugging Tips", "architecture.html#arch_debugging", null ],
      [ "Further Reading", "architecture.html#arch_references", null ],
      [ "AeroDynControlRig Architecture Guide", "architecture.html#autotoc_md23", [
        [ "Table of Contents", "architecture.html#autotoc_md24", null ],
        [ "System Overview", "architecture.html#autotoc_md26", [
          [ "From Keyboard to Photons: The Journey", "architecture.html#autotoc_md0", null ],
          [ "VBO (Vertex Buffer Object) - The Data", "architecture.html#autotoc_md1", null ],
          [ "VAO (Vertex Array Object) - The Recipe", "architecture.html#autotoc_md2", null ],
          [ "EBO (Element Buffer Object) - The Connections", "architecture.html#autotoc_md3", null ],
          [ "Drawing with VAO/VBO/EBO", "architecture.html#autotoc_md4", null ],
          [ "Why Three Representations?", "architecture.html#autotoc_md5", [
            [ "Quaternion (Internal Representation)", "architecture.html#autotoc_md6", null ],
            [ "DCM (Direction Cosine Matrix - 3×3)", "architecture.html#autotoc_md7", null ],
            [ "OpenGL Matrix (4×4)", "architecture.html#autotoc_md8", null ]
          ] ],
          [ "Conversion Code", "architecture.html#autotoc_md9", null ],
          [ "Vertex Shader (runs once per vertex)", "architecture.html#autotoc_md10", null ],
          [ "Fragment Shader (runs once per pixel)", "architecture.html#autotoc_md11", null ],
          [ "CPU RAM", "architecture.html#autotoc_md12", null ],
          [ "GPU VRAM", "architecture.html#autotoc_md13", null ],
          [ "Bandwidth Usage", "architecture.html#autotoc_md14", null ],
          [ "How ImGui Works", "architecture.html#autotoc_md15", null ],
          [ "Update Loop", "architecture.html#autotoc_md16", null ],
          [ "Module Execution Order", "architecture.html#autotoc_md17", null ],
          [ "Bottlenecks", "architecture.html#autotoc_md18", null ],
          [ "Optimizations", "architecture.html#autotoc_md19", null ],
          [ "GPU Architecture (Simplified)", "architecture.html#autotoc_md20", null ],
          [ "Common Issues", "architecture.html#autotoc_md21", null ],
          [ "Useful Debug Prints", "architecture.html#autotoc_md22", null ],
          [ "High-Level Architecture", "architecture.html#autotoc_md27", null ],
          [ "Technology Stack", "architecture.html#autotoc_md28", null ]
        ] ],
        [ "OpenGL Rendering Pipeline", "architecture.html#autotoc_md30", [
          [ "The Graphics Hardware Pipeline", "architecture.html#autotoc_md31", null ],
          [ "Key OpenGL Concepts", "architecture.html#autotoc_md32", [
            [ "1. Vertex Buffer Objects (VBO)", "architecture.html#autotoc_md33", null ],
            [ "2. Vertex Array Objects (VAO)", "architecture.html#autotoc_md34", null ],
            [ "3. Element Buffer Objects (EBO)", "architecture.html#autotoc_md35", null ],
            [ "4. Shaders (GLSL)", "architecture.html#autotoc_md36", null ],
            [ "5. Framebuffer Objects (FBO)", "architecture.html#autotoc_md37", null ]
          ] ],
          [ "Coordinate Transformations", "architecture.html#autotoc_md38", null ]
        ] ],
        [ "ImGui Integration", "architecture.html#autotoc_md40", [
          [ "Immediate Mode GUI Paradigm", "architecture.html#autotoc_md41", null ],
          [ "ImGui Rendering Flow", "architecture.html#autotoc_md42", null ],
          [ "ImGui + OpenGL Interaction", "architecture.html#autotoc_md43", null ]
        ] ],
        [ "Frame Execution Flow", "architecture.html#autotoc_md45", [
          [ "Main Loop Sequence Diagram", "architecture.html#autotoc_md46", null ],
          [ "Timing Diagram", "architecture.html#autotoc_md47", null ]
        ] ],
        [ "Class Interactions", "architecture.html#autotoc_md49", [
          [ "Module System", "architecture.html#autotoc_md50", null ],
          [ "Renderer Hierarchy", "architecture.html#autotoc_md51", null ],
          [ "Panel System", "architecture.html#autotoc_md52", null ]
        ] ],
        [ "Data Flow", "architecture.html#autotoc_md54", [
          [ "State Updates", "architecture.html#autotoc_md55", null ],
          [ "Simulation Data Flow", "architecture.html#autotoc_md56", null ],
          [ "Rendering Data Flow", "architecture.html#autotoc_md57", null ]
        ] ],
        [ "Quick Reference: Key Files", "architecture.html#autotoc_md59", null ],
        [ "Debugging Tips", "architecture.html#autotoc_md61", [
          [ "OpenGL Debugging", "architecture.html#autotoc_md62", null ],
          [ "ImGui Debugging", "architecture.html#autotoc_md63", null ]
        ] ],
        [ "Next Steps for Learning", "architecture.html#autotoc_md65", null ]
      ] ]
    ] ],
    [ "architecture", "md_docs_architecture.html", null ],
    [ "complete_picture", "md_docs_complete_picture.html", null ],
    [ "The Complete Picture: From Your Intent to Pixels on Screen", "complete_picture.html", null ],
    [ "display_fundamentals", "md_docs_display_fundamentals.html", null ],
    [ "Display Fundamentals: Pixels, Framebuffers, and the OS", "display_fundamentals.html", null ],
    [ "Documentation Update Summary", "md_docs_DOCUMENTATION_UPDATE.html", [
      [ "What Was Created", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md68", [
        [ "1. <strong>Web Analogy Guide</strong> (<tt>docs/web_analogy.md</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md69", null ],
        [ "2. <strong>GPU Pipeline Guide</strong> (<tt>docs/gpu_pipeline.md</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md70", null ],
        [ "3. <strong>Doxygen Module Groups</strong> (<tt>docs/groups.dox</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md71", null ],
        [ "4. <strong>Enhanced Header Documentation</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md72", [
          [ "<tt>src/gui/widgets/card.h</tt>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md73", null ],
          [ "<tt>src/gui/style.h</tt>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md74", null ]
        ] ],
        [ "5. <strong>Updated Main Documentation</strong> (<tt>docs/mainpage.md</tt>)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md75", null ]
      ] ],
      [ "How to View the Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md76", [
        [ "Option 1: Generate HTML Docs (Recommended)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md77", null ],
        [ "Option 2: Read Markdown Directly", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md78", null ]
      ] ],
      [ "Key Features", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md79", [
        [ "1. Web Analogy Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md80", null ],
        [ "2. Hardware Pipeline Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md81", null ],
        [ "3. Cross-Referenced Examples", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md82", null ]
      ] ],
      [ "File Changes Summary", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md83", null ],
      [ "What This Achieves", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md84", [
        [ "1. <strong>Lowers Barrier to Entry</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md85", null ],
        [ "2. <strong>Explains the \"Why\"</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md86", null ],
        [ "3. <strong>Demystifies the Hardware</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md87", null ],
        [ "4. <strong>Makes Design Decisions Clear</strong>", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md88", null ]
      ] ],
      [ "Example Documentation Snippets", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md89", [
        [ "Color Palette Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md90", null ],
        [ "Font Loading Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md91", null ]
      ] ],
      [ "How to Use This Documentation", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md92", [
        [ "For New Contributors", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md93", null ],
        [ "For Yourself (Future Reference)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md94", null ],
        [ "For Teaching Others", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md95", null ]
      ] ],
      [ "Next Steps (Optional)", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md96", null ],
      [ "Generating the Docs", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md97", null ],
      [ "Documentation Quality", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md98", [
        [ "Coverage", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md99", null ],
        [ "Depth", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md100", null ],
        [ "Accessibility", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md101", null ]
      ] ],
      [ "Summary", "md_docs_DOCUMENTATION_UPDATE.html#autotoc_md103", null ]
    ] ],
    [ "GPU Pipeline: From RAM to Screen", "gpu_pipeline.html", [
      [ "Overview", "gpu_pipeline.html#autotoc_md104", null ],
      [ "The Complete Pipeline", "gpu_pipeline.html#autotoc_md106", null ],
      [ "Stage 1: CPU Side - Your C++ Code", "gpu_pipeline.html#autotoc_md108", [
        [ "1.1 Application Update Loop", "gpu_pipeline.html#autotoc_md109", null ],
        [ "1.2 Module Updates (Pure CPU Work)", "gpu_pipeline.html#autotoc_md111", null ],
        [ "1.3 ImGui Vertex Generation", "gpu_pipeline.html#autotoc_md113", null ],
        [ "1.4 OpenGL Command Submission", "gpu_pipeline.html#autotoc_md115", null ]
      ] ],
      [ "Stage 2: PCIe Bus Transfer", "gpu_pipeline.html#autotoc_md117", [
        [ "2.1 CPU → GPU Communication", "gpu_pipeline.html#autotoc_md118", null ]
      ] ],
      [ "Stage 3: GPU Side - Graphics Card", "gpu_pipeline.html#autotoc_md120", [
        [ "3.1 GPU Architecture Overview", "gpu_pipeline.html#autotoc_md121", null ],
        [ "3.2 GPU Memory Hierarchy", "gpu_pipeline.html#autotoc_md123", null ],
        [ "3.3 Vertex Shader Execution", "gpu_pipeline.html#autotoc_md125", null ],
        [ "3.4 Rasterization (Fixed-Function Hardware)", "gpu_pipeline.html#autotoc_md127", null ],
        [ "3.5 Fragment Shader Execution", "gpu_pipeline.html#autotoc_md129", null ],
        [ "3.6 Framebuffer Operations", "gpu_pipeline.html#autotoc_md131", null ]
      ] ],
      [ "Stage 4: Display Output", "gpu_pipeline.html#autotoc_md133", [
        [ "4.1 Swap Buffers", "gpu_pipeline.html#autotoc_md134", null ],
        [ "4.2 Display Controller Scanout", "gpu_pipeline.html#autotoc_md136", null ]
      ] ],
      [ "Complete Frame Timing Breakdown", "gpu_pipeline.html#autotoc_md138", null ],
      [ "Memory Bandwidth Analysis", "gpu_pipeline.html#autotoc_md140", [
        [ "CPU Side (System RAM)", "gpu_pipeline.html#autotoc_md141", null ],
        [ "PCIe Bus (CPU ↔ GPU)", "gpu_pipeline.html#autotoc_md142", null ],
        [ "GPU Side (VRAM)", "gpu_pipeline.html#autotoc_md143", null ]
      ] ],
      [ "Optimization Insights", "gpu_pipeline.html#autotoc_md145", [
        [ "Where Time Is Actually Spent", "gpu_pipeline.html#autotoc_md146", null ],
        [ "If You Wanted to Optimize", "gpu_pipeline.html#autotoc_md147", null ]
      ] ],
      [ "Key Takeaways", "gpu_pipeline.html#autotoc_md149", null ],
      [ "Mapping to AeroDynControlRig Code", "gpu_pipeline.html#autotoc_md151", null ],
      [ "Related Documentation", "gpu_pipeline.html#autotoc_md153", null ],
      [ "Further Reading", "gpu_pipeline.html#autotoc_md155", null ]
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
      [ "ImGui Deep Dive Tutorial", "imgui_tutorial.html#autotoc_md162", [
        [ "What is ImGui?", "imgui_tutorial.html#autotoc_md163", [
          [ "Why ImGui for Our Application?", "imgui_tutorial.html#autotoc_md164", null ]
        ] ],
        [ "The Big Idea: Immediate Mode", "imgui_tutorial.html#autotoc_md166", [
          [ "Traditional GUI (Retained Mode)", "imgui_tutorial.html#autotoc_md167", null ],
          [ "ImGui (Immediate Mode)", "imgui_tutorial.html#autotoc_md168", null ]
        ] ],
        [ "Setup and Initialization", "imgui_tutorial.html#autotoc_md170", [
          [ "From <tt>application.cpp init()</tt>", "imgui_tutorial.html#autotoc_md171", null ],
          [ "What Each Backend Does", "imgui_tutorial.html#autotoc_md172", null ]
        ] ],
        [ "Frame Structure", "imgui_tutorial.html#autotoc_md174", null ],
        [ "Core Widgets", "imgui_tutorial.html#autotoc_md176", [
          [ "Buttons", "imgui_tutorial.html#autotoc_md177", null ],
          [ "Text", "imgui_tutorial.html#autotoc_md178", null ],
          [ "Sliders", "imgui_tutorial.html#autotoc_md179", null ],
          [ "Input Fields", "imgui_tutorial.html#autotoc_md180", null ],
          [ "Checkboxes", "imgui_tutorial.html#autotoc_md181", null ],
          [ "Combo Boxes (Dropdowns)", "imgui_tutorial.html#autotoc_md182", null ],
          [ "Radio Buttons", "imgui_tutorial.html#autotoc_md183", null ]
        ] ],
        [ "Layout and Spacing", "imgui_tutorial.html#autotoc_md185", [
          [ "Same Line", "imgui_tutorial.html#autotoc_md186", null ],
          [ "Spacing", "imgui_tutorial.html#autotoc_md187", null ],
          [ "Columns", "imgui_tutorial.html#autotoc_md188", null ],
          [ "Groups", "imgui_tutorial.html#autotoc_md189", null ]
        ] ],
        [ "Windows and Panels", "imgui_tutorial.html#autotoc_md191", [
          [ "Basic Window", "imgui_tutorial.html#autotoc_md192", null ],
          [ "Window Flags", "imgui_tutorial.html#autotoc_md193", null ],
          [ "Child Windows (Scrollable Regions)", "imgui_tutorial.html#autotoc_md194", null ]
        ] ],
        [ "Advanced Widgets", "imgui_tutorial.html#autotoc_md196", [
          [ "Plotting", "imgui_tutorial.html#autotoc_md197", null ],
          [ "Progress Bars", "imgui_tutorial.html#autotoc_md198", null ],
          [ "Color Pickers", "imgui_tutorial.html#autotoc_md199", null ],
          [ "Images", "imgui_tutorial.html#autotoc_md200", null ],
          [ "Trees", "imgui_tutorial.html#autotoc_md201", null ],
          [ "Tabs", "imgui_tutorial.html#autotoc_md202", null ]
        ] ],
        [ "Styling and Theming", "imgui_tutorial.html#autotoc_md204", [
          [ "Colors", "imgui_tutorial.html#autotoc_md205", null ],
          [ "Sizes and Spacing", "imgui_tutorial.html#autotoc_md206", null ],
          [ "Fonts", "imgui_tutorial.html#autotoc_md207", null ]
        ] ],
        [ "Docking", "imgui_tutorial.html#autotoc_md209", [
          [ "Enable Docking", "imgui_tutorial.html#autotoc_md210", null ],
          [ "Dockspace Example", "imgui_tutorial.html#autotoc_md211", null ]
        ] ],
        [ "Input Handling", "imgui_tutorial.html#autotoc_md213", [
          [ "Mouse", "imgui_tutorial.html#autotoc_md214", null ],
          [ "Keyboard", "imgui_tutorial.html#autotoc_md215", null ],
          [ "Capturing Input", "imgui_tutorial.html#autotoc_md216", null ]
        ] ],
        [ "Custom Rendering", "imgui_tutorial.html#autotoc_md218", [
          [ "Draw Lists", "imgui_tutorial.html#autotoc_md219", null ]
        ] ],
        [ "Panel Manager Pattern", "imgui_tutorial.html#autotoc_md221", [
          [ "Base Panel Interface", "imgui_tutorial.html#autotoc_md222", null ],
          [ "Implementing a Panel", "imgui_tutorial.html#autotoc_md223", null ],
          [ "Panel Manager", "imgui_tutorial.html#autotoc_md224", null ],
          [ "Usage", "imgui_tutorial.html#autotoc_md225", null ]
        ] ],
        [ "Common Patterns", "imgui_tutorial.html#autotoc_md227", [
          [ "Read-Only Display", "imgui_tutorial.html#autotoc_md228", null ],
          [ "Conditional UI", "imgui_tutorial.html#autotoc_md229", null ],
          [ "Tooltips", "imgui_tutorial.html#autotoc_md230", null ]
        ] ],
        [ "Performance Tips", "imgui_tutorial.html#autotoc_md232", [
          [ "1. Minimize State Changes", "imgui_tutorial.html#autotoc_md233", null ],
          [ "2. Collapse Unused Panels", "imgui_tutorial.html#autotoc_md234", null ],
          [ "3. Limit PlotLines Data", "imgui_tutorial.html#autotoc_md235", null ]
        ] ],
        [ "Debugging ImGui", "imgui_tutorial.html#autotoc_md237", [
          [ "Demo Window", "imgui_tutorial.html#autotoc_md238", null ],
          [ "Metrics Window", "imgui_tutorial.html#autotoc_md239", null ],
          [ "Debug Log", "imgui_tutorial.html#autotoc_md240", null ]
        ] ],
        [ "Next Steps", "imgui_tutorial.html#autotoc_md242", null ],
        [ "Resources", "imgui_tutorial.html#autotoc_md243", null ]
      ] ]
    ] ],
    [ "How to Navigate the Documentation", "md_docs_NAVIGATION.html", [
      [ "Quick Access to Architecture Guide", "md_docs_NAVIGATION.html#autotoc_md269", [
        [ "Method 1: From the Main Page (Easiest)", "md_docs_NAVIGATION.html#autotoc_md270", null ],
        [ "Method 2: From the Top Navigation", "md_docs_NAVIGATION.html#autotoc_md271", null ],
        [ "Method 3: Direct Link", "md_docs_NAVIGATION.html#autotoc_md272", null ]
      ] ],
      [ "What You'll Find in the Architecture Guide", "md_docs_NAVIGATION.html#autotoc_md274", [
        [ "Section 1: System Overview", "md_docs_NAVIGATION.html#autotoc_md275", null ],
        [ "Section 2: OpenGL Rendering Pipeline", "md_docs_NAVIGATION.html#autotoc_md276", null ],
        [ "Section 3: ImGui Integration", "md_docs_NAVIGATION.html#autotoc_md277", null ],
        [ "Section 4: Frame Execution Flow", "md_docs_NAVIGATION.html#autotoc_md278", null ],
        [ "Section 5: Class Interactions", "md_docs_NAVIGATION.html#autotoc_md279", null ],
        [ "Section 6: Data Flow", "md_docs_NAVIGATION.html#autotoc_md280", null ],
        [ "Section 7: Quick Reference", "md_docs_NAVIGATION.html#autotoc_md281", null ]
      ] ],
      [ "Navigation Tips", "md_docs_NAVIGATION.html#autotoc_md283", [
        [ "Finding Specific Information", "md_docs_NAVIGATION.html#autotoc_md284", null ],
        [ "Using the Table of Contents", "md_docs_NAVIGATION.html#autotoc_md285", null ],
        [ "Viewing Diagrams", "md_docs_NAVIGATION.html#autotoc_md286", null ]
      ] ],
      [ "Other Useful Pages", "md_docs_NAVIGATION.html#autotoc_md288", [
        [ "Main Page (<tt>index.html</tt>)", "md_docs_NAVIGATION.html#autotoc_md289", null ],
        [ "Class List (<tt>classes.html</tt>)", "md_docs_NAVIGATION.html#autotoc_md290", null ],
        [ "Class Hierarchy (<tt>hierarchy.html</tt>)", "md_docs_NAVIGATION.html#autotoc_md291", null ],
        [ "Files → File List", "md_docs_NAVIGATION.html#autotoc_md292", null ]
      ] ],
      [ "Diagram Examples in Architecture Guide", "md_docs_NAVIGATION.html#autotoc_md294", [
        [ "OpenGL Pipeline Diagram", "md_docs_NAVIGATION.html#autotoc_md295", null ],
        [ "ImGui Frame Rendering", "md_docs_NAVIGATION.html#autotoc_md296", null ],
        [ "Module Data Flow", "md_docs_NAVIGATION.html#autotoc_md297", null ],
        [ "Sequence Diagrams", "md_docs_NAVIGATION.html#autotoc_md298", null ]
      ] ],
      [ "For First-Time Readers", "md_docs_NAVIGATION.html#autotoc_md300", null ],
      [ "Common Questions", "md_docs_NAVIGATION.html#autotoc_md302", null ],
      [ "Shortcut: View Docs Script", "md_docs_NAVIGATION.html#autotoc_md304", null ]
    ] ],
    [ "opengl_imgui_integration", "md_docs_opengl_imgui_integration.html", null ],
    [ "OpenGL + ImGui Integration", "opengl_imgui_integration.html", [
      [ "OpenGL + ImGui Integration", "opengl_imgui_integration.html#autotoc_md306", [
        [ "The Challenge", "opengl_imgui_integration.html#autotoc_md307", null ],
        [ "Architecture Overview", "opengl_imgui_integration.html#autotoc_md309", null ],
        [ "Step-by-Step Integration", "opengl_imgui_integration.html#autotoc_md311", [
          [ "Step 1: Initialize Both Systems", "opengl_imgui_integration.html#autotoc_md312", null ],
          [ "Step 2: Create Off-Screen Render Target", "opengl_imgui_integration.html#autotoc_md314", null ],
          [ "Step 3: Render 3D Scene to Texture", "opengl_imgui_integration.html#autotoc_md316", null ],
          [ "Step 4: Display Texture in ImGui", "opengl_imgui_integration.html#autotoc_md318", null ],
          [ "Step 5: Render UI Panels", "opengl_imgui_integration.html#autotoc_md320", null ],
          [ "Step 6: Render ImGui Draw Data", "opengl_imgui_integration.html#autotoc_md322", null ]
        ] ],
        [ "Complete Frame Breakdown", "opengl_imgui_integration.html#autotoc_md324", null ],
        [ "Handling Input", "opengl_imgui_integration.html#autotoc_md326", [
          [ "Problem: Who Gets the Mouse?", "opengl_imgui_integration.html#autotoc_md327", null ],
          [ "Solution: Input Capture Flags", "opengl_imgui_integration.html#autotoc_md328", null ]
        ] ],
        [ "Viewport-Specific Input", "opengl_imgui_integration.html#autotoc_md330", null ],
        [ "Common Pitfalls", "opengl_imgui_integration.html#autotoc_md332", [
          [ "1. Texture Appears Upside-Down", "opengl_imgui_integration.html#autotoc_md333", null ],
          [ "2. Viewport Appears Black", "opengl_imgui_integration.html#autotoc_md334", null ],
          [ "3. UI Doesn't Respond to Input", "opengl_imgui_integration.html#autotoc_md335", null ],
          [ "4. Framebuffer Leak", "opengl_imgui_integration.html#autotoc_md336", null ]
        ] ],
        [ "Performance Considerations", "opengl_imgui_integration.html#autotoc_md338", [
          [ "1. Minimize FBO Recreations", "opengl_imgui_integration.html#autotoc_md339", null ],
          [ "2. Render Scene Only When Visible", "opengl_imgui_integration.html#autotoc_md340", null ],
          [ "3. Use Appropriate Texture Format", "opengl_imgui_integration.html#autotoc_md341", null ],
          [ "4. Limit Panel Updates", "opengl_imgui_integration.html#autotoc_md342", null ]
        ] ],
        [ "Advanced: Multiple Viewports", "opengl_imgui_integration.html#autotoc_md344", null ],
        [ "Debugging Integration Issues", "opengl_imgui_integration.html#autotoc_md346", [
          [ "1. Visualize FBO Contents", "opengl_imgui_integration.html#autotoc_md347", null ],
          [ "2. Check OpenGL State", "opengl_imgui_integration.html#autotoc_md348", null ],
          [ "3. Use RenderDoc", "opengl_imgui_integration.html#autotoc_md349", null ]
        ] ],
        [ "Summary", "opengl_imgui_integration.html#autotoc_md351", null ]
      ] ]
    ] ],
    [ "opengl_tutorial", "md_docs_opengl_tutorial.html", null ],
    [ "OpenGL Deep Dive Tutorial", "opengl_tutorial.html", [
      [ "OpenGL Deep Dive Tutorial", "opengl_tutorial.html#autotoc_md353", [
        [ "What is OpenGL?", "opengl_tutorial.html#autotoc_md354", [
          [ "Why Use OpenGL?", "opengl_tutorial.html#autotoc_md355", null ],
          [ "OpenGL in Our Application", "opengl_tutorial.html#autotoc_md356", null ]
        ] ],
        [ "The Mental Model: CPU ↔ GPU Pipeline", "opengl_tutorial.html#autotoc_md358", null ],
        [ "Core Concept 1: Vertices", "opengl_tutorial.html#autotoc_md360", [
          [ "What is a Vertex?", "opengl_tutorial.html#autotoc_md361", null ],
          [ "Example from <tt>renderer.cpp</tt>", "opengl_tutorial.html#autotoc_md362", null ]
        ] ],
        [ "Core Concept 2: VBO (Vertex Buffer Object)", "opengl_tutorial.html#autotoc_md364", [
          [ "What is a VBO?", "opengl_tutorial.html#autotoc_md365", null ],
          [ "Why Use VBOs?", "opengl_tutorial.html#autotoc_md366", null ],
          [ "Creating a VBO in Our Code", "opengl_tutorial.html#autotoc_md367", null ],
          [ "Buffer Usage Hints", "opengl_tutorial.html#autotoc_md368", null ]
        ] ],
        [ "Core Concept 3: VAO (Vertex Array Object)", "opengl_tutorial.html#autotoc_md370", [
          [ "What is a VAO?", "opengl_tutorial.html#autotoc_md371", null ],
          [ "Analogy", "opengl_tutorial.html#autotoc_md372", null ],
          [ "Creating a VAO in Our Code", "opengl_tutorial.html#autotoc_md373", null ],
          [ "Understanding Stride and Offset", "opengl_tutorial.html#autotoc_md374", null ]
        ] ],
        [ "Core Concept 4: EBO (Element Buffer Object)", "opengl_tutorial.html#autotoc_md376", [
          [ "What is an EBO?", "opengl_tutorial.html#autotoc_md377", null ],
          [ "The Problem: Duplicate Vertices", "opengl_tutorial.html#autotoc_md378", null ],
          [ "The Solution: Indexed Drawing", "opengl_tutorial.html#autotoc_md379", null ],
          [ "Creating an EBO", "opengl_tutorial.html#autotoc_md380", null ],
          [ "Drawing with Indices", "opengl_tutorial.html#autotoc_md381", null ]
        ] ],
        [ "Core Concept 5: Shaders", "opengl_tutorial.html#autotoc_md383", [
          [ "What are Shaders?", "opengl_tutorial.html#autotoc_md384", null ],
          [ "Two Required Shaders", "opengl_tutorial.html#autotoc_md385", null ],
          [ "Vertex Shader Example (from <tt>shaders/vertex_shader.glsl</tt>)", "opengl_tutorial.html#autotoc_md386", null ],
          [ "Fragment Shader Example (from <tt>shaders/fragment_shader.glsl</tt>)", "opengl_tutorial.html#autotoc_md387", null ],
          [ "Compiling Shaders in C++", "opengl_tutorial.html#autotoc_md388", null ]
        ] ],
        [ "Core Concept 6: Uniforms", "opengl_tutorial.html#autotoc_md390", [
          [ "What are Uniforms?", "opengl_tutorial.html#autotoc_md391", null ],
          [ "Setting Uniforms in Our Code", "opengl_tutorial.html#autotoc_md392", null ],
          [ "Uniform Types", "opengl_tutorial.html#autotoc_md393", null ]
        ] ],
        [ "Core Concept 7: Coordinate Transformations", "opengl_tutorial.html#autotoc_md395", [
          [ "The Four Coordinate Spaces", "opengl_tutorial.html#autotoc_md396", null ],
          [ "1. Local Space (Model Space)", "opengl_tutorial.html#autotoc_md397", null ],
          [ "2. World Space", "opengl_tutorial.html#autotoc_md398", null ],
          [ "3. View Space (Camera Space)", "opengl_tutorial.html#autotoc_md399", null ],
          [ "4. Clip Space (NDC - Normalized Device Coordinates)", "opengl_tutorial.html#autotoc_md400", null ]
        ] ],
        [ "Core Concept 8: FBO (Framebuffer Object)", "opengl_tutorial.html#autotoc_md402", [
          [ "What is an FBO?", "opengl_tutorial.html#autotoc_md403", null ],
          [ "Why Use FBOs?", "opengl_tutorial.html#autotoc_md404", null ],
          [ "Creating an FBO in Our Code", "opengl_tutorial.html#autotoc_md405", null ],
          [ "Rendering to an FBO", "opengl_tutorial.html#autotoc_md406", null ]
        ] ],
        [ "Complete Rendering Loop", "opengl_tutorial.html#autotoc_md408", null ],
        [ "Common Pitfalls and Solutions", "opengl_tutorial.html#autotoc_md410", [
          [ "Problem: Black Screen", "opengl_tutorial.html#autotoc_md411", null ],
          [ "Problem: Vertices in Wrong Position", "opengl_tutorial.html#autotoc_md412", null ],
          [ "Problem: Memory Leaks", "opengl_tutorial.html#autotoc_md413", null ]
        ] ],
        [ "Performance Tips", "opengl_tutorial.html#autotoc_md415", [
          [ "1. Minimize State Changes", "opengl_tutorial.html#autotoc_md416", null ],
          [ "2. Use Instancing for Repeated Objects", "opengl_tutorial.html#autotoc_md417", null ],
          [ "3. Batch Similar Objects", "opengl_tutorial.html#autotoc_md418", null ]
        ] ],
        [ "Debugging Tools", "opengl_tutorial.html#autotoc_md420", [
          [ "1. glGetError()", "opengl_tutorial.html#autotoc_md421", null ],
          [ "2. Shader Info Log", "opengl_tutorial.html#autotoc_md422", null ],
          [ "3. RenderDoc", "opengl_tutorial.html#autotoc_md423", null ]
        ] ],
        [ "Next Steps", "opengl_tutorial.html#autotoc_md425", null ],
        [ "Resources", "opengl_tutorial.html#autotoc_md426", null ]
      ] ]
    ] ],
    [ "pixel_truth", "md_docs_pixel_truth.html", null ],
    [ "The Pixel Truth: Everything is Just Computing RGBA", "pixel_truth.html", null ],
    [ "React/Redux to Vanilla HTML/CSS/JS Cleanup", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html", [
      [ "Files Updated", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md430", [
        [ "1. <tt>/docs/web_analogy.md</tt>", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md431", null ],
        [ "2. <tt>/src/gui/widgets/card.h</tt>", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md432", null ],
        [ "3. <tt>/docs/groups.dox</tt>", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md433", null ]
      ] ],
      [ "Philosophy Change", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md435", [
        [ "Before", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md436", null ],
        [ "After", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md437", null ]
      ] ],
      [ "Why This Change", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md439", null ],
      [ "Vanilla JavaScript Pattern Used", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md441", [
        [ "Component Pattern", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md442", null ],
        [ "State Management Pattern", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md443", null ],
        [ "Component Registry Pattern", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md444", null ]
      ] ],
      [ "Comparison Table Update", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md446", [
        [ "Before", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md447", null ],
        [ "After", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md448", null ]
      ] ],
      [ "Verification", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md450", null ],
      [ "Next Steps", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md452", null ],
      [ "Notes", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md454", null ],
      [ "Summary of Changes", "md_docs_REACT_TO_VANILLA_CLEANUP_2025_10_25.html#autotoc_md455", null ]
    ] ],
    [ "Documentation Guide", "md_docs_README.html", [
      [ "Generating Documentation", "md_docs_README.html#autotoc_md457", [
        [ "Prerequisites", "md_docs_README.html#autotoc_md458", null ],
        [ "Build Documentation", "md_docs_README.html#autotoc_md459", null ],
        [ "View Documentation", "md_docs_README.html#autotoc_md460", null ]
      ] ],
      [ "Documentation Structure", "md_docs_README.html#autotoc_md461", null ],
      [ "Documentation Features", "md_docs_README.html#autotoc_md462", [
        [ "1. Interactive Class Diagrams", "md_docs_README.html#autotoc_md463", null ],
        [ "2. Call Graphs", "md_docs_README.html#autotoc_md464", null ],
        [ "3. Source Code Browser", "md_docs_README.html#autotoc_md465", null ],
        [ "4. Visual SVG Diagrams", "md_docs_README.html#autotoc_md466", null ],
        [ "5. Search Functionality", "md_docs_README.html#autotoc_md467", null ]
      ] ],
      [ "Key Pages to Start With", "md_docs_README.html#autotoc_md468", null ],
      [ "Diagram Types", "md_docs_README.html#autotoc_md469", [
        [ "Class Diagrams", "md_docs_README.html#autotoc_md470", null ],
        [ "Collaboration Diagrams", "md_docs_README.html#autotoc_md471", null ],
        [ "Call Graphs", "md_docs_README.html#autotoc_md472", null ]
      ] ],
      [ "Tips for Navigating", "md_docs_README.html#autotoc_md473", [
        [ "For Beginners", "md_docs_README.html#autotoc_md474", null ],
        [ "For Understanding Data Flow", "md_docs_README.html#autotoc_md475", null ],
        [ "For Understanding Rendering", "md_docs_README.html#autotoc_md476", null ],
        [ "For Adding Features", "md_docs_README.html#autotoc_md477", null ]
      ] ],
      [ "Customizing Documentation", "md_docs_README.html#autotoc_md478", null ],
      [ "Updating Documentation", "md_docs_README.html#autotoc_md479", null ],
      [ "Troubleshooting", "md_docs_README.html#autotoc_md480", [
        [ "\"Doxygen not found\"", "md_docs_README.html#autotoc_md481", null ],
        [ "\"Graphs not generating\"", "md_docs_README.html#autotoc_md482", null ],
        [ "\"Call graphs missing\"", "md_docs_README.html#autotoc_md483", null ],
        [ "\"Class diagrams missing\"", "md_docs_README.html#autotoc_md484", null ],
        [ "\"Documentation incomplete\"", "md_docs_README.html#autotoc_md485", null ]
      ] ],
      [ "Resources", "md_docs_README.html#autotoc_md486", null ]
    ] ],
    [ "Web Development Analogy", "web_analogy.html", [
      [ "Overview", "web_analogy.html#autotoc_md488", null ],
      [ "The Three Pillars: HTML ↔ CSS ↔ JavaScript", "web_analogy.html#autotoc_md490", null ],
      [ "1. HTML = Structure (ImGui Widgets)", "web_analogy.html#autotoc_md492", [
        [ "Web Markup", "web_analogy.html#autotoc_md493", null ],
        [ "AeroDynControlRig Equivalent", "web_analogy.html#autotoc_md494", null ],
        [ "Key Equivalences", "web_analogy.html#autotoc_md495", null ],
        [ "Custom Components (Like Reusable Web Components)", "web_analogy.html#autotoc_md496", null ]
      ] ],
      [ "2. CSS = Styling (Theme System)", "web_analogy.html#autotoc_md498", [
        [ "Web Stylesheet", "web_analogy.html#autotoc_md499", null ],
        [ "AeroDynControlRig Equivalent", "web_analogy.html#autotoc_md500", null ],
        [ "CSS-Like Utilities", "web_analogy.html#autotoc_md501", null ],
        [ "Typography System", "web_analogy.html#autotoc_md502", null ]
      ] ],
      [ "3. JavaScript = Interactivity (Immediate Mode)", "web_analogy.html#autotoc_md504", [
        [ "Web Event Handling", "web_analogy.html#autotoc_md505", null ],
        [ "AeroDynControlRig Equivalent (Immediate Mode)", "web_analogy.html#autotoc_md506", null ],
        [ "Data Binding (Like Dynamic Web UIs)", "web_analogy.html#autotoc_md507", null ]
      ] ],
      [ "4. Component Architecture (Panel Pattern)", "web_analogy.html#autotoc_md509", [
        [ "JavaScript Component Pattern", "web_analogy.html#autotoc_md510", null ],
        [ "AeroDynControlRig Panel", "web_analogy.html#autotoc_md511", null ],
        [ "Panel Registration (Like Component Registry)", "web_analogy.html#autotoc_md512", null ]
      ] ],
      [ "5. Layout System (Flexbox/Grid-Like)", "web_analogy.html#autotoc_md514", [
        [ "Web Flexbox", "web_analogy.html#autotoc_md515", null ],
        [ "ImGui Equivalent", "web_analogy.html#autotoc_md516", null ],
        [ "Dockspace = CSS Grid", "web_analogy.html#autotoc_md517", null ]
      ] ],
      [ "6. State Management (Global State Pattern)", "web_analogy.html#autotoc_md519", [
        [ "JavaScript Global State", "web_analogy.html#autotoc_md520", null ],
        [ "SimulationState (Global State)", "web_analogy.html#autotoc_md521", null ]
      ] ],
      [ "7. Comparison Table", "web_analogy.html#autotoc_md523", null ],
      [ "8. Code-to-Code Translation Examples", "web_analogy.html#autotoc_md525", [
        [ "Example 1: Button with Conditional Text", "web_analogy.html#autotoc_md526", null ],
        [ "Example 2: Styled Container with Header", "web_analogy.html#autotoc_md527", null ],
        [ "Example 3: Color Badge Based on Condition", "web_analogy.html#autotoc_md528", null ]
      ] ],
      [ "9. Best Practices (Web → ImGui)", "web_analogy.html#autotoc_md530", [
        [ "✅ DO: Think in Components", "web_analogy.html#autotoc_md531", null ],
        [ "✅ DO: Use Design Tokens", "web_analogy.html#autotoc_md532", null ],
        [ "✅ DO: Separate Logic from Presentation", "web_analogy.html#autotoc_md533", null ],
        [ "❌ DON'T: Mix Styles Inconsistently", "web_analogy.html#autotoc_md534", null ]
      ] ],
      [ "10. Summary", "web_analogy.html#autotoc_md536", null ],
      [ "Related Documentation", "web_analogy.html#autotoc_md538", null ]
    ] ],
    [ "AeroDynControlRig", "md_README.html", [
      [ "Overview", "md_README.html#autotoc_md540", null ],
      [ "Directory Layout", "md_README.html#autotoc_md541", null ],
      [ "Getting Started", "md_README.html#autotoc_md542", null ],
      [ "Current Features", "md_README.html#autotoc_md543", null ],
      [ "Roadmap", "md_README.html#autotoc_md544", null ],
      [ "Long-Term Goals", "md_README.html#autotoc_md545", null ],
      [ "Ecosystem Map", "md_README.html#autotoc_md546", null ],
      [ "UI Preview", "md_README.html#autotoc_md547", [
        [ "Latest UI (October 2025)", "md_README.html#autotoc_md548", null ],
        [ "Previous Iterations", "md_README.html#autotoc_md549", null ]
      ] ]
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
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
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
"classTransform.html#aa08ca4266efabc768973cdeea51945ab",
"md_README.html#autotoc_md541",
"panel__manager_8h_source.html",
"style_8cpp.html#gad4204ae9873b183dca2479b338348718"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';