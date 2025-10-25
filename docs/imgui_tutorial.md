/**
 * @page imgui_tutorial ImGui Deep Dive Tutorial
 * @brief Complete guide to Dear ImGui with examples from our application
 *
 * @tableofcontents
 */

# ImGui Deep Dive Tutorial

## What is ImGui?

**Dear ImGui** (Immediate Mode GUI) is a bloat-free graphical user interface library for C++. It's perfect for:
- Debug tools
- Game editors
- Visualization dashboards (like ours!)
- Prototyping UIs quickly

### Why ImGui for Our Application?

- **Fast Development**: Create complex UIs in minutes
- **No XML/Designers**: Pure C++ code
- **Docking Support**: Arrange panels like an IDE
- **OpenGL Integration**: Works perfectly with our 3D renderer

---

## The Big Idea: Immediate Mode

### Traditional GUI (Retained Mode)

```cpp
// Create widgets once, they persist
Button* pauseButton = new Button("Pause");
pauseButton->setPosition(10, 10);
pauseButton->setCallback([]() {
    paused = !paused;
});

// Main loop
while (running) {
    // Widgets handle their own state
    if (pauseButton->isClicked()) {
        // Handle click
    }
    pauseButton->draw();
}
```

**Problems**:
- Widgets store state
- Synchronization issues
- Memory management complexity

### ImGui (Immediate Mode)

```cpp
// Re-declare UI every frame!
while (running) {
    ImGui::NewFrame();

    if (ImGui::Button("Pause")) {
        // Button was just clicked
        paused = !paused;
    }

    ImGui::Render();
}
```

**Benefits**:
- No widget state to manage
- Code is declarative and simple
- Easy to add/remove UI elements
- Less error-prone

**Key Insight**: You describe what the UI should look like RIGHT NOW. ImGui figures out the rest.

---

## Setup and Initialization

### From `application.cpp init()`

```cpp
// Step 1: Create ImGui context
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO();

// Step 2: Enable docking (allows panel arrangement)
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

// Step 3: Set visual style
ImGui::StyleColorsDark();

// Step 4: Initialize platform backends
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init("#version 330");
```

### What Each Backend Does

| Backend | Purpose |
|---------|---------|
| `ImGui_ImplGlfw` | Handles input (mouse, keyboard, scroll) from GLFW |
| `ImGui_ImplOpenGL3` | Renders ImGui using OpenGL draw calls |

---

## Frame Structure

Every frame follows this pattern:

```cpp
void Application::tick() {
    // 1. Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. Define UI (your code goes here)
    ImGui::Begin("My Window");
    ImGui::Text("Hello, World!");
    ImGui::End();

    // 3. Finalize ImGui data
    ImGui::Render();

    // 4. Render ImGui draw data with OpenGL
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 5. Swap buffers
    glfwSwapBuffers(window);
}
```

---

## Core Widgets

### Buttons

```cpp
// From control_panel.cpp

if (ImGui::Button("Pause")) {
    state.control.paused = !state.control.paused;
}

// Button with size
if (ImGui::Button("Reset", ImVec2(100, 30))) {
    // Reset logic
}

// Disabled button
ImGui::BeginDisabled(true);
ImGui::Button("Can't Click Me");
ImGui::EndDisabled();
```

**Return value**: `true` if clicked THIS frame.

### Text

```cpp
// Simple text
ImGui::Text("Simulation Time: %.2f s", time);

// Colored text
ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error!");

// Wrapped text (for long strings)
ImGui::TextWrapped("This text will wrap if it's too long...");

// Formatted text
ImGui::Text("Quaternion: [%.3f, %.3f, %.3f, %.3f]",
            quat[0], quat[1], quat[2], quat[3]);
```

### Sliders

```cpp
// From control_panel.cpp

// Float slider
ImGui::SliderFloat("Speed", &state.control.time_scale, 0.1f, 5.0f);

// Int slider
int frameRate = 60;
ImGui::SliderInt("FPS", &frameRate, 30, 144);

// Slider with format
ImGui::SliderFloat("Gain", &gain, 0.0f, 10.0f, "%.2f");

// Angle slider (displays in degrees)
ImGui::SliderAngle("Yaw", &yaw);
```

**Return value**: `true` if value changed.

### Input Fields

```cpp
// Float input
float value = 1.0f;
ImGui::InputFloat("Value", &value);

// Text input
char buffer[256] = "Enter text";
ImGui::InputText("Name", buffer, IM_ARRAYSIZE(buffer));

// Multi-line text
char text[1024] = "";
ImGui::InputTextMultiline("Notes", text, IM_ARRAYSIZE(text));

// With hints
ImGui::InputTextWithHint("##search", "Search...", buffer, 256);
```

