#pragma once

namespace FWK::Struct
{
    struct NodeEditorLinkData final
    {
        TypeAlias::NodeEditorID m_linkID      = Constant::k_imguiInvalidNodeEditorID;
        TypeAlias::NodeEditorID m_inputPinID  = Constant::k_imguiInvalidNodeEditorID;
        TypeAlias::NodeEditorID m_outputPinID = Constant::k_imguiInvalidNodeEditorID;
    };
}