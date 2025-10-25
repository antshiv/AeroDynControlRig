# React/Redux to Vanilla HTML/CSS/JS Cleanup

**Date:** 2025-10-25
**Reason:** Remove React/Redux/JSX references and replace with pure HTML/CSS/JavaScript examples

---

## Files Updated

### 1. `/docs/web_analogy.md`
**Changes:**
- Replaced all React component examples with vanilla JavaScript component pattern
- Changed `React.Component` to JavaScript class with `render()` method
- Removed JSX syntax (`<Card />`) and replaced with `innerHTML` or DOM manipulation
- Replaced Redux store examples with `window.appState` global state pattern
- Changed `React Router` examples to manual component registry pattern
- Updated all code comments mentioning React/Vue/Redux

**Key Replacements:**
- `React.Component` → JavaScript class pattern
- `Redux store` → `window.appState`
- `JSX <Card />` → `<div class="card">` with `innerHTML`
- `React props` → HTML data attributes or config objects
- `useState/useEffect` → Manual DOM updates
- `Redux actions` → Direct state mutation

### 2. `/src/gui/widgets/card.h`
**Changes:**
- Changed "React/Vue component libraries" to "CSS component libraries"
- Replaced JSX code example with HTML data attributes
- Updated component prop analogy from React to vanilla web components

### 3. `/docs/groups.dox`
**Changes:**
- Line 61: "React/Bootstrap equivalent" → "vanilla web component pattern"
- Line 73: "web frameworks (React, Vue)" → "web fundamentals (HTML, CSS, JavaScript)"
- Line 83: "Like Redux/Vuex store" → "Like window.appState global state"
- Line 91: "Like React render()" → "Like JavaScript component render()"
- Line 147: "like React components" → "like JavaScript component classes"
- Line 152: "like React render" → "like JavaScript component logic"

**Before:**
```cpp
/**
 * **Web Equivalent:** React component props
 * @code{.jsx}
 * <Card minSize={{width: 320, height: 200}} />
 * @endcode
 */
```

**After:**
```cpp
/**
 * **Web Equivalent:** HTML data attributes or JavaScript config object
 * @code{.html}
 * <div class="card" data-min-width="320" data-min-height="200">
 * </div>
 * @endcode
 */
```

---

## Philosophy Change

### Before
Documentation compared ImGui to modern JavaScript frameworks:
- React for component architecture
- Redux for state management
- JSX for declarative UI

### After
Documentation compares ImGui to **pure web fundamentals**:
- Vanilla JavaScript component pattern (ES6 classes)
- Global `window.appState` for state management
- Direct DOM manipulation or `innerHTML`

---

## Why This Change

**From user feedback:**
> "please don't compare to react and whatever bullshit framework. those are nothing to do with html css and js... please stick with pure html css and js since that is how i build all my applications in web."

**Reasoning:**
1. React/Redux/JSX are **transpiled abstractions** built on top of HTML/CSS/JS
2. User builds with **vanilla HTML/CSS/JS** (no frameworks)
3. ImGui philosophy aligns with **zero abstraction cost** (same as vanilla web)
4. Framework-free comparisons are **more universal** and timeless

---

## Vanilla JavaScript Pattern Used

### Component Pattern
```javascript
class RotorPanel {
  constructor(containerId) {
    this.container = document.getElementById(containerId);
  }

  render(state) {
    // Calculate data
    const avgRpm = state.rotor.rpm.reduce((a, b) => a + b, 0) / state.rotor.rpm.length;

    // Build HTML
    this.container.innerHTML = `
      <div class="card">
        <h2>Rotor Dynamics</h2>
        <p>${avgRpm.toFixed(0)} RPM</p>
      </div>
    `;
  }
}
```

### State Management Pattern
```javascript
// Global state (like window.appState)
const appState = {
  attitude: { quaternion: [1, 0, 0, 0] },
  rotor: { rpm: [0, 0, 0, 0] },
  control: { paused: false }
};

// Update state directly
function togglePause() {
  appState.control.paused = !appState.control.paused;
  renderAll(); // Re-render UI
}
```

### Component Registry Pattern
```javascript
const panels = [
  new RotorPanel('rotor-container'),
  new TelemetryPanel('telemetry-container')
];

function renderAll() {
  panels.forEach(panel => panel.render(appState));
}

// Update loop
setInterval(renderAll, 16); // ~60fps
```

---

## Comparison Table Update

### Before
| Feature | AeroDynControlRig |
|---------|-------------------|
| **Components** | Like React components |
| **State** | Like Redux store |
| **Rendering** | Like React render() |

### After
| Feature | AeroDynControlRig |
|---------|-------------------|
| **Components** | JavaScript component classes |
| **State** | Global `window.appState` object |
| **Rendering** | Re-render DOM every frame |

---

## Verification

Run these commands to verify all React/Redux references are removed:

```bash
# Check docs for React mentions
grep -r "React\|Redux\|jsx" /home/antshiv/Workspace/AeroDynControlRig/docs/ --include="*.md"

# Check source code for React mentions
grep -r "React\|Redux\|jsx" /home/antshiv/Workspace/AeroDynControlRig/src/ --include="*.h" --include="*.cpp"

# Expected: Only this cleanup document should show up
```

---

## Next Steps

If more React/Redux/JSX mentions are found:
1. Replace with vanilla JavaScript examples
2. Use `window.appState` for state management
3. Use ES6 class pattern for components
4. Use `innerHTML` or DOM manipulation for rendering
5. Update this document with new changes

---

## Notes

- All examples now use **pure HTML, CSS, and JavaScript**
- No transpilation required (JSX → JS)
- No build steps needed (npm, webpack, etc.)
- Matches the **C and CPU, zero abstraction cost** philosophy throughout the project

**Status:** ✅ Complete
**Last Verified:** 2025-10-25

## Summary of Changes

**Total files updated:** 3
1. `/docs/web_analogy.md` - 15+ React/Redux/JSX replacements
2. `/src/gui/widgets/card.h` - 2 replacements
3. `/docs/groups.dox` - 6 replacements

**Verification Results:**
- ✅ Source code (`.h`, `.cpp`): 0 React/Redux mentions
- ✅ Documentation (`.md`, `.dox`): 0 React/Redux mentions (excluding this cleanup document)
- ✅ All examples now use pure HTML, CSS, and JavaScript
- ✅ Philosophy aligned with "C and CPU, zero abstraction cost"