### Checkboxes

```cpp
// From control_panel.cpp

bool paused = state.control.paused;
if (ImGui::Checkbox("Paused", &paused)) {
    state.control.paused = paused;
}

// Alternative: toggle button
bool& flag = state.control.use_fixed_dt;
ImGui::Checkbox("Fixed Timestep", &flag);
```

### Combo Boxes (Dropdowns)

```cpp
const char* items[] = { "Option 1", "Option 2", "Option 3" };
static int currentItem = 0;

if (ImGui::BeginCombo("Select", items[currentItem])) {
    for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
        bool isSelected = (currentItem == i);
        if (ImGui::Selectable(items[i], isSelected)) {
            currentItem = i;
        }
        if (isSelected) {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();
}
```

### Radio Buttons

```cpp
static int selected = 0;

ImGui::RadioButton("Mode 1", &selected, 0); ImGui::SameLine();
ImGui::RadioButton("Mode 2", &selected, 1); ImGui::SameLine();
ImGui::RadioButton("Mode 3", &selected, 2);
```

---

## Layout and Spacing

### Same Line

```cpp
ImGui::Text("Label:");
ImGui::SameLine();
ImGui::InputFloat("##value", &value);  // ## hides label
```

### Spacing

```cpp
ImGui::Spacing();        // Add vertical space
ImGui::Separator();      // Horizontal line
ImGui::Dummy(ImVec2(0, 20));  // Custom spacing
```

### Columns

```cpp
ImGui::Columns(2, "mycolumns");

// Column 1
ImGui::Text("Left");
ImGui::NextColumn();

// Column 2
ImGui::Text("Right");
ImGui::NextColumn();

ImGui::Columns(1);  // Back to single column
```

### Groups

```cpp
ImGui::BeginGroup();
ImGui::Text("Grouped");
ImGui::Button("Button 1");
ImGui::Button("Button 2");
ImGui::EndGroup();

// Can query group size
ImVec2 size = ImGui::GetItemRectSize();
```

---

## Windows and Panels

### Basic Window

```cpp
// From telemetry_panel.cpp

void TelemetryPanel::draw(SimulationState& state, Camera& camera) {
    ImGui::Begin("Flight Telemetry");

    ImGui::Text("Quaternion: [%.3f, %.3f, %.3f, %.3f]",
                state.quaternion[0],
                state.quaternion[1],
                state.quaternion[2],
                state.quaternion[3]);

    ImGui::End();
}
```

### Window Flags

```cpp
ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse;

ImGui::Begin("Fixed Window", nullptr, flags);
// Content...
ImGui::End();
```

Common flags:
- `ImGuiWindowFlags_NoTitleBar` - No title bar
- `ImGuiWindowFlags_NoResize` - Can't resize
- `ImGuiWindowFlags_NoMove` - Can't move
- `ImGuiWindowFlags_NoScrollbar` - No scroll bars
- `ImGuiWindowFlags_MenuBar` - Has menu bar
- `ImGuiWindowFlags_HorizontalScrollbar` - Horizontal scroll

### Child Windows (Scrollable Regions)

```cpp
ImGui::BeginChild("ScrollRegion", ImVec2(0, 300), true);

// Lots of content...
for (int i = 0; i < 100; i++) {
    ImGui::Text("Line %d", i);
}

ImGui::EndChild();
```

---

## Advanced Widgets

### Plotting

```cpp
// From dynamics_panel.cpp

float values[100];
// Fill with data...

ImGui::PlotLines("Output", values, IM_ARRAYSIZE(values),
                 0, nullptr, 0.0f, 1.0f, ImVec2(400, 100));

// Histogram
ImGui::PlotHistogram("Distribution", values, 100);
```

### Progress Bars

```cpp
float progress = 0.6f;  // 60%
ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));

// With overlay text
ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f), "60%");
```

### Color Pickers

```cpp
ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red

ImGui::ColorEdit3("Color", (float*)&color);  // RGB
ImGui::ColorEdit4("Color", (float*)&color);  // RGBA

// Color picker
ImGui::ColorPicker3("Pick Color", (float*)&color);
```

### Images

```cpp
// Display OpenGL texture
ImGui::Image((void*)(intptr_t)textureID, ImVec2(800, 600));

// With UV coordinates (flip Y)
ImGui::Image((void*)(intptr_t)textureID,
             ImVec2(800, 600),
             ImVec2(0, 1),   // uv0
             ImVec2(1, 0));  // uv1 (flipped)

// Clickable image
if (ImGui::ImageButton((void*)(intptr_t)textureID, ImVec2(64, 64))) {
    // Clicked!
}
```

