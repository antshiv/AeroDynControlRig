# Web Development Analogy {#web_analogy}

[TOC]

## Overview

This document explains how AeroDynControlRig's ImGui UI system maps directly to modern web development concepts. If you understand HTML, CSS, and JavaScript, you already understand the architecture of this codebase.

---

## The Three Pillars: HTML ↔ CSS ↔ JavaScript

| Web Technology | AeroDynControlRig Equivalent | Files | Purpose |
|----------------|------------------------------|-------|---------|
| **HTML** | ImGui widget calls + custom widgets | `gui/widgets/*.h`, panel `draw()` methods | Structure and hierarchy |
| **CSS** | Style system (colors, spacing, rounding) | `gui/style.h`, `gui/style.cpp` | Visual appearance |
| **JavaScript** | ImGui immediate mode + state updates | Panel logic, button callbacks | Interactivity and behavior |

---

## 1. HTML = Structure (ImGui Widgets)

### Web Markup
```html
<div class="card">
  <div class="card-header">
    <h2>Rotor Dynamics</h2>
    <span class="badge badge-success">Real-time +2.5%</span>
  </div>
  <p class="text-muted">Rotor RPM</p>
  <h1 class="heading">2140 RPM</h1>
  <div class="chip-row">
    <span class="chip">Total Thrust: 45.2 N</span>
    <span class="chip">Total Power: 320 W</span>
  </div>
</div>
```

### AeroDynControlRig Equivalent

File: `src/gui/panels/rotor_panel.cpp`

```cpp
ui::BeginCard(name(), options);                                    // <div class="card">
    ui::CardHeader("Rotor Dynamics", "Real-time +2.5%", &color);   //   <div class="card-header"> + badge

    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);      //   <p class="text-muted">
    ImGui::TextUnformatted("Rotor RPM");                           //     Rotor RPM
    ImGui::PopStyleColor();                                        //   </p>

    if (fonts.heading) ImGui::PushFont(fonts.heading);             //   <h1 class="heading">
    ImGui::Text("%.0f RPM", avg_rpm);                              //     2140 RPM
    if (fonts.heading) ImGui::PopFont();                           //   </h1>

    // <div class="chip-row">
    ui::ValueChip("Total Thrust", "45.2 N", config);               //   <span class="chip">
    ImGui::SameLine();                                             //   (inline layout)
    ui::ValueChip("Total Power", "320 W", config);                 //   <span class="chip">
    // </div>
ui::EndCard();                                                      // </div>
```

### Key Equivalences

| HTML Element | ImGui Widget | Example |
|--------------|--------------|---------|
| `<div>` | `ImGui::BeginChild()` / `ui::BeginCard()` | Container grouping |
| `<h1>` | `ImGui::Text()` with heading font | Large title text |
| `<p>` | `ImGui::TextUnformatted()` | Regular paragraph |
| `<button>` | `ImGui::Button()` | Clickable button |
| `<input type="range">` | `ImGui::SliderFloat()` | Numeric slider |
| `<input type="checkbox">` | `ImGui::Checkbox()` | Toggle checkbox |
| `<select>` | `ImGui::Combo()` | Dropdown menu |
| `<span class="badge">` | `ui::CardHeader()` badge parameter | Status indicator |

### Custom Components (Like Reusable Web Components)

Just as web developers create reusable components with custom elements, we have:

```cpp
namespace ui {
    bool BeginCard(const char* id, const CardOptions& options);  // <Card>
    void EndCard();                                               // </Card>

    void CardHeader(const char* title,                            // <CardHeader>
                    const char* badge_label,                      //   with optional badge
                    const ImVec4* badge_color);

    ImVec2 ValueChip(const char* label,                           // <Chip>
                     const char* value,
                     const ChipConfig& config);
}
```

**Usage matches component configuration:**

```cpp
// Web component: <custom-card min-width="320" min-height="360" scrollable="false">
ui::CardOptions options;
options.min_size = ImVec2(320.0f, 360.0f);
options.allow_scrollbar = false;
ui::BeginCard("MyCard", options);
```

---

## 2. CSS = Styling (Theme System)

### Web Stylesheet
```css
/* Design tokens (CSS custom properties) */
:root {
  --canvas-bg: #0c1014;
  --card-bg: #111618;
  --card-border: #293640;
  --accent-base: #1193d4;
  --success: #39c26e;
  --danger: #f05446;
  --text-primary: #dceef8;
  --text-muted: #98aeb9;
}

/* Component styles */
.card {
  background-color: var(--card-bg);
  border: 1px solid var(--card-border);
  border-radius: 18px;
  padding: 24px 22px;
  min-width: 320px;
  min-height: 200px;
}

.badge-success {
  background-color: var(--success);
  border-radius: 12px;
  padding: 4px 12px;
}
```

