#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowDeleteConfirmState final
    {
        std::vector<std::filesystem::path> m_filePathList = {};

        Editor::EditorPopupNavigator m_popupNavigator = {};

        bool m_isActive        = Constant::k_initialAssetBrowserDeleteConfirmIsActive;
        bool m_isOpenRequested = Constant::k_initialAssetBrowserDeleteConfirmIsOpenRequested;
    };
}