### Trees

```cpp
if (ImGui::TreeNode("Settings")) {
    ImGui::Text("Nested content");
    ImGui::SliderFloat("Value", &value, 0, 1);
    ImGui::TreePop();
}

// With flags
if (ImGui::TreeNodeEx("Advanced", ImGuiTreeNodeFlags_DefaultOpen)) {
    // Content...
    ImGui::TreePop();
}
```

### Tabs

```cpp
if (ImGui::BeginTabBar("MyTabBar")) {
    if (ImGui::BeginTabItem("Tab 1")) {
        ImGui::Text("Content of tab 1");
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Tab 2")) {
        ImGui::Text("Content of tab 2");
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
```

---

## Styling and Theming

### Colors

```cpp
// From style.cpp

ImGuiStyle& style = ImGui::GetStyle();

// Set individual colors
style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.5f, 0.9f, 1.0f);
style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.4f, 0.8f, 1.0f);

// Or push/pop temporarily
ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
ImGui::Text("Red text");
ImGui::PopStyleColor();
```

### Sizes and Spacing

```cpp
ImGuiStyle& style = ImGui::GetStyle();

style.WindowPadding = ImVec2(12, 12);
style.FramePadding = ImVec2(8, 4);
style.ItemSpacing = ImVec2(8, 8);
style.WindowRounding = 6.0f;
style.FrameRounding = 4.0f;

// Temporary changes
ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 10));
ImGui::Button("Big Padding");
ImGui::PopStyleVar();
```

### Fonts

```cpp
// From style.cpp LoadFonts()

ImGuiIO& io = ImGui::GetIO();

// Load custom font
ImFont* customFont = io.Fonts->AddFontFromFileTTF(
    "assets/fonts/Roboto-Regular.ttf",
    18.0f  // Size
);

// Use it
ImGui::PushFont(customFont);
ImGui::Text("Custom font!");
ImGui::PopFont();
```

---

## Docking

### Enable Docking

```cpp
// From application.cpp init()

ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
```

### Dockspace Example

```cpp
// From application.cpp renderDashboardLayout()

ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

// Now all windows can be docked!
ImGui::Begin("Panel 1");
// ...
ImGui::End();

ImGui::Begin("Panel 2");
// ...
ImGui::End();

// User can drag panels to dock them
```

---

## Input Handling

### Mouse

```cpp
// Get mouse position
ImVec2 mousePos = ImGui::GetMousePos();

// Check if mouse is over item
if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Hover text");
}

// Mouse clicks
if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    // Left click
}

if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
    ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
    // Handle drag
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
}

// Scroll wheel
float wheel = ImGui::GetIO().MouseWheel;
```

### Keyboard

```cpp
ImGuiIO& io = ImGui::GetIO();

if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
    // Ctrl+S pressed
    save();
}

// Check specific keys
if (ImGui::IsKeyDown(ImGuiKey_Space)) {
    // Space held down
}
```

### Capturing Input

```cpp
ImGuiIO& io = ImGui::GetIO();

// Check if ImGui wants mouse input
if (io.WantCaptureMouse) {
    // Don't process mouse in your 3D scene
    return;
}

// Check if ImGui wants keyboard input
if (io.WantCaptureKeyboard) {
    // Don't process keyboard in your game
    return;
}
```

---

## Custom Rendering

### Draw Lists

```cpp
ImDrawList* draw_list = ImGui::GetWindowDrawList();

// Get window position
ImVec2 pos = ImGui::GetCursorScreenPos();

// Draw line
draw_list->AddLine(
    ImVec2(pos.x, pos.y),
    ImVec2(pos.x + 100, pos.y + 100),
    IM_COL32(255, 0, 0, 255),  // Red
    2.0f                       // Thickness
);

// Draw rectangle
draw_list->AddRect(
    ImVec2(pos.x, pos.y),
    ImVec2(pos.x + 200, pos.y + 100),
    IM_COL32(0, 255, 0, 255)   // Green
);

// Filled rectangle
draw_list->AddRectFilled(
    ImVec2(pos.x, pos.y),
    ImVec2(pos.x + 200, pos.y + 100),
    IM_COL32(0, 0, 255, 128)   // Blue, semi-transparent
);

// Circle
draw_list->AddCircle(
    ImVec2(pos.x + 100, pos.y + 100),
    50.0f,                     // Radius
    IM_COL32(255, 255, 0, 255) // Yellow
);

// Text
draw_list->AddText(
    ImVec2(pos.x, pos.y),
    IM_COL32(255, 255, 255, 255),
    "Custom text"
);
```

