#pragma once

namespace FWK::Constant
{
    // エディタ全体で使う配色(UnrealEngine5のダークテーマ風)
    // 色はすべてここに集めて、各EditorWindowはここの定数だけを使う
    // (ウィンドウごとに色を書くと、同じ「ホバー」でも色がばらばらになってしまうため)

    inline constexpr ImVec4 k_imguiTitleColor      = { 0.059F, 0.059F, 0.059F, 1.00F };
    inline constexpr ImVec4 k_imguiBackgroundColor = { 0.082F, 0.082F, 0.082F, 1.00F };
    inline constexpr ImVec4 k_imguiRecessedColor   = { 0.102F, 0.102F, 0.102F, 1.00F };
    inline constexpr ImVec4 k_imguiPanelColor      = { 0.141F, 0.141F, 0.141F, 1.00F };
    inline constexpr ImVec4 k_imguiHeaderColor     = { 0.184F, 0.184F, 0.184F, 1.00F };
    inline constexpr ImVec4 k_imguiDropdownColor   = { 0.220F, 0.220F, 0.220F, 1.00F };
    inline constexpr ImVec4 k_imguiInputColor      = { 0.059F, 0.059F, 0.059F, 1.00F };

    inline constexpr ImVec4 k_imguiBorderColor = { 0.220F, 0.220F, 0.220F, 1.00F };

    // ホバーと押下は、ボタン、入力欄、タブ、行などすべての部品で同じ色を使う
    inline constexpr ImVec4 k_imguiHoverColor  = { 0.341F, 0.341F, 0.341F, 1.00F };
    inline constexpr ImVec4 k_imguiHover2Color = { 0.502F, 0.502F, 0.502F, 1.00F };
    inline constexpr ImVec4 k_imguiPressColor  = { 0.102F, 0.102F, 0.102F, 1.00F };

    inline constexpr ImVec4 k_imguiForegroundColor         = { 0.753F, 0.753F, 0.753F, 1.00F };
    inline constexpr ImVec4 k_imguiForegroundHoverColor    = { 1.000F, 1.000F, 1.000F, 1.00F };
    inline constexpr ImVec4 k_imguiForegroundDisabledColor = { 0.502F, 0.502F, 0.502F, 1.00F };

    inline constexpr ImVec4 k_imguiAccentColor      = { 0.000F, 0.439F, 0.878F, 1.00F };
    inline constexpr ImVec4 k_imguiAccentHoverColor = { 0.055F, 0.525F, 1.000F, 1.00F };
    inline constexpr ImVec4 k_imguiAccentPressColor = { 0.000F, 0.314F, 0.627F, 1.00F };

    inline constexpr ImVec4 k_imguiAccentTranslucentColor = { k_imguiAccentColor.x,
                                                              k_imguiAccentColor.y,
                                                              k_imguiAccentColor.z,
                                                              0.50F };

    inline constexpr ImVec4 k_imguiAccentFaintColor = { k_imguiAccentColor.x,
                                                        k_imguiAccentColor.y,
                                                        k_imguiAccentColor.z,
                                                        0.20F };

    inline constexpr ImVec4 k_imguiDangerColor = { 1.000F, 0.251F, 0.251F, 1.00F };

    inline constexpr ImVec4 k_imguiTransparentColor = { 0.0F, 0.0F, 0.0F, 0.00F };

    inline constexpr ImVec4 k_imguiModalDimColor = { 0.0F, 0.0F, 0.0F, 0.60F };

    inline constexpr ImVec4 k_imguiItemColor        = k_imguiHeaderColor;
    inline constexpr ImVec4 k_imguiItemHoveredColor = k_imguiHoverColor;
    inline constexpr ImVec4 k_imguiItemActiveColor  = k_imguiPressColor;

    inline constexpr ImVec4 k_imguiItemSelectedColor         = k_imguiAccentColor;
    inline constexpr ImVec4 k_imguiItemSelectedHoveredColor  = k_imguiAccentHoverColor;
    inline constexpr ImVec4 k_imguiItemSelectedActiveColor   = k_imguiAccentPressColor;
    inline constexpr ImVec4 k_imguiItemSelectedInactiveColor = k_imguiAccentTranslucentColor;

    inline constexpr ImVec4 k_imguiItemCutColor = { k_imguiItemSelectedColor.x * 0.50F,
                                                    k_imguiItemSelectedColor.y * 0.50F,
                                                    k_imguiItemSelectedColor.z * 0.50F,
                                                    k_imguiItemSelectedColor.w * 0.50F };
}