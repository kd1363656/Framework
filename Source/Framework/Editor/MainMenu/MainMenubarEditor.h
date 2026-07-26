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

		const auto& GetREFEditorMainMenuSmartPointerVectorArray() const { return m_editorMainMenuSmartPointerVectorArray; }

	private:

		Converter::MainMenubarEditorJsonConverter m_jsonConverter = {};

		Utility::SmartPointerVectorArray<std::unique_ptr<EditorMainMenuBase>> m_editorMainMenuSmartPointerVectorArray = {};
	};
}