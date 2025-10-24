#include "gui/widgets/card.h"

#include <algorithm>
#include <cfloat>

#include "gui/style.h"

namespace ui {

namespace {
constexpr float kBadgeCornerRadius = 12.0f;

ImVec2 ClampMinSize(const CardOptions& options) {
    return ImVec2(
        options.min_size.x > 0.0f ? options.min_size.x : 320.0f,
        options.min_size.y > 0.0f ? options.min_size.y : 200.0f);
}
}  // namespace

bool BeginCard(const char* id,
               const CardOptions& options,
               bool* open,
               ImGuiWindowFlags flags) {
    const Palette& palette = Colors();
    ImVec4 bg = palette.card_bg;
    bg.w = std::clamp(options.background_alpha, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg);
    ImGui::PushStyleColor(ImGuiCol_Border, palette.card_border);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 22.0f));

    if (options.enforce_min_size) {
        const ImVec2 min_size = ClampMinSize(options);
        ImGui::SetNextWindowSizeConstraints(min_size, ImVec2(FLT_MAX, FLT_MAX));
    }

    if (!options.allow_scrollbar) {
        flags |= ImGuiWindowFlags_NoScrollbar;
    }

    bool open_state = ImGui::Begin(id, open, flags);
    return open_state;
}

void EndCard() {
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void CardHeader(const char* title,
                const char* badge_label,
                const ImVec4* badge_color) {
    const Palette& palette = Colors();
    const FontSet& fonts = Fonts();

    const bool push_heading = fonts.heading != nullptr;
    if (push_heading) {
        ImGui::PushFont(fonts.heading);
    }
    ImGui::PushStyleColor(ImGuiCol_Text, palette.text_primary);
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
    if (push_heading) {
        ImGui::PopFont();
    }

    if (badge_label && badge_label[0] != '\0') {
        ImVec2 text_min = ImGui::GetItemRectMin();
        ImVec2 text_max = ImGui::GetItemRectMax();
        ImVec2 text_size = ImVec2(text_max.x - text_min.x, text_max.y - text_min.y);

        const float padding_x = 12.0f;
        const float padding_y = 4.0f;
        ImVec2 badge_text_size = ImGui::CalcTextSize(badge_label);
        ImVec2 badge_size = ImVec2(badge_text_size.x + padding_x * 2.0f,
                                   text_size.y + padding_y * 2.0f);

        const float content_right = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        ImVec2 badge_pos = ImVec2(content_right - badge_size.x, text_min.y - padding_y * 0.5f);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec4& badge_bg = badge_color ? *badge_color : palette.accent_base;
        const ImVec4 badge_text = ImVec4(0.976f, 0.992f, 1.0f, 1.0f);
        draw_list->AddRectFilled(badge_pos,
                                 ImVec2(badge_pos.x + badge_size.x, badge_pos.y + badge_size.y),
                                 ImColor(badge_bg),
                                 kBadgeCornerRadius);
        draw_list->AddText(ImVec2(badge_pos.x + padding_x,
                                  badge_pos.y + padding_y),
                           ImColor(badge_text),
                           badge_label);
        ImGui::Dummy(ImVec2(0.0f, badge_size.y + padding_y));
    } else {
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
    }
}

}  // namespace ui
