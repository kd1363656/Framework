#pragma once

namespace FWK::Struct
{
    struct WorldOutlinerEditorWindowRenameState final
    {
        static constexpr bool k_initialIsActive      = false;
        static constexpr bool k_initialIsFocused     = false;
        static constexpr bool k_initialIsSceneTarget = false;

        std::array<char, Constant::k_imguiInputTextBufferSize> m_inputBuffer = {};

        std::weak_ptr<GameObject> m_targetGameObject = {};

        bool m_isSceneTarget = k_initialIsSceneTarget;
        bool m_isActive      = k_initialIsActive;
        bool m_isFocused     = k_initialIsFocused;
    };
}