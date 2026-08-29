#pragma once

namespace FWK::Struct
{
	struct NodeEditorLinkData final
	{
		TypeAlias::NodeEditorID m_linkID      = Constant::k_invalidNodeEditorID;
		TypeAlias::NodeEditorID m_inputPinID  = Constant::k_invalidNodeEditorID;
		TypeAlias::NodeEditorID m_outputPinID = Constant::k_invalidNodeEditorID;
	};
}