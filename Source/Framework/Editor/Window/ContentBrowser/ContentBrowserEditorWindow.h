#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 ContentBrowserEditorWindow()          = default;
		~ContentBrowserEditorWindow() override = default;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		void Draw() override;

		nlohmann::json Serialize() override;

		const auto& GetREFAssetRegistry() const { return m_assetRegistry; }

		auto& GetMutableREFAssetRegistry() { return m_assetRegistry; }

	private:

		void DrawFolderTree                    ();
		void DrawFolderTreeNode                (const std::filesystem::path& a_folderPath);
		void DrawCurrentFolder                 ();
		void DrawFolderEntry                   (const std::filesystem::path& a_entryPath, bool a_isFolder);
		void DrawGameObjectPrefabDragDropTarget(const std::filesystem::path& a_folderPath);
		void DrawFolderCreateEntry             ();
		void DrawCurrentFolderContextMenu      ();
		void DrawFolderEntryContextMenu        (const std::filesystem::path& a_folderPath);
		
		void RequestFolderCreate(const std::filesystem::path& a_parentFolderPath);

		void ConfirmFolderCreate();

		void CancelFolderCreate();

		void ClearFolderCreateState();

		void ApplyFolderCreateShortcut      ();
		void ApplyEntrySelectionShortcut    ();
		void ApplySelectedEntryDeleteRequest();
		void ApplyFolderCreateRequest       ();
		void ApplyCurrentFolderPath         (const std::filesystem::path& a_folderPath);
		void ApplyFolderDeleteRequest       ();

		std::filesystem::path FetchVALFolderCreateParentPath() const;

		std::string_view FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, bool a_isFolder) const;

		inline static const std::filesystem::path k_contentRootFolderPath = "Content";
		
		static constexpr std::string_view k_editorName = "コンテンツブラウザー";

		static constexpr std::string_view k_defaultNewFolderName          = "NewFolder";
		static constexpr std::string_view k_folderCreateInputeLabel       = "##ContentBrowserFolderCreateInput";
		static constexpr std::string_view k_currentFolderContextMenuLabel = "##ContentBrowserCurrentFolderContextMenu";
		static constexpr std::string_view k_folderEntryContextMenuLabel   = "##ContentBrowserCurrentFolderContextMenu";
		static constexpr std::string_view k_addFolderMenuItemName         = "フォルダーを追加";
		static constexpr std::string_view k_addFolderShortcutText         = "Ctrl + Shift + N";
		static constexpr std::string_view k_deleteFolderMenuItemName      = "フォルダーを削除";

		static constexpr std::string_view k_folderTreeChildLabel     = "##ContentBrowserFolderTree";
		static constexpr std::string_view k_currentFolderChildLabel  = "##ContentBrowserCurrentFolder";
		static constexpr std::string_view k_folderEntryButtonLabel   = "##ContentBrowserFolderEntry";
		static constexpr std::string_view k_folderEntryNameEllipsis  = "...";

		static constexpr float k_folderTreePanelWidth = 240.0F;
		static constexpr float k_filleRemainingSize   = 0.0F;

		static constexpr float k_folderEntryWidth  = 104.0F;
		static constexpr float k_folderEntryHeight = 92.0F;

		static constexpr float k_folderEntryIconFontSize = 48.0F;

		static constexpr float k_folderEntryIconTopPadding    = 8.0F;
		static constexpr float k_folderEntryTextBottomPadding = 7.0F;
		
		static constexpr float k_folderEntryRounding = 4.0F;
		static constexpr float k_centeringRatio      = 0.5F;

		static constexpr std::size_t k_folderEntryNameDisplayCharacterCount = 9ULL;
		static constexpr std::size_t k_folderEntryNameStartIndex            = 0ULL;

		static constexpr std::uint32_t k_minFolderEntryColumnCount     = 1U;
		static constexpr std::uint32_t k_initialFolderEntryColumnCount = 0U;
		
		std::filesystem::path m_currentFolderPath = k_contentRootFolderPath;
		
		ContentBrowserEditorWindowAssetRegistry m_assetRegistry = {};
		ContentBrowserEditorWindowFileSystem    m_fileSystem    = {};

		Converter::ContentBrowserEditorWindowJsonConverter m_jsonConverter = {};

		std::filesystem::path m_folderCreateParentPath    = {};
		std::filesystem::path m_requestedDeleteFolderPath = {};

		std::string m_folderCreateNameBuffer = {};

		bool m_isFolderCreateActive              = false;
		bool m_isFolderCreateInputFocusRequested = false;
		bool m_isFolderCreateRequested           = false;

		FWK_DEFINE_TYPE_INFO(ContentBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ContentBrowserEditorWindow)