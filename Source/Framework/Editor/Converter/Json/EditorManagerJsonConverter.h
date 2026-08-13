#pragma once

namespace FWK::Editor
{
	class EditorManager;
}

namespace FWK::Converter
{
	class EditorManagerJsonConverter final
	{
	public:

		 EditorManagerJsonConverter() = default;
		~EditorManagerJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::EditorManager& a_editorManager) const;

		nlohmann::json Serialize(const Editor::EditorManager& a_editorManager) const;

	private:

		void DeserializeWindow(const nlohmann::json& a_rootJson, Editor::EditorManager& a_editorManager) const;

		nlohmann::json SerializeWindow(const Editor::EditorManager& a_editorManager) const;

		static constexpr std::string_view k_windowListJsonKey = "WindowList";
		static constexpr std::string_view k_windowJsonKey     = "Window";
		static constexpr std::string_view k_windowTypeJsonKey = "WindowType";

		static constexpr std::string_view k_mainMenubarJsonKey = "MainMenubar";

		static constexpr std::string_view k_isDisableDrawJsonKey = "IsDisableEditor";
	};
}