### AeroDynControlRig Equivalent

File: `src/gui/style.h` and `src/gui/style.cpp`

```cpp
// Design tokens (Palette struct)
struct Palette {
    ImVec4 canvas_bg;      // #0c1014
    ImVec4 card_bg;        // #111618
    ImVec4 card_border;    // #293640
    ImVec4 accent_base;    // #1193d4
    ImVec4 success;        // #39c26e
    ImVec4 danger;         // #f05446
    ImVec4 text_primary;   // #dceef8
    ImVec4 text_muted;     // #98aeb9
};

const Palette& palette = ui::Colors();  // Global palette access

// Component styling (in card.cpp)
void BeginCard() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, palette.card_bg);       // background-color
    ImGui::PushStyleColor(ImGuiCol_Border, palette.card_border);     // border-color
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);        // border-radius
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);       // border-width
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 22)); // padding
    ImGui::SetNextWindowSizeConstraints(ImVec2(320, 200), ...);      // min-width, min-height
}

// Badge styling (in card.cpp CardHeader())
draw_list->AddRectFilled(badge_pos, badge_size,
                         ImColor(palette.success),      // background-color
                         12.0f);                        // border-radius: 12px
```

### CSS-Like Utilities

**Push/Pop Style Stack = Inline Styles**

```cpp
// Like: <p style="color: #98aeb9">Label</p>
ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
ImGui::TextUnformatted("Label");
ImGui::PopStyleColor();

// Like: <button style="border-radius: 24px; padding: 20px 10px">
ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.0f);
ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 10));
ImGui::Button("Click");
ImGui::PopStyleVar(2);
```

**Theme Application = Global Stylesheet**

File: `src/gui/style.cpp:71-125`

```cpp
void ApplyTheme(ImGuiStyle& style) {
    // Spacing (like CSS box model)
    style.WindowPadding = ImVec2(26.0f, 22.0f);        // padding
    style.ItemSpacing = ImVec2(14.0f, 12.0f);          // gap
    style.FramePadding = ImVec2(16.0f, 12.0f);         // input padding

    // Rounding (border-radius)
    style.WindowRounding = 18.0f;    // .window { border-radius: 18px }
    style.FrameRounding = 12.0f;     // .input { border-radius: 12px }

    // All color assignments (90+ lines mapping ImGui colors to palette)
    style.Colors[ImGuiCol_WindowBg] = palette.card_bg;
    style.Colors[ImGuiCol_Button] = palette.slate_base;
    style.Colors[ImGuiCol_ButtonHovered] = palette.slate_hover;
    // ... etc
}
```

### Typography System

**CSS:**
```css
@font-face {
  font-family: 'SpaceGrotesk';
  src: url('SpaceGrotesk-Regular.ttf');
}

body { font: 18px 'SpaceGrotesk'; }
h1 { font: 22px 'SpaceGrotesk SemiBold'; }
code { font: 17px 'SpaceGrotesk Medium'; }
```

**AeroDynControlRig:**
```cpp
struct FontSet {
    ImFont* body = nullptr;     // 18px SpaceGrotesk-Regular
    ImFont* heading = nullptr;  // 22px SpaceGrotesk-SemiBold
    ImFont* mono = nullptr;     // 17px SpaceGrotesk-Medium
    ImFont* icon = nullptr;     // 20px MaterialSymbolsRounded
};

// Usage:
const FontSet& fonts = ui::Fonts();
if (fonts.heading) ImGui::PushFont(fonts.heading);  // font-family: heading
ImGui::Text("Title");
if (fonts.heading) ImGui::PopFont();
```

---

## 3. JavaScript = Interactivity (Immediate Mode)

### Web Event Handling
```javascript
// Traditional retained mode (DOM)
const button = document.querySelector('#pause-button');
button.addEventListener('click', () => {
  state.paused = !state.paused;
  button.textContent = state.paused ? 'Resume' : 'Pause';
});

const slider = document.querySelector('#zoom-slider');
slider.addEventListener('input', (e) => {
  camera.zoom = parseFloat(e.target.value);
});
```

### AeroDynControlRig Equivalent (Immediate Mode)

File: `src/gui/panels/control_panel.cpp`

