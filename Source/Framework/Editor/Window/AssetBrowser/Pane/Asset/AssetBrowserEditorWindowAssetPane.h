#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
    class AssetBrowserEditorWindowAssetPane final
    {
    public:

         AssetBrowserEditorWindowAssetPane() = default;
        ~AssetBrowserEditorWindowAssetPane() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        void Draw(AssetBrowserEditorWindow& a_editorWindow);

        nlohmann::json Serialize() const;

        void MoveSelectionUp   (AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection = false);
        void MoveSelectionDown (AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection = false);
        void MoveSelectionLeft (AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection = false);
        void MoveSelectionRight(AssetBrowserEditorWindow& a_editorWindow, const bool a_isRangeSelection = false);

        void NavigateToCurrentCursor(AssetBrowserEditorWindow& a_editorWindow);

        std::vector<std::filesystem::path> FetchVALDisplayedFilePathList(AssetBrowserEditorWindow& a_editorWindow);

        std::filesystem::path FetchVALOperationTargetFilePath() const;

        const auto& GetREFSelectionState() const { return m_selectionState; }

        auto& GetMutableREFSelectionState() { return m_selectionState; }

    private:

        void DrawCard(const std::filesystem::path& a_filePath, AssetBrowserEditorWindow& a_editorWindow);

        void BuildDisplayedFilePathList(AssetBrowserEditorWindow& a_editorWindow, std::vector<std::filesystem::path>& a_displayedList);

        void SelectFile(const std::vector<std::filesystem::path>& a_displayedFilePathList,
                        const std::filesystem::path&              a_filePath,
                              AssetBrowserEditorWindow&           a_editorWindow,
                        const bool                                a_isRangeSelection  = false,
                        const bool                                a_isToggleSelection = false);

        void NavigateToFolder(const std::filesystem::path& a_folderPath, AssetBrowserEditorWindow& a_editorWindow);


        std::string TruncateText(const std::string& a_text, const float a_maxWidth) const;

        int CalculateCardPerRow(const float a_availableWidth) const;

        std::string FetchIcon(const std::filesystem::path& a_filePath, AssetBrowserEditorWindow& a_editorWindow) const;

        static constexpr std::string_view k_childLabel                 = "##AssetBrowserEditorWindowAssetPane";
        static constexpr std::string_view k_paneTitleLabel             = "アセット";
        static constexpr std::string_view k_emptySpaceContextMenuLabel = "##AssetPaneEmptyContextMenu";
        static constexpr std::string_view k_cardContextMenuPrefixLabel = "##AssetPaneCardContextMenu";
        static constexpr std::string_view k_cardPrefixLabel            = "##AssetPaneCardContextMenu";
        static constexpr std::string_view k_renameInputTextLabel       = "##AssetPaneRenameInputText";
        static constexpr std::string_view k_ellipsis                   = "...";

        static constexpr float k_cardWidth    = 60.0F;
        static constexpr float k_cardHeight   = 80.0F;
        static constexpr float k_cardSpacing  = 8.0F;
        static constexpr float k_cardRounding = 6.0F;
        static constexpr float k_cardPadding  = 4.0F;
        static constexpr float k_iconRatio    = 0.5F;

        static constexpr float k_selectionBorderThickness = 2.0F;
        static constexpr float k_hoverBorderThickness     = 1.5F;

        static constexpr int k_keyboardFocusNextItem = 0;
        static constexpr int k_initialCardColumn     = 0;

        AssetBrowserEditorWindowSelectionState     m_selectionState = {};
        AssetBrowserEditorWindowAssetPaneBreadcrumb m_breadcrumb    = {};

        Converter::AssetBrowserEditorWindowAssetPaneJsonConverter m_jsonConverter = {};

        std::filesystem::path m_currentCursorFilePath = {};
    };
}