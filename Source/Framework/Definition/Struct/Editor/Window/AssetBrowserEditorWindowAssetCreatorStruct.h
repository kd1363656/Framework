#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowAssetCreationResult final
    {
        static constexpr bool k_initialSuccessValue = false;

        std::filesystem::path m_createdFilePath = {};

        bool m_isSuccess = k_initialSuccessValue;
    };
}