```cpp
// ImGui immediate mode - re-declare every frame!
if (ImGui::Button(state.control.paused ? "Resume" : "Pause")) {
    state.control.paused = !state.control.paused;  // Toggle on click
}

// Slider automatically updates the variable when dragged
if (ImGui::SliderFloat("Zoom", &camera.zoom, 0.5f, 5.0f)) {
    // Optional: do something when value changes
}
```

**Key Difference:**
- **Web (retained mode)**: Create widget once, attach listeners, manually update DOM when state changes
- **ImGui (immediate mode)**: Re-declare widget every frame with current state, ImGui handles everything

### Data Binding (Like Dynamic Web UIs)

**JavaScript (vanilla):**
```javascript
function renderControlPanel(state) {
  const button = document.getElementById('pause-btn');
  button.textContent = state.paused ? 'Resume' : 'Pause';
  button.onclick = () => {
    state.paused = !state.paused;
    renderControlPanel(state); // Re-render
  };

  const slider = document.getElementById('time-scale');
  slider.value = state.timeScale;
  slider.oninput = (e) => {
    state.timeScale = e.target.value;
  };
}
```

**AeroDynControlRig:**
```cpp
void ControlPanel::draw(SimulationState& state, Camera& camera) {
    if (ImGui::Button(state.control.paused ? "Resume" : "Pause")) {
        state.control.paused = !state.control.paused;
    }

    ImGui::SliderFloat("Time Scale", &state.control.time_scale, 0.1f, 5.0f);
}
```

