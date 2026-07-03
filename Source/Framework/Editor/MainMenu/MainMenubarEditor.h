#pragma once

namespace FWK::Editor
{
	class MainMenubarEditor
	{
	public:

		 MainMenubarEditor() = default;
		~MainMenubarEditor() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		void Draw() const;

		nlohmann::json Serialize() const;

		void AddEditorMainMenu(std::unique_ptr<EditorMainMenuBase>&& a_editorMainMenu);

		const auto& GetREFEditorMainMenuList() const { return m_editorMainMenuList; }

	private:

		Converter::MainMenubarEditorJsonConverter m_jsonConverter = {};

		Utility::VectorArray<std::unique_ptr<EditorMainMenuBase>> m_editorMainMenuList = {};
	};
}