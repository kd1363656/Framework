#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
    class AssetBrowserEditorWindowFolderPane final
    {
    public:

         AssetBrowserEditorWindowFolderPane() = default;
        ~AssetBrowserEditorWindowFolderPane() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        void Draw(AssetBrowserEditorWindow& a_editorWindow);

        void MoveSelectionUp    (AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection = false);
        void MoveSelectionDown  (AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection = false);
        void ForciblyFolderOpen (AssetBrowserEditorWindow& a_editorWindow);
        void ForciblyFolderClose(AssetBrowserEditorWindow& a_editorWindow);

        void ToggleCurrentFolderOpen(AssetBrowserEditorWindow& a_editorWindow);

        nlohmann::json Serialize() const;

        void ApplyFolderOpenState(const std::filesystem::path& a_folderPath, const bool a_isOpen);

        void AddFolderOpenState(const std::filesystem::path& a_folderPath, const bool a_isOpen);

        std::vector<std::filesystem::path> FetchVALDisplayedFolderList(AssetBrowserEditorWindow& a_editorWindow);

        std::filesystem::path FetchVALOperationTargetFolderPath() const;

        const auto& GetREFFolderOpenStateMap() const { return m_folderOpenStateMap; }

        const auto& GetREFSelectionState() const { return m_selectionState; }

        auto& GetMutableREFSelectionState() { return m_selectionState; }

    private:

        void DrawTreeNode(const std::filesystem::path& a_currentFolderPath, AssetBrowserEditorWindow& a_editorWindow);

        void BuildDisplayedFolderList(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap, const std::filesystem::path& a_folderPath, std::vector<std::filesystem::path>& a_displayedList);

        void SelectFolder(const std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>>& a_folderHierarchyMap, 
                          const std::filesystem::path&                                                         a_folderPath, 
                                AssetBrowserEditorWindow&                                                      a_editorWindow,
                          const bool                                                                           a_isRangeSelection  = false, 
                          const bool                                                                           a_isToggleSelection = false);

        bool IsFolderOpen(const std::filesystem::path& a_folderPath) const;

        void ToggleFolderOpen(const std::filesystem::path& a_folderPath);

        static constexpr std::string_view k_childLabel                     = "##AssetBrowserEditorWindowFolderPane";
        static constexpr std::string_view k_paneTitleLabel                 = "ファイル";
        static constexpr std::string_view k_contextMenuOpenPopupLabel      = "##FolderContextMnuOpenPopup";
        static constexpr std::string_view k_emptySpaceContextMenuOpenLabel = "##FolderPaneEmptyContextMenu";
        static constexpr std::string_view k_renameInputTextLabel           = "##RenameInputText";

        static constexpr int k_treeNodePopStyleColorPaneActiveNUM        = 3;
        static constexpr int k_treeNodePopStyleColorPaneInactiveNUM      = 1;
        static constexpr int k_initialTreeNodePopStyleColorPaneActiveNUM = 0;
        static constexpr int k_keyboardFocusNextItem                     = 0;

        std::unordered_map<std::filesystem::path, bool> m_folderOpenStateMap = {};

        AssetBrowserEditorWindowSelectionState m_selectionState = {};

        Converter::AssetBrowserEditorWindowFolderPaneJsonConverter m_jsonConverter = {};
    };
}