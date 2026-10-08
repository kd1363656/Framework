#pragma once

namespace FWK::Editor
{
    class MainMenuBarEditor final
    {
    public:

         MainMenuBarEditor() = default;
        ~MainMenuBarEditor() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        void Draw() const;

        nlohmann::json Serialize() const;

        void AddEditorMainMenu(std::unique_ptr<EditorMainMenuBase>&& a_editorMainMenu);

        const auto& GetREFEditorMainMenuList() const { return m_editorMainMenuList; }

    private:

        std::vector<std::unique_ptr<EditorMainMenuBase>> m_editorMainMenuList = {};

        Converter::MainMenuBarEditorJsonConverter m_jsonConverter = {};
    };
}