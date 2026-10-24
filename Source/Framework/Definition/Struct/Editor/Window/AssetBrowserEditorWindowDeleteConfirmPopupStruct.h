#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowDeleteConfirmState final
    {
        std::vector<std::filesystem::path> m_filePathList = {};

        Enum::AssetBrowserDeleteConfirmSelectedButton m_keySelectedButton   = Enum::AssetBrowserDeleteConfirmSelectedButton::Delete;
        Enum::AssetBrowserDeleteConfirmSelectedButton m_mouseHoveredButton  = Enum::AssetBrowserDeleteConfirmSelectedButton::Delete;

        bool m_isActive        = Constant::k_initialAssetBrowserDeleteConfirmIsActive;
        bool m_isOpenRequested = Constant::k_initialAssetBrowserDeleteConfirmIsOpenRequested;
    };
}