#pragma once

namespace FWK::Constant
{
    inline constexpr std::string_view k_imguiFontAwesomeFolderOpenIcon  = "\xEF\x81\xBC";
    inline constexpr std::string_view k_imguiFontAwesomeFolderCloseIcon = "\xEF\x81\xBB";
    inline constexpr std::string_view k_imguiFontAwesomeBanIcon         = "\xEF\x81\x9E";

    inline constexpr std::string_view k_imguiAssetBrowserFolderDragAndDropPayloadLabel = "AssetBrowserFolder";

    inline constexpr ImVec4 k_imguiDangerColor = { 0.937F,
                                                   0.208F,
                                                   0.208F,
                                                   1.000F };

    inline constexpr float k_imguiInputTextHightPaddingAlignHight = 0.50F;
    inline constexpr float k_imguiImVec4ToImU32                   = 255.0F;

    inline constexpr float k_imguiDragDropUpperZoneRatio = 0.33F;
}