#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowFolderPane;
}

namespace FWK::Converter
{
    class AssetBrowserEditorWindowFolderPaneJsonConverter final
    {
    public:

         AssetBrowserEditorWindowFolderPaneJsonConverter() = default;
        ~AssetBrowserEditorWindowFolderPaneJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const;

        nlohmann::json Serialize(const Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const;

    private:

        void DeserializeOpenStateMap(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const;

        nlohmann::json SerializeOpenStateMap(const Editor::AssetBrowserEditorWindowFolderPane& a_assetBrowserEditorWindowFolderPane) const;

        static constexpr std::string_view k_currentFolderPathJsonKey       = "CurrentFolderPath";
        static constexpr std::string_view k_folderOpenStateMapJsonKey      = "FolderOpenStateMap";
        static constexpr std::string_view k_folderOpenStateFilePathJsonKey = "FilePath";
        static constexpr std::string_view k_folderOpenStateIsOpenJsonKey   = "IsOpen";

        static constexpr bool k_initialIsFolderOpen = false;
    };
}