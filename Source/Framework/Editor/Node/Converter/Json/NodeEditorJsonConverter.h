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

		static constexpr std::string_view k_nodeEditorAllocatorJsonKey = "NodeEditorAllocator";
	};
}