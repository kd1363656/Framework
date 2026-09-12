#pragma once

namespace FWK::Editor
{
    class EditorStyle final
    {
    public:

         EditorStyle() = default;
        ~EditorStyle() = default;

        static void ApplySakuraDarkStyle();
        static void ApplyFont           ();

    private:

        static ImVec4 ConvertEditorColorToIMVEC4(const TypeAlias::Math::Color& a_color);

        inline static const std::filesystem::path k_fontPath     = "Asset/Data/Font/Cica/Cica-Regular.ttf";
        inline static const std::filesystem::path k_iconFontPath = "Asset/Data/Font/Icon/FontAwesome/Font Awesome 7 Free-Solid-900.otf";

        static constexpr ImWchar k_iconGlyphRanges[] = { 0xE000, 0xF8FF, 0 };

        static constexpr float k_editorGlobalScale = 1.0F;

        // Unreal Engine風のシャープな角丸。
        // UEエディタは角が立ったUIが基本だが、わずかに丸めることで柔らかさを出す。
        static constexpr float k_windowRounding    = 2.0F;
        static constexpr float k_childRounding     = 2.0F;
        static constexpr float k_frameRounding     = 2.0F;
        static constexpr float k_popupRounding     = 2.0F;
        static constexpr float k_scrollbarRounding = 2.0F;
        static constexpr float k_grabRounding      = 2.0F;
        static constexpr float k_tabRounding       = 2.0F;

        // UE風の控えめな枠線。
        // UEエディタは枠線が薄く、パネルの区別は色味で表現される。
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

        static constexpr int k_fontHorizontalOversample = 3;
        static constexpr int k_fontVerticalOversample   = 1;

        // UEエディタはやや小さめのフォントサイズ。
        static constexpr float k_editorFontSize = 12.0F;

        // UEエディタ風のダークグレー基調 + 青アクセント配色。
        // クリアカラー: ビューポートのクリア背景。UEはほぼ黒に近い濃グレー。
        static constexpr TypeAlias::Math::Color k_clearColor = { 0.04F,
                                                                 0.04F,
                                                                 0.04F,
                                                                 1.00F };

        // 通常ウィンドウ背景。UEエディタの基本背景色。
        static constexpr TypeAlias::Math::Color k_backgroundColor = { 0.09F,
                                                                      0.09F,
                                                                      0.09F,
                                                                      1.00F };

        // 子パネル背景。背景より一段明るいグレー。
        static constexpr TypeAlias::Math::Color k_panelColor = { 0.14F,
                                                                 0.14F,
                                                                 0.14F,
                                                                 1.00F };

        static constexpr TypeAlias::Math::Color k_panelHoverColor = { 0.20F,
                                                                      0.20F,
                                                                      0.20F,
                                                                      1.00F };

        static constexpr TypeAlias::Math::Color k_panelActiveColor = { 0.25F,
                                                                       0.25F,
                                                                       0.25F,
                                                                       1.00F };

        // 枠線色。UEは控えめで暗めのグレー。
        static constexpr TypeAlias::Math::Color k_borderColor = { 0.22F,
                                                                  0.22F,
                                                                  0.22F,
                                                                  0.60F };

        static constexpr TypeAlias::Math::Color k_borderShadowColor = { 0.00F,
                                                                        0.00F,
                                                                        0.00F,
                                                                        0.00F };

        // テキスト色。UEは明るいグレー。
        static constexpr TypeAlias::Math::Color k_textColor = { 0.88F,
                                                                0.88F,
                                                                0.88F,
                                                                1.00F };

        static constexpr TypeAlias::Math::Color k_textDisabledColor = { 0.45F,
                                                                        0.45F,
                                                                        0.45F,
                                                                        1.00F };

        // アクセントカラー: UEのシグネチャブルー。
        // 選択、チェック、スライダーつまみ、Dockingプレビューなどに使う。
        static constexpr TypeAlias::Math::Color k_accentColor = { 0.20F,
                                                                  0.52F,
                                                                  1.00F,
                                                                  1.00F };

        static constexpr TypeAlias::Math::Color k_accentHoverColor = { 0.30F,
                                                                       0.62F,
                                                                       1.00F,
                                                                       1.00F };

        static constexpr TypeAlias::Math::Color k_accentActiveColor = { 0.15F,
                                                                        0.45F,
                                                                        0.95F,
                                                                        1.00F };

        // ボタン: 基本はグレー、押下時は青みを帯びる。
        static constexpr TypeAlias::Math::Color k_buttonColor = { 0.16F,
                                                                  0.16F,
                                                                  0.16F,
                                                                  1.00F };

        static constexpr TypeAlias::Math::Color k_buttonHoverColor = { 0.22F,
                                                                       0.22F,
                                                                       0.22F,
                                                                       1.00F };

        static constexpr TypeAlias::Math::Color k_buttonActiveColor = { 0.10F,
                                                                        0.45F,
                                                                        0.95F,
                                                                        1.00F };

        // ヘッダー: TreeNode/Selectableの背景。
        // ホバー・選択時は青系でUEの選択ハイライトを再現。
        static constexpr TypeAlias::Math::Color k_headerColor = { 0.18F,
                                                                  0.18F,
                                                                  0.18F,
                                                                  1.00F };

        static constexpr TypeAlias::Math::Color k_headerHoverColor = { 0.12F,
                                                                       0.40F,
                                                                       0.85F,
                                                                       1.00F };

        static constexpr TypeAlias::Math::Color k_headerActiveColor = { 0.15F,
                                                                        0.50F,
                                                                        1.00F,
                                                                        1.00F };

        // タブ: 非アクティブは暗め、アクティブは一段明るいグレー。
        static constexpr TypeAlias::Math::Color k_tabColor = { 0.12F,
                                                               0.12F,
                                                               0.12F,
                                                               1.00F };

        static constexpr TypeAlias::Math::Color k_tabHoverColor = { 0.20F,
                                                                    0.20F,
                                                                    0.20F,
                                                                    1.00F };

        static constexpr TypeAlias::Math::Color k_tabActiveColor = { 0.18F,
                                                                     0.18F,
                                                                     0.18F,
                                                                     1.00F };

        // Frame: InputText/Combo/Checkbox背景。
        // アクティブ時は青系でフォーカス状態を表現。
        static constexpr TypeAlias::Math::Color k_frameColor = { 0.12F,
                                                                 0.12F,
                                                                 0.12F,
                                                                 1.00F };

        static constexpr TypeAlias::Math::Color k_frameHoverColor = { 0.18F,
                                                                      0.18F,
                                                                      0.18F,
                                                                      1.00F };

        static constexpr TypeAlias::Math::Color k_frameActiveColor = { 0.10F,
                                                                       0.45F,
                                                                       0.95F,
                                                                       1.00F };
    };
}