#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowDeleteConfirmState final
    {
        static constexpr bool k_initialIsActive        = false;
        static constexpr bool k_initialIsOpenRequested = true;

        std::vector<std::filesystem::path> m_filePathList = {};

        Enum::AssetBrowserDeleteConfirmSelectedButton m_keySelectedButton   = Enum::AssetBrowserDeleteConfirmSelectedButton::Delete;
        Enum::AssetBrowserDeleteConfirmSelectedButton m_mouseHoveredButton  = Enum::AssetBrowserDeleteConfirmSelectedButton::Delete;

        bool m_isActive        = k_initialIsActive;
        bool m_isOpenRequested = k_initialIsOpenRequested;
    };
}