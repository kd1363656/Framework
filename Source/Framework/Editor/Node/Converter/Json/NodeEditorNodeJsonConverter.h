#pragma once

namespace FWK::Editor
{
	class NodeEditorNode;
}

namespace FWK::Converter
{
	class NodeEditorNodeJsonConverter final
	{
	public:

		 NodeEditorNodeJsonConverter() = default;
		~NodeEditorNodeJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::NodeEditorNode& a_nodeEditorNode) const;
		
		nlohmann::json Serialize(const Editor::NodeEditorNode& a_nodeEditorNode) const;

	private:

		void DeserializeInputPinIDList (const nlohmann::json& a_rootJson, Editor::NodeEditorNode& a_nodeEditorNode) const;
		void DeserializeOutputPinIDList(const nlohmann::json& a_rootJson, Editor::NodeEditorNode& a_nodeEditorNode) const;

		nlohmann::json SerializePinIDList(const std::vector<TypeAlias::NodeEditorID>& a_pidIDList) const;

		static constexpr std::string_view k_inputPinListJsonKey  = "InputPinIDList";
		static constexpr std::string_view k_outputPinListJsonKey = "OutputPinIDList";
		static constexpr std::string_view k_pinIDJsonKey         = "PinID";
		static constexpr std::string_view k_nodeIDJsonKey        = "NodeID";
		static constexpr std::string_view k_nodePositionJsonKey  = "NodePosition";
	};
}