ImGui is **declarative** - you describe what the UI should look like given the current state, and ImGui handles updates (similar to how you'd re-render the DOM in vanilla JavaScript).

---

## 4. Component Architecture (Panel Pattern)

### JavaScript Component Pattern
```javascript
class RotorPanel {
  constructor(containerId) {
    this.container = document.getElementById(containerId);
  }

  render(state) {
    const avgRpm = state.rotor.rpm.reduce((a, b) => a + b, 0) / state.rotor.rpm.length;

    this.container.innerHTML = `
      <div class="card">
        <div class="card-header">
          <h2>Rotor Dynamics</h2>
          <span class="badge">Real-time +2.5%</span>
        </div>
        <p class="text-muted">Rotor RPM</p>
        <h1>${avgRpm.toFixed(0)} RPM</h1>
        <div class="chip">Total Thrust: ${state.rotor.totalThrust} N</div>
      </div>
    `;
  }
}
```

### AeroDynControlRig Panel

File: `src/gui/panels/rotor_panel.h` and `rotor_panel.cpp`

```cpp
class RotorPanel : public Panel {
public:
    const char* name() const override { return "Rotor Dynamics"; }

    void draw(SimulationState& state, Camera& camera) override {
        // Calculate derived data (like JavaScript component logic)
        float rpm_sum = 0.0f;
        for (double rpm : state.rotor.rpm) {
            rpm_sum += static_cast<float>(rpm);
        }
        float avg_rpm = rpm_sum / state.rotor.rpm.size();

        // Render UI (declarative, like building HTML)
        ui::BeginCard(name(), options);
            ui::CardHeader("Rotor Dynamics", "Real-time +2.5%", &palette.success);

            ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
            ImGui::TextUnformatted("Rotor RPM");
            ImGui::PopStyleColor();

            if (fonts.heading) ImGui::PushFont(fonts.heading);
            ImGui::Text("%.0f RPM", avg_rpm);
            if (fonts.heading) ImGui::PopFont();

            char thrust_value[32];
            std::snprintf(thrust_value, sizeof(thrust_value), "%.1f N", state.rotor.total_thrust_newton);
            ui::ValueChip("Total Thrust", thrust_value, config);
        ui::EndCard();
    }
};
```

### Panel Registration (Like Component Registry)

**JavaScript (manual component registration):**
```javascript
const panelRegistry = [];
panelRegistry.push(new RotorPanel('rotor-container'));
panelRegistry.push(new TelemetryPanel('telemetry-container'));

// Render all panels
function renderAll(state) {
  panelRegistry.forEach(panel => panel.render(state));
}
```

**AeroDynControlRig:**
```cpp
// In Application::initializePanels()
panelManager.registerPanel(std::make_unique<RotorPanel>());
panelManager.registerPanel(std::make_unique<TelemetryPanel>());
panelManager.registerPanel(std::make_unique<DynamicsPanel>());
// ...

// In Application::render3D()
panelManager.drawAll(simulationState, camera);  // Renders all panels
```

---

## 5. Layout System (Flexbox/Grid-Like)

### Web Flexbox
```html
<div class="flex gap-4">
  <span class="chip">Thrust: 45.2 N</span>
  <span class="chip">Power: 320 W</span>
</div>
```

```css
.flex {
  display: flex;
  gap: 16px;
}
```

### ImGui Equivalent
```cpp
ui::ValueChip("Total Thrust", "45.2 N", config);
ImGui::SameLine(0.0f, 10.0f);  // Horizontal layout with 10px gap
ui::ValueChip("Total Power", "320 W", config);
```

### Dockspace = CSS Grid

**CSS Grid:**
```css
.dashboard {
  display: grid;
  grid-template-columns: 2fr 1fr;
  grid-template-rows: 1fr 1fr;
}
```

**ImGui Dockspace:**

File: `src/app/application.cpp:565-579`

```cpp
// Split screen into left (68%) and right (32%)
ImGui::DockBuilderSplitNode(dock_root, ImGuiDir_Right, 0.32f, &dock_right, &dock_left);

// Split left into top (58%) and bottom (42%)
ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Down, 0.42f, &dock_left_bottom, &dock_left);

// Assign panels to grid cells
ImGui::DockBuilderDockWindow("Flight Scene", dock_left);        // Top-left (large)
ImGui::DockBuilderDockWindow("Rotor Dynamics", dock_right);     // Top-right
ImGui::DockBuilderDockWindow("Control Panel", dock_bottom_left); // Bottom-left
// ...
```

This creates a complex grid layout similar to CSS Grid's `grid-template-areas`.

---

## 6. State Management (Global State Pattern)

### JavaScript Global State
```javascript
// Central state (like window.appState)
const appState = {
  attitude: { quaternion: [1, 0, 0, 0] },
  rotor: { rpm: [0, 0, 0, 0], totalThrust: 0 },
  control: { paused: false, timeScale: 1.0 }
};

// All components read from global state
function renderRotorPanel() {
  const rpm = appState.rotor.rpm;
  // Update DOM based on state
}
```

### SimulationState (Global State)

File: `src/core/simulation_state.h`

```cpp
// Central state (like window.appState)
struct SimulationState {
    // Attitude state
    struct {
        glm::dquat quaternion = glm::dquat(1.0, 0.0, 0.0, 0.0);
        glm::dvec3 euler_deg = glm::dvec3(0.0);
        glm::dvec3 angular_rate_deg_per_sec = glm::dvec3(0.0);
        glm::dmat4 model_matrix = glm::dmat4(1.0);
    } attitude;

    // Rotor state
    struct {
        std::vector<double> rpm;
        std::vector<double> thrust_newton;
        double total_thrust_newton = 0.0;
        double total_power_watt = 0.0;
    } rotor;

    // Control state
    struct {
        bool paused = false;
        double time_scale = 1.0;
        bool use_fixed_dt = false;
    } control;
    // ... more nested structs
};
```

**All modules and panels receive a reference to this shared state:**

```cpp
// Modules update state (like JavaScript updating appState)
void QuaternionDemoModule::update(double dt, SimulationState& state) {
    state.attitude.quaternion = /* integrate angular velocity */;
}

// Panels read state (like JavaScript reading appState)
void RotorPanel::draw(SimulationState& state, Camera& camera) {
    float avg_rpm = calculateAverage(state.rotor.rpm);
    ImGui::Text("%.0f RPM", avg_rpm);
}
```

---

## 7. Comparison Table

| Feature | Web | AeroDynControlRig |
|---------|-----|-------------------|
| **Structure** | HTML tags | ImGui widget calls |
| **Styling** | CSS properties | `PushStyleColor/Var` |
| **Layout** | Flexbox/Grid | `SameLine()` / Dockspace |
| **Components** | `<Card>`, `<Button>` | `ui::BeginCard()`, `ImGui::Button()` |
| **Design Tokens** | CSS custom properties | `Palette` struct |
| **Typography** | `@font-face`, `font-family` | `FontSet` struct |
| **State Management** | `window.appState` | `SimulationState` |
| **Data Binding** | DOM manipulation | ImGui immediate mode |
| **Component Props** | `<Card title="..." />` | `ui::BeginCard(id, options)` |
| **Theming** | CSS theme files | `ApplyTheme(style)` |
| **Interactivity** | Event listeners | `if (ImGui::Button(...))` |

---

## 8. Code-to-Code Translation Examples

### Example 1: Button with Conditional Text

**Web (vanilla JavaScript):**
```javascript
const button = document.getElementById('pause-btn');
button.textContent = state.paused ? 'Resume' : 'Pause';
button.onclick = () => {
  state.paused = !state.paused;
  button.textContent = state.paused ? 'Resume' : 'Pause';
};
```

**AeroDynControlRig:**
```cpp
if (ImGui::Button(state.control.paused ? "Resume" : "Pause")) {
    state.control.paused = !state.control.paused;
}
```

### Example 2: Styled Container with Header

**Web (HTML + CSS):**
```html
<div class="card">
  <h2>Telemetry</h2>
  <p>Content here...</p>
</div>
```
```css
.card {
  background: #111618;
  border-radius: 18px;
  padding: 24px 22px;
}
```

**AeroDynControlRig:**
```cpp
ui::BeginCard("Telemetry", options);  // options contain min_size, etc.
    ui::CardHeader("Telemetry");
    ImGui::TextUnformatted("Content here...");
ui::EndCard();
```

### Example 3: Color Badge Based on Condition

**Web (JavaScript):**
```javascript
const badge = document.createElement('span');
badge.className = delta >= 0 ? 'badge-success' : 'badge-danger';
badge.textContent = `Real-time ${delta >= 0 ? '+' : ''}${delta.toFixed(1)}%`;
```

**AeroDynControlRig:**
```cpp
char badge_label[32];
std::snprintf(badge_label, sizeof(badge_label), "Real-time %+0.1f%%", delta_percent);
const ImVec4& badge_color = delta_percent >= 0.0f ? palette.success : palette.danger;
ui::CardHeader("Rotor Dynamics", badge_label, &badge_color);
```

---

## 9. Best Practices (Web → ImGui)

### ✅ DO: Think in Components

**Web:** Break UI into reusable component functions
**ImGui:** Create reusable widget functions in `gui/widgets/`

```cpp
// Good: Reusable widget
namespace ui {
    ImVec2 ValueChip(const char* label, const char* value, const ChipConfig& config);
}

// Usage in multiple panels:
ui::ValueChip("Thrust", "45.2 N", config);
ui::ValueChip("Power", "320 W", config);
```

### ✅ DO: Use Design Tokens

**Web:** Define colors in CSS custom properties
**ImGui:** Use the `Palette` struct

```cpp
// Good: Use palette colors
ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);

// Bad: Hardcoded colors
ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
```

### ✅ DO: Separate Logic from Presentation

**Web:** Keep business logic separate from DOM manipulation
**ImGui:** Calculate data before rendering widgets

```cpp
void RotorPanel::draw(SimulationState& state, Camera& camera) {
    // 1. Calculate derived data (business logic)
    float avg_rpm = calculateAverage(state.rotor.rpm);
    float delta = calculateDelta(avg_rpm, previous_avg);

    // 2. Render UI (presentation)
    ui::BeginCard(name(), options);
        ImGui::Text("%.0f RPM", avg_rpm);
        ui::ValueChip("Delta", formatDelta(delta), config);
    ui::EndCard();
}
```

### ❌ DON'T: Mix Styles Inconsistently

**Bad:**
```cpp
// Mixing palette colors with hardcoded values
ImGui::PushStyleColor(ImGuiCol_WindowBg, palette.card_bg);  // Good
ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2, 0.3, 0.4, 1.0));  // Bad!
```

**Good:**
```cpp
// Use palette everywhere
ImGui::PushStyleColor(ImGuiCol_WindowBg, palette.card_bg);
ImGui::PushStyleColor(ImGuiCol_Border, palette.card_border);
```

---

## 10. Summary

If you know web development, you already understand this codebase:

| Concept | You Already Know This From Web |
|---------|--------------------------------|
| **ImGui widgets** | HTML elements (`<div>`, `<button>`, `<input>`) |
| **Style push/pop** | Inline CSS (`style="..."`) |
| **Palette** | CSS custom properties (`--color-primary`) |
| **Panels** | JavaScript component classes |
| **SimulationState** | Global `window.appState` object |
| **Immediate mode** | Declarative UI (re-rendering the DOM) |
| **Dockspace** | CSS Grid layout |

The only difference is **syntax**:
- Instead of `<Card>`, write `BeginCard()`
- Instead of `.card { ... }`, write `PushStyleColor/Var()`
- Instead of `onClick={...}`, write `if (ImGui::Button(...)) { ... }`

**The architecture is the same. The thinking is the same.**

---

## Related Documentation

- @ref ui_architecture - UI architecture details
- @ref gpu_pipeline - How this all gets rendered to the screen
- @ref Panel - Base class for all UI panels
- `src/gui/style.h` - Complete palette and theming system
- `src/gui/widgets/` - Custom widget implementations
