#pragma once

namespace FWK::Editor
{
    // ImGuiの見た目(余白、角丸、配色)をUnrealEngine5のエディタ風に設定するクラス
    // 配色はDefinition/Constant/Editor/Style/EditorStyleConstant.hにまとめてあり、
    // ここではImGuiの各部品(ImGuiCol_～)にどの色を割り当てるかだけを決める
    class EditorStyle final
    {
    public:

         EditorStyle() = default;
        ~EditorStyle() = default;

        static void ApplyUnrealEngineDarkStyle();
        static void ApplyFont                 ();

    private:

        static void ApplyStyleSize ();
        static void ApplyStyleColor();

        inline static const std::filesystem::path k_fontPath     = "Asset/Data/Font/Cica/Cica-Regular.ttf";
        inline static const std::filesystem::path k_iconFontPath = "Asset/Data/Font/Icon/FontAwesome/Font Awesome 7 Free-Solid-900.otf";

        static constexpr ImWchar k_iconGlyphRanges[] = { 0xE000, 0xF8FF, 0 };

        static constexpr float k_editorGlobalScale = 1.0F;

        static constexpr float k_windowRounding    = 0.0F;
        static constexpr float k_childRounding     = 0.0F;
        static constexpr float k_frameRounding     = 4.0F;
        static constexpr float k_popupRounding     = 4.0F;
        static constexpr float k_scrollbarRounding = 8.0F;
        static constexpr float k_grabRounding      = 4.0F;
        static constexpr float k_tabRounding       = 4.0F;

        static constexpr float k_windowBorderSize = 1.0F;
        static constexpr float k_childBorderSize  = 1.0F;
        static constexpr float k_popupBorderSize  = 1.0F;
        static constexpr float k_frameBorderSize  = 0.0F;
        static constexpr float k_tabBorderSize    = 0.0F;

        static constexpr float k_tabBarOverlineSize = 2.0F;

        static constexpr float k_dockingSeparatorSize = 2.0F;

        static constexpr float k_windowPaddingX = 8.0F;
        static constexpr float k_windowPaddingY = 8.0F;

        static constexpr float k_framePaddingX = 8.0F;
        static constexpr float k_framePaddingY = 4.0F;

        static constexpr float k_itemSpacingX = 8.0F;
        static constexpr float k_itemSpacingY = 4.0F;

        static constexpr float k_indentSpacing = 16.0F;
        static constexpr float k_scrollbarSize = 12.0F;

        static constexpr int k_fontHorizontalOversample = 3;
        static constexpr int k_fontVerticalOversample   = 1;

        static constexpr float k_editorFontSize = 12.0F;
    };
}