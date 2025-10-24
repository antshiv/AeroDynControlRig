#include "gui/style.h"

#include <array>
#include <filesystem>
#include <string>

#include "imgui.h"

namespace ui {
namespace {
FontSet g_fonts;

Palette MakePalette() {
    Palette palette{};
    palette.canvas_bg   = ImVec4(0.047f, 0.062f, 0.078f, 1.0f);  // #0c1014
    palette.card_bg     = ImVec4(0.067f, 0.086f, 0.094f, 1.0f);  // #111618
    palette.card_border = ImVec4(0.161f, 0.212f, 0.247f, 0.6f);  // #293640
    palette.card_header = ImVec4(0.094f, 0.122f, 0.141f, 1.0f);  // #182025

    palette.slate_base   = ImVec4(0.157f, 0.200f, 0.223f, 1.0f);  // #283339
    palette.slate_hover  = ImVec4(0.188f, 0.235f, 0.259f, 1.0f);  // #303d42
    palette.slate_active = ImVec4(0.129f, 0.168f, 0.192f, 1.0f);  // #213037

    palette.accent_base   = ImVec4(0.066f, 0.576f, 0.831f, 1.0f);  // #1193d4
    palette.accent_hover  = ImVec4(0.086f, 0.640f, 0.898f, 1.0f);  // lighter
    palette.accent_active = ImVec4(0.047f, 0.470f, 0.733f, 1.0f);  // darker

    palette.text_primary = ImVec4(0.862f, 0.937f, 0.972f, 1.0f);
    palette.text_muted   = ImVec4(0.596f, 0.682f, 0.725f, 1.0f);

    palette.success = ImVec4(0.223f, 0.760f, 0.431f, 1.0f);
    palette.warning = ImVec4(0.964f, 0.749f, 0.239f, 1.0f);
    palette.danger  = ImVec4(0.941f, 0.329f, 0.274f, 1.0f);

    return palette;
}

Palette g_palette = MakePalette();

std::string JoinPath(std::string_view directory, std::string_view file) {
    if (directory.empty()) {
        return std::string(file);
    }

    std::filesystem::path dir_path(directory);
    dir_path /= file;
    return dir_path.string();
}

ImFont* LoadFont(ImGuiIO& io, const std::string& path, float size, const ImWchar* range = nullptr, bool merge = false) {
    if (path.empty() || !std::filesystem::exists(path)) {
        return nullptr;
    }

    ImFontConfig config;
    config.MergeMode = merge;
    config.PixelSnapH = false;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.GlyphOffset = ImVec2(0.0f, 0.0f);

    if (merge) {
        config.GlyphOffset.y = -1.0f;
    }

    return io.Fonts->AddFontFromFileTTF(path.c_str(), size, &config, range);
}

}  // namespace

void ApplyTheme(ImGuiStyle& style) {
    style.WindowPadding = ImVec2(26.0f, 22.0f);
    style.FramePadding = ImVec2(16.0f, 12.0f);
    style.CellPadding = ImVec2(12.0f, 8.0f);
    style.ItemSpacing = ImVec2(14.0f, 12.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.ScrollbarSize = 16.0f;

    style.WindowRounding = 18.0f;
    style.ChildRounding = 16.0f;
    style.FrameRounding = 12.0f;
    style.GrabRounding = 12.0f;
    style.PopupRounding = 14.0f;
    style.TabRounding = 10.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    style.Colors[ImGuiCol_WindowBg] = g_palette.card_bg;
    style.Colors[ImGuiCol_ChildBg] = ImVec4(g_palette.card_bg.x, g_palette.card_bg.y, g_palette.card_bg.z, 0.9f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.067f, 0.086f, 0.102f, 0.98f);
    style.Colors[ImGuiCol_Border] = g_palette.card_border;
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    style.Colors[ImGuiCol_FrameBg] = g_palette.slate_base;
    style.Colors[ImGuiCol_FrameBgHovered] = g_palette.slate_hover;
    style.Colors[ImGuiCol_FrameBgActive] = g_palette.slate_active;

    style.Colors[ImGuiCol_TitleBg] = g_palette.card_header;
    style.Colors[ImGuiCol_TitleBgCollapsed] = g_palette.card_header;
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(g_palette.card_header.x, g_palette.card_header.y, g_palette.card_header.z, 1.0f);

    style.Colors[ImGuiCol_Header] = g_palette.slate_base;
    style.Colors[ImGuiCol_HeaderHovered] = g_palette.slate_hover;
    style.Colors[ImGuiCol_HeaderActive] = g_palette.slate_active;

    style.Colors[ImGuiCol_Button] = g_palette.slate_base;
    style.Colors[ImGuiCol_ButtonHovered] = g_palette.slate_hover;
    style.Colors[ImGuiCol_ButtonActive] = g_palette.slate_active;

    style.Colors[ImGuiCol_CheckMark] = g_palette.accent_base;
    style.Colors[ImGuiCol_SliderGrab] = g_palette.accent_base;
    style.Colors[ImGuiCol_SliderGrabActive] = g_palette.accent_hover;

    style.Colors[ImGuiCol_Text] = g_palette.text_primary;
    style.Colors[ImGuiCol_TextDisabled] = g_palette.text_muted;

    style.Colors[ImGuiCol_Tab] = g_palette.slate_base;
    style.Colors[ImGuiCol_TabHovered] = g_palette.slate_hover;
    style.Colors[ImGuiCol_TabActive] = g_palette.accent_base;

    style.Colors[ImGuiCol_DockingPreview] = ImVec4(g_palette.accent_base.x, g_palette.accent_base.y, g_palette.accent_base.z, 0.4f);
    style.Colors[ImGuiCol_NavHighlight] = style.Colors[ImGuiCol_DockingPreview];
}

bool LoadFonts(ImGuiIO& io, std::string_view font_directory) {
    g_fonts = FontSet{};

    constexpr float kBaseFont = 18.0f;
    constexpr float kHeadingFont = 22.0f;
    constexpr float kMonoFont = 17.0f;
    constexpr float kIconFont = 20.0f;

    const std::string body_path = JoinPath(font_directory, "SpaceGrotesk-Regular.ttf");
    const std::string heading_path = JoinPath(font_directory, "SpaceGrotesk-SemiBold.ttf");
    const std::string mono_path = JoinPath(font_directory, "SpaceGrotesk-Medium.ttf");
    const std::string icon_path = JoinPath(font_directory, "MaterialSymbolsRounded.ttf");

    g_fonts.body = LoadFont(io, body_path, kBaseFont);
    if (!g_fonts.body) {
        g_fonts.body = io.Fonts->AddFontDefault();
    }

    g_fonts.heading = LoadFont(io, heading_path, kHeadingFont);
    if (!g_fonts.heading) {
        g_fonts.heading = g_fonts.body;
    }

    g_fonts.mono = LoadFont(io, mono_path, kMonoFont);
    if (!g_fonts.mono) {
        ImFontConfig mono_config;
        mono_config.OversampleH = mono_config.OversampleV = 1;
        mono_config.PixelSnapH = true;
        mono_config.SizePixels = kMonoFont;
        g_fonts.mono = io.Fonts->AddFontDefault(&mono_config);
        if (!g_fonts.mono) {
            g_fonts.mono = g_fonts.body;
        }
    }

    static const std::array<ImWchar, 3> icon_range = {0xe000, 0xf8ff, 0};
    g_fonts.icon = LoadFont(io, icon_path, kIconFont, icon_range.data(), true);

    io.FontDefault = g_fonts.body;
    return g_fonts.body != nullptr;
}

const FontSet& Fonts() {
    return g_fonts;
}

const Palette& Colors() {
    return g_palette;
}

void PushPillButtonStyle(PillStyle style) {
    const Palette& palette = Colors();

    ImVec4 base;
    ImVec4 hover;
    ImVec4 active;
    ImVec4 text = palette.text_primary;

    if (style == PillStyle::Primary) {
        base = palette.accent_base;
        hover = palette.accent_hover;
        active = palette.accent_active;
        text = ImVec4(0.976f, 0.992f, 1.0f, 1.0f);
    } else {
        base = palette.slate_base;
        hover = palette.slate_hover;
        active = palette.slate_active;
    }

    ImGui::PushStyleColor(ImGuiCol_Button, base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
    ImGui::PushStyleColor(ImGuiCol_Text, text);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.0f, 10.0f));
}

void PopPillButtonStyle() {
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
}

}  // namespace ui
