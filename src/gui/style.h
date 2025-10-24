#ifndef GUI_STYLE_H
#define GUI_STYLE_H

#include <string_view>

#include "imgui.h"

struct ImFont;
struct ImGuiIO;
struct ImGuiStyle;

namespace ui {

struct FontSet {
    ImFont* body = nullptr;
    ImFont* heading = nullptr;
    ImFont* mono = nullptr;
    ImFont* icon = nullptr;
};

struct Palette {
    ImVec4 canvas_bg;
    ImVec4 card_bg;
    ImVec4 card_border;
    ImVec4 card_header;
    ImVec4 slate_base;
    ImVec4 slate_hover;
    ImVec4 slate_active;
    ImVec4 accent_base;
    ImVec4 accent_hover;
    ImVec4 accent_active;
    ImVec4 text_primary;
    ImVec4 text_muted;
    ImVec4 success;
    ImVec4 warning;
    ImVec4 danger;
};

void ApplyTheme(ImGuiStyle& style);
bool LoadFonts(ImGuiIO& io, std::string_view font_directory = "assets/fonts");

const FontSet& Fonts();
const Palette& Colors();

enum class PillStyle {
    Primary,
    Secondary
};

void PushPillButtonStyle(PillStyle style);
void PopPillButtonStyle();

}  // namespace ui

#endif  // GUI_STYLE_H
