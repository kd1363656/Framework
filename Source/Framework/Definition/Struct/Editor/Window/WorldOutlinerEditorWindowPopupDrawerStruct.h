#pragma once

namespace FWK::Struct
{
    struct WorldOutlinerEditorWindowRenameState final
    {
        std::weak_ptr<GameObject> m_targetGameObject = {};
 
        std::array<char, Constant::k_imguiInputTextBufferSize> m_inputBuffer = {};
 
        bool m_isSceneTarget = Constant::k_initialWorldOutlinerRenameIsSceneTarget;
        bool m_isActive      = Constant::k_initialWorldOutlinerRenameIsActive;
        bool m_isFocused     = Constant::k_initialWorldOutlinerRenameIsFocused;
    };
}