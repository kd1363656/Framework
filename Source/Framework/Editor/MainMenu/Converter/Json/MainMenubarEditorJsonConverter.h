#pragma once

namespace FWK::Editor
{
	class MainMenubarEditor;
}

namespace FWK::Converter
{
	class MainMenubarEditorJsonConverter
	{
	public:

		 MainMenubarEditorJsonConverter() = default;
		~MainMenubarEditorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::MainMenubarEditor& a_mainMenubarEditor) const;

		nlohmann::json Serialize(const Editor::MainMenubarEditor& a_mainMenubarEditor) const;

	private:

		void DeserializeEditorMainMenu(const nlohmann::json& a_rootJson, Editor::MainMenubarEditor& a_mainMenubarEditor) const;

		nlohmann::json SerializeEditormainMenu(const Editor::MainMenubarEditor& a_mainMenubarEditor) const;

		static constexpr std::string_view k_mainMenuListJsonKey = "MainMenuList";
		static constexpr std::string_view k_mainMenuTypeName    = "MainMenuTypeName";
	};
}