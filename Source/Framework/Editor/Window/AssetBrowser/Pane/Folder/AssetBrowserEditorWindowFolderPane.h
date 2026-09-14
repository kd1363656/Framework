#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowFolderPane final
    {
    public:

         AssetBrowserEditorWindowFolderPane() = default;
        ~AssetBrowserEditorWindowFolderPane() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        void Draw(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap,
                  const AssetBrowserEditorWindowPopupDrawer&                                           a_popupDrawer,
                  const AssetBrowserEditorWindowAssetCreator&                                          a_assetCreator,
                  const std::filesystem::path&                                                         a_assetRootFolderPath,
                  const Enum::AssetBrowserActivePaneType                                               a_activePane,
                  const float                                                                          a_paneWidth,
                        AssetBrowserEditorWindowFileOperation&                                         a_fileOperation,
                        AssetBrowserEditorWindowClipboard&                                             a_clipboard, 
                        AssetFilePathRegistry&                                                         a_assetFilePathRegistry,
                        Struct::AssetBrowserEditorWindowRenameState&                                   a_renameState);

        void ClearSelection();

        nlohmann::json Serialize() const;

        void AddFolderOpenState(const std::filesystem::path& a_folderPath, const bool a_isOpen);

        void SetCurrentFolderPath(const std::filesystem::path& a_set) { m_currentFolderPath = a_set; }

        const auto& GetREFFolderOpenStateMap() const { return m_folderOpenStateMap; }

        const auto& GetREFSelectedFilePathList() const { return m_selectedFilePathList; }

        const auto& GetREfCurrentFolderPath() const { return m_currentFolderPath; }

    private:

        void DrawTreeNode(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap,
                          const AssetBrowserEditorWindowPopupDrawer&                                           a_popupDrawer,
                          const AssetBrowserEditorWindowAssetCreator&                                          a_assetCreator,
                          const std::filesystem::path&                                                         a_currentFolderPath,
                          const std::filesystem::path&                                                         a_assetRootFolderPath,
                          const Enum::AssetBrowserActivePaneType                                               a_activePane,
                          const float                                                                          a_paneWidth,
                                AssetBrowserEditorWindowFileOperation&                                         a_fileOperation,
                                AssetBrowserEditorWindowClipboard&                                             a_clipboard, 
                                AssetFilePathRegistry&                                                         a_assetFilePathRegistry,
                                Struct::AssetBrowserEditorWindowRenameState&                                   a_renameState);

        bool IsFolderOpen(const std::filesystem::path& a_folderPath) const;

        void ToggleFolderOpen(const std::filesystem::path& a_folderPath);

        void SelectFolder(const std::filesystem::path& a_folderPath, const bool a_isRangeSelection = false, const bool a_isToggleSelection = false);

        static constexpr std::string_view k_childLabel                     = "##AssetBrowserEditorWindowFolderPane";
        static constexpr std::string_view k_paneTitleLabel                 = "ファイル";
        static constexpr std::string_view k_contextMenuOpenPopupLabel      = "##FolderContextMnuOpenPopup";
        static constexpr std::string_view k_emptySpaceContextMenuOpenLabel = "##FolderPaneEmptyContextMenu";

        static constexpr int k_treeNodePopStyleColorNUM = 3;

        std::unordered_map<std::filesystem::path, bool> m_folderOpenStateMap = {};

        std::vector<std::filesystem::path> m_selectedFilePathList = {};

        Converter::AssetBrowserEditorWindowFolderPaneJsonConverter m_jsonConverter = {};

        std::filesystem::path m_rangeSelectionStartPath = {};
        std::filesystem::path m_currentFolderPath       = {};
    };
}