#ifndef GUI_WIDGETS_CARD_H
#define GUI_WIDGETS_CARD_H

#include "imgui.h"

/**
 * @defgroup UIWidgets UI Widget Library
 * @ingroup WebAnalogy
 * @brief Custom ImGui widgets that mirror web component libraries
 *
 * This module provides reusable UI components similar to React/Vue component libraries
 * or CSS frameworks like Bootstrap. Each widget encapsulates styling, layout, and behavior.
 *
 * **Web Analogy:**
 * - `BeginCard/EndCard` = `<Card>` component (React) or `.card` class (Bootstrap)
 * - `CardHeader` = `<CardHeader>` with optional badge
 * - `ValueChip` = `<span class="badge">` or Material-UI Chip
 *
 * **Why Custom Widgets?**
 * 1. **Consistency:** Enforces uniform spacing, colors, and layout
 * 2. **Reusability:** Write once, use in all panels
 * 3. **Maintainability:** Change styling in one place
 *
 * @see @ref web_analogy for detailed web development mapping
 * @{
 */

namespace ui {

/**
 * @brief Configuration options for Card widget
 *
 * **Web Equivalent:** React component props
 * @code{.jsx}
 * <Card
 *   minSize={{width: 320, height: 200}}
 *   enforceMinSize={true}
 *   allowScrollbar={false}
 *   backgroundAlpha={1.0}
 * />
 * @endcode
 *
 * **CSS Equivalent:**
 * @code{.css}
 * .card {
 *   min-width: 320px;
 *   min-height: 200px;
 *   background-color: rgba(..., 1.0);
 *   overflow: hidden; // if !allow_scrollbar
 * }
 * @endcode
 */
struct CardOptions {
    ImVec2 min_size = ImVec2(320.0f, 200.0f);  ///< Minimum card dimensions (CSS: min-width, min-height)
    bool enforce_min_size = true;              ///< Whether to constrain size (default: yes)
    bool allow_scrollbar = false;              ///< Enable scrolling for overflow content
    float background_alpha = 1.0f;             ///< Background opacity (0.0 = transparent, 1.0 = opaque)
};

/**
 * @brief Begin a Card container with consistent styling
 *
 * **Web Analogy:**
 * @code{.html}
 * <div class="card" id="myCard">
 *   <!-- content goes here -->
 * </div>
 * @endcode
 *
 * **What This Does:**
 * 1. Applies card background color from palette
 * 2. Sets border color and 1px border width
 * 3. Applies 18px corner rounding
 * 4. Sets padding to 24px horizontal, 22px vertical
 * 5. Enforces minimum size constraints (if enabled)
 *
 * **Memory Flow (Hardware Level):**
 * - CPU pushes style values onto ImGui's internal stack (in RAM)
 * - Creates ImGui window with specified ID
 * - When frame renders, vertex data generated in RAM, uploaded to GPU VRAM
 * - GPU rasterizes rounded rectangle and fills with background color
 *
 * @param id Unique identifier for this card (like HTML `id` attribute)
 * @param options Card configuration (see CardOptions)
 * @param open Pointer to bool for closeable cards (nullptr = not closeable)
 * @param flags Additional ImGui window flags (e.g., ImGuiWindowFlags_NoTitleBar)
 * @return true if card is open and should render content, false if closed/collapsed
 *
 * @note Always call EndCard() after BeginCard(), even if it returns false
 *
 * **Example Usage:**
 * @code{.cpp}
 * ui::CardOptions options;
 * options.min_size = ImVec2(320.0f, 360.0f);
 *
 * if (ui::BeginCard("MyPanel", options, nullptr, ImGuiWindowFlags_NoTitleBar)) {
 *     ui::CardHeader("Title", "Badge Text", &palette.success);
 *     ImGui::Text("Content goes here");
 * }
 * ui::EndCard();
 * @endcode
 *
 * @see EndCard(), CardHeader()
 * @see gui/widgets/card.cpp for implementation details
 */
bool BeginCard(const char* id,
               const CardOptions& options = CardOptions(),
               bool* open = nullptr,
               ImGuiWindowFlags flags = 0);

/**
 * @brief End a Card container (must be called after BeginCard)
 *
 * **Web Analogy:**
 * @code{.html}
 * </div>  <!-- closes the .card div -->
 * @endcode
 *
 * **What This Does:**
 * 1. Calls ImGui::End() to close the window
 * 2. Pops 3 style variables from stack (rounding, border, padding)
 * 3. Pops 2 style colors from stack (background, border)
 *
 * **Why Stack-Based?**
 * ImGui uses a stack-based styling system (like OpenGL state machine).
 * Push/Pop ensures styles only apply to this card, not other widgets.
 *
 * @note Must be called even if BeginCard() returned false
 */
void EndCard();

/**
 * @brief Render a card header with optional badge
 *
 * **Web Analogy:**
 * @code{.html}
 * <div class="card-header">
 *   <h2>Title</h2>
 *   <span class="badge badge-success">Real-time +2.5%</span>
 * </div>
 * @endcode
 *
 * **CSS Equivalent:**
 * @code{.css}
 * .card-header h2 {
 *   font-family: 'SpaceGrotesk-SemiBold';
 *   font-size: 22px;
 *   color: #dceef8;
 * }
 * .badge {
 *   background-color: var(--success);
 *   border-radius: 12px;
 *   padding: 4px 12px;
 *   float: right;
 * }
 * @endcode
 *
 * **Layout:**
 * ```
 * ┌────────────────────────────────────────────┐
 * │ Title                      [Badge Text]    │ ← Header
 * │                                            │
 * │ (card content below)                       │
 * ```
 *
 * **GPU Rendering:**
 * 1. Title text rendered using heading font (22px SemiBold)
 * 2. Badge background drawn as rounded rectangle using ImDrawList
 * 3. Badge text rendered on top with white color
 *
 * @param title Main heading text (left-aligned)
 * @param badge_label Optional badge text (right-aligned, can be nullptr)
 * @param badge_color Optional badge background color (uses accent color if nullptr)
 *
 * **Example:**
 * @code{.cpp}
 * const ui::Palette& palette = ui::Colors();
 *
 * // Header with success badge (green)
 * ui::CardHeader("Rotor Dynamics", "Real-time +2.5%", &palette.success);
 *
 * // Header with danger badge (red)
 * ui::CardHeader("Rotor Dynamics", "Offline", &palette.danger);
 *
 * // Header with no badge
 * ui::CardHeader("Telemetry");
 * @endcode
 *
 * @see BeginCard(), gui/style.h (for Palette colors)
 */
void CardHeader(const char* title,
                const char* badge_label = nullptr,
                const ImVec4* badge_color = nullptr);

/** @} */ // end of UIWidgets group

}  // namespace ui

#endif  // GUI_WIDGETS_CARD_H
