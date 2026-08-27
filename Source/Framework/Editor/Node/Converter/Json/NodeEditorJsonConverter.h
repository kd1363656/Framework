#pragma once

namespace FWK::Editor
{
	class NodeEditor;
}

namespace FWK::Converter
{
	class NodeEditorJsonConverter final
	{
	public:

		 NodeEditorJsonConverter() = default;
		~NodeEditorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::NodeEditor& a_nodeEditor) const;
		
		nlohmann::json Serialize(const Editor::NodeEditor& a_nodeEditor) const;

	private:

		void DeserializeLinkDataList(const nlohmann::json& a_rootJson, Editor::NodeEditor& a_nodeEditor) const;

		nlohmann::json SerializeLinkDataList(const Editor::NodeEditor& a_nodeEditor) const;

		static constexpr std::string_view k_nodeEditorAllocatorJsonKey = "NodeEditorAllocator";
		static constexpr std::string_view k_linkDataListJsonKey        = "LinkDataList";
		static constexpr std::string_view k_linkIDJsonKey              = "LinkID";
		static constexpr std::string_view k_outputPinIDJsonKey         = "OutputPinID";
		static constexpr std::string_view k_inputPinIDJsonKey          = "InputPinID";
	};
}