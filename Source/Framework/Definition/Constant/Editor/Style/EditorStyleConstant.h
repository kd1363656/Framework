#pragma once

namespace FWK::Constant
{
    inline constexpr ImVec4 k_imguiAccentColor = { 0.000F,
                                                   0.439F,
                                                   0.878F,
                                                   1.000F };

    inline constexpr ImVec4 k_imguiItemColor = { 0.184F,
                                                 0.184F,
                                                 0.184F,
                                                 1.000F };

    inline constexpr ImVec4 k_imguiItemHoveredColor       = { 0.220F,
                                                              0.220F,
                                                              0.220F,
                                                              1.000F };
    inline constexpr ImVec4 k_imguiRaisedItemHoveredColor = { 0.341F,
                                                              0.341F,
                                                              0.341F,
                                                              1.000F };
    inline constexpr ImVec4 k_imguiItemActiveColor        = { 0.000F,
                                                              0.314F,
                                                              0.627F,
                                                              1.000F };

    inline constexpr ImVec4 k_imguiItemSelectedColor         = k_imguiAccentColor;
    inline constexpr ImVec4 k_imguiItemSelectedInactiveColor = { 0.251F,
                                                                 0.341F,
                                                                 0.435F,
                                                                 1.000F };
    inline constexpr ImVec4 k_imguiItemSelectedParentColor   = { 0.173F,
                                                                 0.192F,
                                                                 0.231F,
                                                                 1.000F };

    inline constexpr ImVec4 k_imguiRowStripeColor = { 0.082F,
                                                      0.082F,
                                                      0.082F,
                                                      1.000F };

    inline constexpr float k_imguiItemCutColorRate = 0.5F;

    inline constexpr ImVec4 k_imguiItemCutColor = { k_imguiAccentColor.x * k_imguiItemCutColorRate,
                                                    k_imguiAccentColor.y * k_imguiItemCutColorRate,
                                                    k_imguiAccentColor.z * k_imguiItemCutColorRate,
                                                    k_imguiAccentColor.w * k_imguiItemCutColorRate };
}