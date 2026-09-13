#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowFolderPane final
    {
    public:

         AssetBrowserEditorWindowFolderPane() = default;
        ~AssetBrowserEditorWindowFolderPane() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        void Draw(const AssetBrowserEditorWindowPopupDrawer&         a_popupDrawer,
                  const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                  const std::filesystem::path&                       a_assetRootFolderPath,
                  const Enum::AssetBrowserActivePaneType             a_activePane,
                  const float                                        a_paneWidth,
                        AssetBrowserEditorWindowFileOperation&       a_fileOperation,
                        AssetBrowserEditorWindowClipboard&           a_clipboard, 
                        AssetFilePathRegistry&                       a_assetFilePathRegistry,
                        Struct::AssetBrowserEditorWindowRenameState& a_renameState);

        void ClearSelection();

        nlohmann::json Serialize() const;

        void SetCurrentFolderPath(const std::filesystem::path& a_set) { m_currentFolderPath = a_set; }

        const auto& GetREFSelectedFilePathList() const { return m_selectedFilePathList; }

        const auto& GetREfCurrentFolderPath() const { return m_currentFolderPath; }

    private:

        void DrawTreeNode(const AssetBrowserEditorWindowPopupDrawer&         a_popupDrawer,
                          const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                          const std::filesystem::path&                       a_folderPath,
                          const std::filesystem::path&                       a_assetFolderPath,
                          const Enum::AssetBrowserActivePaneType             a_activePane,
                          const float                                        a_paneWidth,
                                AssetBrowserEditorWindowFileOperation&       a_fileOperation,
                                AssetBrowserEditorWindowClipboard&           a_clipboard, 
                                AssetFilePathRegistry&                       a_assetFilePathRegistry,
                                Struct::AssetBrowserEditorWindowRenameState& a_renameState);

        bool IsFolderOpen(const std::filesystem::path& a_folderPath) const;

        void ToggleFolderOpen(const std::filesystem::path& a_folderPath);

        void SelectFolder(const std::filesystem::path& a_folderPath, const bool a_isRangeSelection = false, const bool a_isToggleSelection = false);

        static constexpr std::string_view k_childLabel      = "##AssetBrowserEditorWindowFolderPane";
        static constexpr std::string_view k_paneTitleLabel = "ファイル";

        std::unordered_map<std::filesystem::path, bool> m_folderOpenStateMap = {};

        std::vector<std::filesystem::path> m_selectedFilePathList = {};

        std::filesystem::path m_rangeSelectionStartPath = {};
        std::filesystem::path m_currentFolderPath       = {};
    };
}