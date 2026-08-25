#pragma once

namespace FWK::Struct
{
	struct NodeEditorPinData final
	{
		TypeAlias::NodeEditorID m_nodeID = {};

		ImVec2 m_position = {};

		std::vector<NodeEditorPinData> m_pinList = {};
	};

	struct NodeEditorNodeData final
	{
		TypeAlias::NodeEditorID m_nodeID = {};

		ImVec2 m_position = {};

		std::vector<NodeEditorPinData> m_pinList = {};
	};

	struct NodeEditorLinkData final
	{
		TypeAlias::NodeEditorID m_linkID           = {};
		TypeAlias::NodeEditorID m_sourcePinID      = {};
		TypeAlias::NodeEditorID m_destinationPinID = {};
	};
}