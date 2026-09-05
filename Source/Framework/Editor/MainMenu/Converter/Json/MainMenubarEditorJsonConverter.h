#pragma once

namespace FWK::Editor
{
	class MainMenuBarEditor;
}

namespace FWK::Converter
{
	class MainMenuBarEditorJsonConverter
	{
	public:

		 MainMenuBarEditorJsonConverter() = default;
		~MainMenuBarEditorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Editor::MainMenuBarEditor& a_mainMenuBarEditor) const;

		nlohmann::json Serialize(const Editor::MainMenuBarEditor& a_mainMenubarEditor) const;

	private:

		void DeserializeEditorMainMenu(const nlohmann::json& a_rootJson, Editor::MainMenuBarEditor& a_mainMenuBarEditor) const;

		nlohmann::json SerializeEditorMainMenu(const Editor::MainMenuBarEditor& a_mainMenuBarEditor) const;

		static constexpr std::string_view k_mainMenuListJsonKey = "MainMenuList";
		static constexpr std::string_view k_mainMenuTypeName    = "MainMenuTypeName";
	};
}