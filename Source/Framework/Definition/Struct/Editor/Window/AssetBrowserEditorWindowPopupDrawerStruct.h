#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowRenameState final
    {
        std::filesystem::path m_targetFilePath = {};

        std::array<char, Constant::k_assetBrowserRenameInputTextBufferSize> m_inputBuffer = {};

        bool m_isActive  = false;
        bool m_isFocused = false;
    };
}