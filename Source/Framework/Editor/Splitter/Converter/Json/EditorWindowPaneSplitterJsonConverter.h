#pragma once

namespace FWK::Editor
{
	class EditorWindowPaneSplitter;
}

namespace FWK::Converter
{
	class EditorWindowPaneSplitterJsonConverter
	{
	public:

		 EditorWindowPaneSplitterJsonConverter() = default;
		~EditorWindowPaneSplitterJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::EditorWindowPaneSplitter& a_editorWindowPaneSplitter) const;

		nlohmann::json Serialize(const Editor::EditorWindowPaneSplitter& a_editorWindowPaneSplitter) const;

	private:

		static constexpr std::string_view k_primaryPaneSizeJsonKey = "PrimaryPaneSize";
	};
}