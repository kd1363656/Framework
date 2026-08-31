#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindow;
}

namespace FWK::Editor
{
	class ContentBrowserEditorWindowPanel final
	{
	public:

		 ContentBrowserEditorWindowPanel() = default;
		~ContentBrowserEditorWindowPanel() = default;

		void DrawFolderTree   (ContentBrowserEditorWindow& a_contentBrowserEditorWindow);
		void DrawCurrentFolder(ContentBrowserEditorWindow& a_contentBrowserEditorWindow);

	private:

		void DrawFolderTreeNode                (const std::filesystem::path&           a_folderPath, ContentBrowserEditorWindow& a_contentBrowserEditorWindow);
		bool DrawFolderEntry                   (const Struct::ContentBrowserEntryData& a_entryData,  ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;
		void DrawGameObjectPrefabDragDropTarget(const std::filesystem::path&           a_folderPath, ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;
		void DrawCurrentFolderContextMenu      (      ContentBrowserEditorWindow&      a_contentBrowserEditorWindow);
		void DrawSceneDragDropTarget           (const std::filesystem::path&           a_folderPath, ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;
		void DrawFolderEntryContextMenu        (const Struct::ContentBrowserEntryData& a_entryData,  ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;
		void DrawFolderCreateEntry             (      ContentBrowserEditorWindow&      a_contentBrowserEditorWindow)                                           const;

		void ApplyEntrySelectionShortcut     (ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;
		void ApplySelectedFolderOpenShortcut (ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;
		void ApplySelectedEntryDeleteShortcut(ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;
		void ApplyFolderCreateShortcut       (ContentBrowserEditorWindow& a_contentBrowserEditorWindow) const;

		std::string_view FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, const bool a_isFolder) const;

		bool ContainsCurrentFolderPath(const std::filesystem::path& a_folderPath, const std::filesystem::path& a_currentFolderPath) const;

		static constexpr std::string_view k_folderTreeChildLabel          = "##ContentBrowserFolderTree";
		static constexpr std::string_view k_currentFolderChildLabel       = "##ContentBrowserCurrentFolder";
		static constexpr std::string_view k_folderEntryButtonLabel        = "##ContentBrowserFolderEntry";
		static constexpr std::string_view k_currentFolderContextMenuLabel = "##ContentBrowserCurrentFolderContextMenu";
		static constexpr std::string_view k_folderEntryContextMenuLabel   = "##ContentBrowserFolderEntryContextMenu";
		static constexpr std::string_view k_folderCreateInputLabel        = "##ContentBrowserFolderCreateInput";

		static constexpr std::string_view k_addFolderMenuItemLabel   = "フォルダーを追加";
		static constexpr std::string_view k_deleteEntryMenuItemLabel = "削除";

		static constexpr std::string_view k_folderEntryNameEllipsis  = "...";
		static constexpr std::string_view k_folderCreateShortcutText = "Ctrl + Shift + N";

		static constexpr float k_folderTreePanelWidth = 240.0F;
		static constexpr float k_fillRemainingSize    = 0.0F;

		static constexpr float k_folderEntryWidth  = 104.0F;
		static constexpr float k_folderEntryHeight = 92.0F;

		static constexpr float k_folderEntryIconFontSize = 48.0F;

		static constexpr float k_folderEntryIconTopPadding    = 8.0F;
		static constexpr float k_folderEntryTextBottomPadding = 7.0F;
		
		static constexpr float k_folderEntryRounding = 4.0F;
		static constexpr float k_centeringRatio      = 0.5F;

		static constexpr std::size_t k_folderEntryNameDisplayCharacterCount = 9ULL;
		static constexpr std::size_t k_folderEntryNameStartIndex            = 0ULL;
		static constexpr std::size_t k_emptySelectionCount                  = 0ULL;

		static constexpr std::uint32_t k_minFolderEntryColumnCount     = 1U;
		static constexpr std::uint32_t k_initialFolderEntryColumnCount = 0U;

		std::filesystem::path m_synchronizedCurrentFolderPath = {};
	};
}