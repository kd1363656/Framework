#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowRenameState final
    {
        static constexpr bool k_initialIsActive  = false;
        static constexpr bool k_initialIsFocused = false;

        std::filesystem::path m_targetFilePath = {};

        std::array<char, Constant::k_assetBrowserRenameInputTextBufferSize> m_inputBuffer = {};

        bool m_isActive  = k_initialIsActive;
        bool m_isFocused = k_initialIsFocused;
    };
}