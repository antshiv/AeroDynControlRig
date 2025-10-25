#ifndef GUI_STYLE_H
#define GUI_STYLE_H

#include <string_view>

#include "imgui.h"

struct ImFont;
struct ImGuiIO;
struct ImGuiStyle;

/**
 * @defgroup UITheme UI Theme System
 * @ingroup WebAnalogy
 * @brief Global styling system (design tokens, typography, theming)
 *
 * This module is the **CSS equivalent** for AeroDynControlRig. It defines:
 * - Color palette (design tokens)
 * - Typography (font families and sizes)
 * - Global theme application
 *
 * **Web Analogy:**
 * @code{.css}
 * :root {
 *   --canvas-bg: #0c1014;
 *   --card-bg: #111618;
 *   --accent-base: #1193d4;
 * }
 *
 * body { font: 18px 'SpaceGrotesk'; }
 * h1 { font: 22px 'SpaceGrotesk SemiBold'; }
 * @endcode
 *
 * **Why Centralized Styling?**
 * 1. **Consistency:** All panels use the same colors and fonts
 * 2. **Themability:** Change entire app appearance from one place
 * 3. **Maintainability:** Update design tokens without touching panel code
 *
 * @see @ref web_analogy for web development mapping
 * @see @ref gpu_pipeline for how styles are rendered to GPU
 * @{
 */

namespace ui {

/**
 * @brief Typography system (font families and sizes)
 *
 * **Web Equivalent:**
 * @code{.css}
 * @font-face {
 *   font-family: 'SpaceGrotesk';
 *   src: url('SpaceGrotesk-Regular.ttf');
 * }
 *
 * body { font-family: 'SpaceGrotesk', sans-serif; font-size: 18px; }
 * h1 { font-family: 'SpaceGrotesk SemiBold'; font-size: 22px; }
 * code { font-family: 'SpaceGrotesk Medium'; font-size: 17px; }
 * .icon { font-family: 'Material Symbols Rounded'; font-size: 20px; }
 * @endcode
 *
 * **Font Files:**
 * - `assets/fonts/SpaceGrotesk-Regular.ttf` (body)
 * - `assets/fonts/SpaceGrotesk-SemiBold.ttf` (heading)
 * - `assets/fonts/SpaceGrotesk-Medium.ttf` (mono)
 * - `assets/fonts/MaterialSymbolsRounded.ttf` (icon)
 *
 * **Memory Details:**
 * - Font atlas generated in RAM at startup (~2-4 MB)
 * - Uploaded to GPU VRAM as texture
 * - Glyphs rendered as textured quads (GPU samples atlas texture)
 *
 * **Usage:**
 * @code{.cpp}
 * const ui::FontSet& fonts = ui::Fonts();
 *
 * // Use heading font
 * if (fonts.heading) ImGui::PushFont(fonts.heading);
 * ImGui::Text("Large Title");
 * if (fonts.heading) ImGui::PopFont();
 *
 * // Use mono font for code
 * if (fonts.mono) ImGui::PushFont(fonts.mono);
 * ImGui::Text("quaternion = [%.3f, %.3f, %.3f, %.3f]", q.w, q.x, q.y, q.z);
 * if (fonts.mono) ImGui::PopFont();
 * @endcode
 */
struct FontSet {
    ImFont* body = nullptr;     ///< 18px SpaceGrotesk-Regular (default font)
    ImFont* heading = nullptr;  ///< 22px SpaceGrotesk-SemiBold (for titles)
    ImFont* mono = nullptr;     ///< 17px SpaceGrotesk-Medium (for numeric data)
    ImFont* icon = nullptr;     ///< 20px MaterialSymbolsRounded (icon glyphs)
};

/**
 * @brief Color palette (design tokens)
 *
 * Web Equivalent (CSS Custom Properties):
 * @verbatim
 * :root {
 *   --canvas-bg: #0c1014;
 *   --card-bg: #111618;
 *   --card-border: #293640;
 *   --card-header: #182025;
 *   --slate-base: #283339;
 *   --slate-hover: #303d42;
 *   --slate-active: #213037;
 *   --accent-base: #1193d4;
 *   --accent-hover: #16a4e5;
 *   --accent-active: #0c78bb;
 *   --text-primary: #dceef8;
 *   --text-muted: #98aeb9;
 *   --success: #39c26e;
 *   --warning: #f6bf3d;
 *   --danger: #f05446;
 * }
 * @endverbatim
 *
 * **Color Format:**
 * - ImVec4 stores RGBA as floats (0.0-1.0 range)
 * - Example: #1193d4 = ImVec4(0.066, 0.576, 0.831, 1.0)
 * - Alpha channel always 1.0 (fully opaque) unless transparency needed
 *
 * **Usage:**
 * @code{.cpp}
 * const ui::Palette& palette = ui::Colors();
 *
 * // Apply text color
 * ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
 * ImGui::TextUnformatted("Label");
 * ImGui::PopStyleColor();
 *
 * // Draw custom colored rectangle
 * ImDrawList* draw_list = ImGui::GetWindowDrawList();
 * draw_list->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(palette.success));
 * @endcode
 *
 * **Hardware Path:**
 * When you use a color:
 * 1. CPU reads color from palette (RAM, likely in L1 cache)
 * 2. CPU converts to 32-bit RGBA (bit packing)
 * 3. Color sent to GPU as vertex attribute or uniform
 * 4. GPU fragment shader outputs color to framebuffer (VRAM)
 */
struct Palette {
    // Background colors
    ImVec4 canvas_bg;    ///< #0c1014 - Application canvas background
    ImVec4 card_bg;      ///< #111618 - Card/panel background
    ImVec4 card_border;  ///< #293640 - Card border color (with alpha)
    ImVec4 card_header;  ///< #182025 - Card header background

