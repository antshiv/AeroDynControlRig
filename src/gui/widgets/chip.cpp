#include "gui/widgets/chip.h"

#include <algorithm>
#include <string_view>

#include "gui/style.h"

namespace ui {

namespace {
struct ChipStyle {
    ImVec4 background;
    ImVec4 border;
    ImVec4 text;
    ImVec4 label;
};

ChipStyle ResolveStyle(ChipVariant variant) {
    const Palette& palette = Colors();
    ChipStyle style{};
    style.background = palette.slate_base;
    style.border = ImVec4(palette.card_border.x, palette.card_border.y, palette.card_border.z, 0.8f);
    style.text = palette.text_primary;
    style.label = palette.text_muted;

    switch (variant) {
        case ChipVariant::Positive:
            style.border = palette.success;
            style.text = palette.success;
            break;
        case ChipVariant::Negative:
            style.border = palette.danger;
            style.text = palette.danger;
            break;
        case ChipVariant::Neutral:
        default:
            break;
    }

    return style;
}
}  // namespace

ImVec2 ValueChip(const char* label,
                 const char* value,
                 const ChipConfig& config) {
    const float padding_x = 16.0f;
    const float padding_y = 10.0f;
    const float value_offset = 6.0f;
    const float rounding = 12.0f;

    ChipStyle style = ResolveStyle(config.variant);
    const FontSet& fonts = Fonts();

    ImFont* label_font = ImGui::GetFont();
    float label_height = label_font ? label_font->FontSize : ImGui::GetFontSize();
    ImVec2 label_size = ImGui::CalcTextSize(label);

    ImFont* value_font = fonts.mono ? fonts.mono : ImGui::GetFont();
    float value_height = value_font ? value_font->FontSize : ImGui::GetFontSize();
    ImVec2 value_size = ImGui::CalcTextSize(value);

    float width = std::max({config.min_width,
                            label_size.x + padding_x * 2.0f,
                            value_size.x + padding_x * 2.0f});

    float height = padding_y * 2.0f + label_height + value_height + value_offset;

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 rect_min = cursor;
    ImVec2 rect_max = ImVec2(cursor.x + width, cursor.y + height);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(rect_min, rect_max, ImColor(style.background), rounding);
    draw_list->AddRect(rect_min, rect_max, ImColor(style.border), rounding);

    ImVec2 label_pos = ImVec2(rect_min.x + padding_x, rect_min.y + padding_y);
    draw_list->AddText(label_font, label_height, label_pos, ImGui::ColorConvertFloat4ToU32(style.label), label);

    ImVec2 value_pos = ImVec2(rect_min.x + padding_x,
                              rect_min.y + padding_y + label_height + value_offset);
    draw_list->AddText(value_font,
                       value_height,
                       value_pos,
                       ImGui::ColorConvertFloat4ToU32(style.text),
                       value);

    ImGui::Dummy(ImVec2(width, height));
    return ImVec2(width, height);
}

}  // namespace ui
