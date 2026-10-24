#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowRenameState final
    {
        std::filesystem::path m_targetFilePath = {};

        std::array<char, Constant::k_imguiInputTextBufferSize> m_inputBuffer = {};

        bool m_isActive  = Constant::k_initialAssetBrowserRenameIsActive;
        bool m_isFocused = Constant::k_initialAssetBrowserRenameIsFocused;
    };
}