    // Interactive element colors (slate = neutral)
    ImVec4 slate_base;   ///< #283339 - Default button/input background
    ImVec4 slate_hover;  ///< #303d42 - Hover state
    ImVec4 slate_active; ///< #213037 - Active/pressed state

    // Accent colors (blue = primary action)
    ImVec4 accent_base;   ///< #1193d4 - Primary action color
    ImVec4 accent_hover;  ///< Lighter blue - Hover state
    ImVec4 accent_active; ///< Darker blue - Active state

    // Text colors
    ImVec4 text_primary; ///< #dceef8 - High contrast text
    ImVec4 text_muted;   ///< #98aeb9 - Lower contrast (labels, secondary)

    // Semantic colors (status indication)
    ImVec4 success; ///< #39c26e - Green (positive, connected, OK)
    ImVec4 warning; ///< #f6bf3d - Yellow (caution, attention)
    ImVec4 danger;  ///< #f05446 - Red (error, critical, offline)
};

/**
 * @brief Apply global theme to ImGui style
 *
 * **Web Equivalent:**
 * Loading a global stylesheet:
 * @code{.html}
 * <link rel="stylesheet" href="theme-dark.css">
 * @endcode
 *
 * **What This Does:**
 * Sets 90+ ImGui style parameters:
 * - **Spacing:** Window padding, item spacing, frame padding
 * - **Rounding:** Corner radii for windows, buttons, sliders
 * - **Borders:** Border widths for windows, frames, tabs
 * - **Colors:** All ImGui color slots (background, text, buttons, etc.)
 *
 * **Implementation Details:**
 * @code{.cpp}
 * void ApplyTheme(ImGuiStyle& style) {
 *     // Spacing
 *     style.WindowPadding = ImVec2(26.0f, 22.0f);     // padding: 26px 22px
 *     style.ItemSpacing = ImVec2(14.0f, 12.0f);       // gap: 14px 12px
 *
 *     // Rounding
 *     style.WindowRounding = 18.0f;                   // border-radius: 18px
 *     style.FrameRounding = 12.0f;                    // border-radius: 12px
 *
 *     // Colors (90+ assignments)
 *     style.Colors[ImGuiCol_WindowBg] = palette.card_bg;
 *     style.Colors[ImGuiCol_Button] = palette.slate_base;
 *     // ... etc
 * }
 * @endcode
 *
 * @param style Reference to ImGui's global style object
 *
 * @note Call this once during initialization (in Application::init())
 * @see gui/style.cpp:71-125 for complete implementation
 */
void ApplyTheme(ImGuiStyle& style);

/**
 * @brief Load custom fonts from TTF files
 *
 * **Web Equivalent:**
 * @code{.css}
 * @font-face {
 *   font-family: 'SpaceGrotesk';
 *   src: url('assets/fonts/SpaceGrotesk-Regular.ttf') format('truetype');
 * }
 * @endcode
 *
 * **What This Does:**
 * 1. Loads 4 TTF files from disk into RAM
 * 2. Rasterizes glyphs at specified sizes (uses FreeType library)
 * 3. Packs glyphs into texture atlas (single 2D image)
 * 4. Uploads atlas to GPU VRAM
 * 5. Stores font metadata (glyph positions, advance widths)
 *
 * **Font Sizes:**
 * - Body: 18px (default)
 * - Heading: 22px (larger, bolder)
 * - Mono: 17px (monospace for numbers)
 * - Icon: 20px (Material Symbols icon font)
 *
 * **Memory Overhead:**
 * - ~2-4 MB in RAM (font atlas texture)
 * - ~2-4 MB in GPU VRAM (uploaded once at startup)
 *
 * **Performance:**
 * - Text rendering: GPU samples font atlas texture
 * - Each character = 2 triangles (quad) with texture coordinates
 * - Very fast (text is batched into single draw call per font)
 *
 * @param io ImGui IO object (contains font atlas)
 * @param font_directory Path to font files (default: "assets/fonts")
 * @return true if all fonts loaded successfully, false if fallback used
 *
 * @note Call this once during initialization, before first frame
 * @see gui/style.cpp:127-167 for implementation
 */
bool LoadFonts(ImGuiIO& io, std::string_view font_directory = "assets/fonts");

/**
 * @brief Get global font set
 *
 * **Web Equivalent:**
 * Accessing CSS font family:
 * @code{.javascript}
 * const heading = window.getComputedStyle(element).fontFamily;
 * @endcode
 *
 * @return Reference to global FontSet (loaded at startup)
 *
 * **Thread Safety:** Read-only after initialization, safe for multi-threaded access
 *
 * @see FontSet, LoadFonts()
 */
const FontSet& Fonts();

/**
 * @brief Get global color palette
 *
 * **Web Equivalent:**
 * Accessing CSS custom property:
 * @code{.javascript}
 * const accentColor = getComputedStyle(document.documentElement)
 *                         .getPropertyValue('--accent-base');
 * @endcode
 *
 * @return Reference to global Palette (const after initialization)
 *
 * **Performance:** Returns reference, no copy overhead (palette is ~200 bytes)
 *
 * @see Palette
 */
const Palette& Colors();

/**
 * @brief Button style variants (like Bootstrap button classes)
 *
 * **Web Equivalent:**
 * @code{.html}
 * <button class="btn btn-primary">Save</button>   <!-- Primary -->
 * <button class="btn btn-secondary">Cancel</button> <!-- Secondary -->
 * @endcode
 */
enum class PillStyle {
    Primary,   ///< Accent color (blue) - primary action
    Secondary  ///< Slate color (gray) - secondary action
};

/**
 * @brief Push pill button styling onto ImGui stack
 *
 * **Web Equivalent:**
 * Applying Bootstrap button classes:
 * @code{.css}
 * .btn-primary {
 *   background-color: var(--accent-base);
 *   border-radius: 24px;
 *   padding: 20px 10px;
 * }
 * @endcode
 *
 * **What This Does:**
 * 1. Pushes 4 color overrides (button, hover, active, text)
 * 2. Pushes 2 style variables (rounding: 24px, padding: 20px 10px)
 *
 * **Usage (Stack-Based):**
 * @code{.cpp}
 * ui::PushPillButtonStyle(ui::PillStyle::Primary);
 * if (ImGui::Button("Save")) {
 *     // Primary action
 * }
 * ui::PopPillButtonStyle();
 *
 * ui::PushPillButtonStyle(ui::PillStyle::Secondary);
 * if (ImGui::Button("Cancel")) {
 *     // Secondary action
 * }
 * ui::PopPillButtonStyle();
 * @endcode
 *
 * @param style Primary (blue) or Secondary (gray)
 *
 * @note Must call PopPillButtonStyle() after button rendering
 * @see PopPillButtonStyle()
 */
void PushPillButtonStyle(PillStyle style);

/**
 * @brief Pop pill button styling from ImGui stack
 *
 * **What This Does:**
 * - Pops 2 style variables (rounding, padding)
 * - Pops 4 style colors (button, hover, active, text)
 *
 * @note Must be called after PushPillButtonStyle() to restore previous style
 */
void PopPillButtonStyle();

/** @} */ // end of UITheme group

}  // namespace ui

#endif  // GUI_STYLE_H