---

## Panel Manager Pattern

Our application uses a **Panel Manager** to organize UI:

### Base Panel Interface

```cpp
// From panel.h

class Panel {
public:
    virtual ~Panel() = default;
    virtual const char* name() const = 0;
    virtual void draw(SimulationState& state, Camera& camera) = 0;
};
```

### Implementing a Panel

```cpp
// From telemetry_panel.h

class TelemetryPanel : public Panel {
public:
    const char* name() const override {
        return "Flight Telemetry";
    }

    void draw(SimulationState& state, Camera& camera) override {
        ImGui::Begin(name());

        ImGui::Text("Quaternion: [%.3f, %.3f, %.3f, %.3f]",
                    state.quaternion[0],
                    state.quaternion[1],
                    state.quaternion[2],
                    state.quaternion[3]);

        ImGui::End();
    }
};
```

### Panel Manager

```cpp
// From panel_manager.cpp

void PanelManager::registerPanel(std::unique_ptr<Panel> panel) {
    panels_.push_back(std::move(panel));
}

void PanelManager::drawAll(SimulationState& state, Camera& camera) {
    for (auto& panel : panels_) {
        panel->draw(state, camera);
    }
}
```

### Usage

```cpp
// From application.cpp

void Application::initializePanels() {
    panelManager.registerPanel(std::make_unique<ControlPanel>());
    panelManager.registerPanel(std::make_unique<TelemetryPanel>());
    panelManager.registerPanel(std::make_unique<DynamicsPanel>());
    // ... more panels
}

void Application::tick() {
    // ...
    ImGui::NewFrame();
    panelManager.drawAll(simulationState, camera);
    ImGui::Render();
    // ...
}
```

---

## Common Patterns

### Read-Only Display

```cpp
ImGui::BeginDisabled(true);
ImGui::InputFloat("RPM", &rpm);  // Grayed out, can't edit
ImGui::EndDisabled();

// Or just use Text
ImGui::Text("RPM: %.0f", rpm);
```

### Conditional UI

```cpp
if (state.control.use_fixed_dt) {
    ImGui::SliderFloat("Fixed dt", &state.control.fixed_dt, 0.001f, 0.1f);
}

// Toggle visibility
static bool showAdvanced = false;
ImGui::Checkbox("Show Advanced", &showAdvanced);
if (showAdvanced) {
    // Advanced controls...
}
```

### Tooltips

```cpp
ImGui::Button("Help");
if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Click for help documentation");
}

// Or with delay
if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
    ImGui::SetTooltip("Delayed tooltip");
}
```

---

## Performance Tips

### 1. Minimize State Changes

```cpp
// BAD: Creates new ID every frame
for (int i = 0; i < items.size(); i++) {
    ImGui::PushID(std::to_string(i).c_str());  // Slow!
    // ...
    ImGui::PopID();
}

// GOOD: Use int ID
for (int i = 0; i < items.size(); i++) {
    ImGui::PushID(i);
    // ...
    ImGui::PopID();
}
```

### 2. Collapse Unused Panels

Users can collapse panels. ImGui skips most processing for collapsed windows.

### 3. Limit PlotLines Data

```cpp
// Don't plot 10,000 points
// Downsample or use a sliding window
constexpr size_t MAX_SAMPLES = 512;
std::deque<float> history;  // Auto-limits size
```

---

## Debugging ImGui

### Demo Window

```cpp
// Show ImGui demo (all widgets)
ImGui::ShowDemoWindow();
```

**Use this to**:
- Learn all widget types
- See code examples
- Test styling

### Metrics Window

```cpp
ImGui::ShowMetricsWindow();
```

**Shows**:
- Frame rate
- Memory usage
- Active windows
- Draw calls

### Debug Log

```cpp
ImGui::LogToTTY();  // Log to console
ImGui::Button("Test");
ImGui::LogFinish();
```

---

## Next Steps

1. **Browse `gui/panels/`**: See real examples
2. **Modify a panel**: Add a new widget
3. **Create a custom panel**: Implement Panel interface
4. **Experiment**: Try different layouts and styles

## Resources

- [ImGui GitHub](https://github.com/ocornut/imgui) - Source and examples
- [ImGui Demo Code](https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp) - All widgets showcased
- [ImGui Manual](https://github.com/ocornut/imgui/wiki) - Official wiki

---

**You now understand ImGui and can build custom panels!**

@see @ref opengl_tutorial for OpenGL guide
@see @ref opengl_imgui_integration for how they work together
