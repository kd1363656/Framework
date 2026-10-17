#pragma once

namespace FWK::Editor
{
    class EditorStyle final
    {
    public:

         EditorStyle() = default;
        ~EditorStyle() = default;

        static void ApplyUnrealDarkStyle();
        static void ApplyFont           ();

    private:

        static ImVec4 ConvertEditorColorToIMVEC4(const TypeAlias::Math::Color& a_color);

        static constexpr ImWchar k_iconGlyphRanges[] = { 0xE000, 0xF8FF, 0 };

        static constexpr TypeAlias::Math::Color k_clearColor = { 0.059F,
                                                                 0.059F,
                                                                 0.059F,
                                                                 1.000F };

        static constexpr TypeAlias::Math::Color k_titleColor = { 0.082F,
                                                                 0.082F,
                                                                 0.082F,
                                                                 1.000F };

        static constexpr TypeAlias::Math::Color k_recessedColor = { 0.102F,
                                                                    0.102F,
                                                                    0.102F,
                                                                    1.000F };

        static constexpr TypeAlias::Math::Color k_backgroundColor = { 0.141F,
                                                                      0.141F,
                                                                      0.141F,
                                                                      1.000F };

        static constexpr TypeAlias::Math::Color k_panelColor = { 0.184F,
                                                                 0.184F,
                                                                 0.184F,
                                                                 1.000F };

        static constexpr TypeAlias::Math::Color k_panelHoverColor = { 0.220F,
                                                                      0.220F,
                                                                      0.220F,
                                                                      1.000F };

        static constexpr TypeAlias::Math::Color k_panelActiveColor = { 0.341F,
                                                                       0.341F,
                                                                       0.341F,
                                                                       1.000F };

        static constexpr TypeAlias::Math::Color k_borderColor = { 0.059F,
                                                                  0.059F,
                                                                  0.059F,
                                                                  1.000F };

        static constexpr TypeAlias::Math::Color k_borderShadowColor = { 0.000F,
                                                                        0.000F,
                                                                        0.000F,
                                                                        0.000F };

        static constexpr TypeAlias::Math::Color k_separatorColor = { 0.184F,
                                                                     0.184F,
                                                                     0.184F,
                                                                     1.000F };

        static constexpr TypeAlias::Math::Color k_textColor = { 0.753F,
                                                                0.753F,
                                                                0.753F,
                                                                1.000F };

        static constexpr TypeAlias::Math::Color k_textDisabledColor = { 0.502F,
                                                                        0.502F,
                                                                        0.502F,
                                                                        1.000F };

        static constexpr TypeAlias::Math::Color k_accentColor = { 0.000F,
                                                                  0.439F,
                                                                  0.878F,
                                                                  1.000F };

        static constexpr TypeAlias::Math::Color k_accentHoverColor = { 0.055F,
                                                                       0.525F,
                                                                       1.000F,
                                                                       1.000F };

        static constexpr TypeAlias::Math::Color k_accentActiveColor = { 0.000F,
                                                                        0.314F,
                                                                        0.627F,
                                                                        1.000F };

        static constexpr TypeAlias::Math::Color k_textSelectedColor = { 0.000F,
                                                                        0.439F,
                                                                        0.878F,
                                                                        0.500F };

        static constexpr TypeAlias::Math::Color k_dropdownColor = { 0.220F,
                                                                    0.220F,
                                                                    0.220F,
                                                                    1.000F };

        static constexpr TypeAlias::Math::Color k_buttonColor = { 0.220F,
                                                                  0.220F,
                                                                  0.220F,
                                                                  1.000F };

        static constexpr TypeAlias::Math::Color k_buttonHoverColor = { 0.341F,
                                                                       0.341F,
                                                                       0.341F,
                                                                       1.000F };

        static constexpr TypeAlias::Math::Color k_buttonActiveColor = { 0.000F,
                                                                        0.314F,
                                                                        0.627F,
                                                                        1.000F };

        static constexpr TypeAlias::Math::Color k_tabColor = { 0.082F,
                                                               0.082F,
                                                               0.082F,
                                                               1.000F };

        static constexpr TypeAlias::Math::Color k_tabHoverColor = { 0.220F,
                                                                    0.220F,
                                                                    0.220F,
                                                                    1.000F };

        static constexpr TypeAlias::Math::Color k_tabActiveColor = { 0.141F,
                                                                     0.141F,
                                                                     0.141F,
                                                                     1.000F };

        static constexpr TypeAlias::Math::Color k_frameColor = { 0.059F,
                                                                 0.059F,
                                                                 0.059F,
                                                                 1.000F };

        static constexpr TypeAlias::Math::Color k_frameHoverColor = { 0.102F,
                                                                      0.102F,
                                                                      0.102F,
                                                                      1.000F };

        static constexpr TypeAlias::Math::Color k_frameActiveColor = { 0.059F,
                                                                       0.059F,
                                                                       0.059F,
                                                                       1.000F };

        static constexpr TypeAlias::Math::Color k_scrollbarGrabColor = { 0.282F,
                                                                         0.282F,
                                                                         0.282F,
                                                                         1.000F };

        static constexpr TypeAlias::Math::Color k_scrollbarGrabHoverColor = { 0.341F,
                                                                              0.341F,
                                                                              0.341F,
                                                                              1.000F };

        static constexpr TypeAlias::Math::Color k_scrollbarGrabActiveColor = { 0.502F,
                                                                               0.502F,
                                                                               0.502F,
                                                                               1.000F };

        static constexpr float k_editorGlobalScale = 1.0F;

        static constexpr float k_windowRounding    = 2.0F;
        static constexpr float k_childRounding     = 2.0F;
        static constexpr float k_frameRounding     = 2.0F;
        static constexpr float k_popupRounding     = 2.0F;
        static constexpr float k_scrollbarRounding = 2.0F;
        static constexpr float k_grabRounding      = 2.0F;
        static constexpr float k_tabRounding       = 2.0F;

        static constexpr float k_windowBorderSize = 1.0F;
        static constexpr float k_childBorderSize  = 1.0F;
        static constexpr float k_popupBorderSize  = 1.0F;
        static constexpr float k_frameBorderSize  = 0.0F;
        static constexpr float k_tabBorderSize    = 0.0F;

        static constexpr float k_windowPaddingX = 8.0F;
        static constexpr float k_windowPaddingY = 8.0F;

        static constexpr float k_framePaddingX = 6.0F;
        static constexpr float k_framePaddingY = 4.0F;

        static constexpr float k_itemSpacingX = 8.0F;
        static constexpr float k_itemSpacingY = 4.0F;

        static constexpr float k_indentSpacing = 16.0F;
        static constexpr float k_scrollbarSize = 13.0F;

        // UEエディタはやや小さめのフォントサイズ。
        static constexpr float k_editorFontSize = 12.0F;

        static constexpr int k_fontHorizontalOversample = 3;
        static constexpr int k_fontVerticalOversample   = 1;

        inline static const std::filesystem::path k_fontPath     = "Asset/Data/Font/Cica/Cica-Regular.ttf";
        inline static const std::filesystem::path k_iconFontPath = "Asset/Data/Font/Icon/FontAwesome/Font Awesome 7 Free-Solid-900.otf";
    };
}