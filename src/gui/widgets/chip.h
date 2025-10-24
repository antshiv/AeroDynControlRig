#ifndef GUI_WIDGETS_CHIP_H
#define GUI_WIDGETS_CHIP_H

#include "imgui.h"

namespace ui {

enum class ChipVariant {
    Neutral,
    Positive,
    Negative
};

struct ChipConfig {
    float min_width = 0.0f;
    ChipVariant variant = ChipVariant::Neutral;
};

ImVec2 ValueChip(const char* label,
                 const char* value,
                 const ChipConfig& config = ChipConfig());

}  // namespace ui

#endif  // GUI_WIDGETS_CHIP_H
