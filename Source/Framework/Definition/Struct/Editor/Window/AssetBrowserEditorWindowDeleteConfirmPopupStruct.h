#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowDeleteConfirmState final
    {
        static constexpr bool k_initialIsActive        = false;
        static constexpr bool k_initialIsOpenRequested = true;

        std::vector<std::filesystem::path> m_filePathList = {};

        bool m_isActive        = k_initialIsActive;
        bool m_isOpenRequested = k_initialIsOpenRequested;
    };
}