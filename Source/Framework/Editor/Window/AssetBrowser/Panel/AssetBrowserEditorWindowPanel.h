#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
	class AssetBrowserEditorWindowPanel final
	{
	public:

		 AssetBrowserEditorWindowPanel() = default;
		~AssetBrowserEditorWindowPanel() = default;

		void DrawFolderTree   (AssetBrowserEditorWindow& a_assetBrowserEditorWindow);
		void DrawCurrentFolder(AssetBrowserEditorWindow& a_assetBrowserEditorWindow);

	private:

		void DrawFolderTreeNode                (const std::filesystem::path&         a_folderPath, AssetBrowserEditorWindow& a_assetBrowserEditorWindow);
		bool DrawFolderEntry                   (const Struct::AssetBrowserEntryData& a_entryData,  AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
		void DrawGameObjectPrefabDragDropTarget(const std::filesystem::path&         a_folderPath, AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
		void DrawCurrentFolderContextMenu      (      AssetBrowserEditorWindow&      a_assetBrowserEditorWindow);
		void DrawSceneDragDropTarget           (const std::filesystem::path&         a_folderPath, AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
		void DrawFolderEntryContextMenu        (const Struct::AssetBrowserEntryData& a_entryData,  AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
		void DrawFolderCreateEntry             (      AssetBrowserEditorWindow&      a_assetBrowserEditorWindow)                                         const;

		void ApplyEntrySelectionShortcut     (AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
		void ApplySelectedFolderOpenShortcut (AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
		void ApplySelectedEntryDeleteShortcut(AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;
		void ApplyFolderCreateShortcut       (AssetBrowserEditorWindow& a_assetBrowserEditorWindow) const;

		std::string_view FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, const bool a_isFolder) const;

		bool ContainsCurrentFolderPath(const std::filesystem::path& a_folderPath, const std::filesystem::path& a_currentFolderPath) const;

		static constexpr std::string_view k_folderTreeChildLabel          = "##AssetBrowserFolderTree";
		static constexpr std::string_view k_currentFolderChildLabel       = "##AssetBrowserCurrentFolder";
		static constexpr std::string_view k_folderEntryButtonLabel        = "##AssetBrowserFolderEntry";
		static constexpr std::string_view k_currentFolderContextMenuLabel = "##AssetBrowserCurrentFolderContextMenu";
		static constexpr std::string_view k_folderEntryContextMenuLabel   = "##AssetBrowserFolderEntryContextMenu";
		static constexpr std::string_view k_folderCreateInputLabel        = "##AssetBrowserFolderCreateInput";

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