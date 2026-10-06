#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowRenameState final
    {
        static constexpr bool k_initialIsActive  = false;
        static constexpr bool k_initialIsFocused = false;

        std::array<char, Constant::k_imguiInputTextBufferSize> m_inputBuffer = {};

        std::filesystem::path m_targetFilePath = {};

        bool m_isActive  = k_initialIsActive;
        bool m_isFocused = k_initialIsFocused;
    };
}