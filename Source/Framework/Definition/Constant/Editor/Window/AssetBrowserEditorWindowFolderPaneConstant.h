#pragma once

namespace FWK::Constant
{
    inline constexpr std::string_view k_imguiFontAwesomeFolderOpenIcon  = "\xEF\x81\xBC";
    inline constexpr std::string_view k_imguiFontAwesomeFolderCloseIcon = "\xEF\x81\xBB";

    inline constexpr ImVec4 k_imguiStrongBlueColor = { 0.20F,                    0.50F,                    1.00F,                    1.0F };
    inline constexpr ImVec4 k_imguiLightGrayColor  = { 0.30F,                    0.30F,                    0.30F,                    1.0F };
    inline constexpr ImVec4 k_imguiWeakBlueColor   = { k_imguiStrongBlueColor.x, k_imguiStrongBlueColor.y, k_imguiStrongBlueColor.z, 0.50F };
    
    inline constexpr float k_imguiInputTextHightPaddingAlignTreeNodeHight = 0.50F;
}