#ifndef GUI_WIDGETS_CARD_H
#define GUI_WIDGETS_CARD_H

#include "imgui.h"

namespace ui {

struct CardOptions {
    ImVec2 min_size = ImVec2(320.0f, 200.0f);
    bool enforce_min_size = true;
    bool allow_scrollbar = false;
    float background_alpha = 1.0f;
};

bool BeginCard(const char* id,
               const CardOptions& options = CardOptions(),
               bool* open = nullptr,
               ImGuiWindowFlags flags = 0);
void EndCard();

void CardHeader(const char* title,
                const char* badge_label = nullptr,
                const ImVec4* badge_color = nullptr);

}  // namespace ui

#endif  // GUI_WIDGETS_CARD_H
