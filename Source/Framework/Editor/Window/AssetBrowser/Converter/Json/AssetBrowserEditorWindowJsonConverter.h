#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Converter
{
    class AssetBrowserEditorWindowJsonConverter final
    {
    public:

         AssetBrowserEditorWindowJsonConverter() = default;
        ~AssetBrowserEditorWindowJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;

        nlohmann::json Serialize(const Editor::AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;

    private:

        static constexpr std::string_view k_editorWindowPaneSplitterJsonKey = "EditorWindowPaneSplitter";
        static constexpr std::string_view k_folderPaneJsonKey               = "FolderPane";
        static constexpr std::string_view k_assetPaneJsonKey                = "AssetPane";
        static constexpr std::string_view k_currentSelectFolderPathJsonKey  = "CurrentSelectFolderPath";
    };
}