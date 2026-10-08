#pragma once

namespace FWK::Constant
{
    inline constexpr ImVec4 k_imguiAccentColor            = { 0.20F,
                                                              0.50F,
                                                              1.00F,
                                                              1.00F };

    inline constexpr ImVec4 k_imguiAccentTranslucentColor = { k_imguiAccentColor.x,
                                                              k_imguiAccentColor.y,
                                                              k_imguiAccentColor.z,
                                                              0.50F };

    inline constexpr ImVec4 k_imguiItemColor= { 0.18F,
                                                        0.18F,
                                                        0.18F,
                                                        1.00F };

    inline constexpr ImVec4 k_imguiItemHoveredColor = { 0.30F,
                                                        0.30F,
                                                        0.30F,
                                                        1.00F };
    inline constexpr ImVec4 k_imguiItemActiveColor  = { 0.38F,
                                                        0.38F,
                                                        0.38F,
                                                        1.00F };

    inline constexpr ImVec4 k_imguiItemSelectedColor         = k_imguiAccentColor;
    inline constexpr ImVec4 k_imguiItemSelectedInactiveColor = k_imguiAccentTranslucentColor;
}