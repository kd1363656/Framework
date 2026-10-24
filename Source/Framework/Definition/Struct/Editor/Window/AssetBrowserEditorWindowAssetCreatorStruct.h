#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowAssetCreationResult final
    {
        std::filesystem::path m_createdFilePath = {};

        bool m_isSuccess = Constant::k_initialAssetBrowserAssetCreationSuccessValue;
    };
}