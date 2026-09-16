#pragma once

namespace FWK::Constant
{
    inline constexpr std::string_view k_imguiFontAwesomeFolderOpenIcon  = "\xEF\x81\xBC";
    inline constexpr std::string_view k_imguiFontAwesomeFolderCloseIcon = "\xEF\x81\xBB";
    inline constexpr std::string_view k_imguiFontAwesomeBanIcon         = "\xEF\x81\x9E";

    inline constexpr std::string_view k_imguiAssetBrowserFolderDragAndDropPayloadLabel = "AssetBrowserFolder";

    inline constexpr ImVec4 k_imguiStrongBlueColor            = { 0.20F,                    0.50F,                    1.0F,                     1.0F };
    inline constexpr ImVec4 k_imguiStrongBlueTranslucentColor = { k_imguiStrongBlueColor.x, k_imguiStrongBlueColor.y, k_imguiStrongBlueColor.z, 0.50F };
    inline constexpr ImVec4 k_imguiDarkBlueTranslucentColor   = { 0.20F,                    0.20F,                    1.0F,                     0.50F };
    inline constexpr ImVec4 k_imguiLightGrayColor             = { 0.30F,                    0.30F,                    0.30F,                    1.0F };
    inline constexpr ImVec4 k_imguiDarkGrayColor              = { 0.18F,                    0.18F,                    0.18F,                    1.0F };
    inline constexpr ImVec4 k_imguiCutTargetTextColor         = { 1.0F,                     1.0F,                     1.0F,                     0.40F };
    inline constexpr ImVec4 k_imguiRedColor                   = { 1.0F,                     0.31F,                    0.31F,                    1.0F };

    inline constexpr float k_imguiInputTextHightPaddingAlignTreeNodeHight = 0.50F;
    inline constexpr float k_imguiImVec4ToImU32                           = 255.0F;

    inline constexpr float k_imguiDragDropUpperZoneRatio = 0.33F;
}