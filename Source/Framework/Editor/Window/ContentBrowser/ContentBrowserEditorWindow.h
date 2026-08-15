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

		void ApplyCurrentFolderPath(const std::filesystem::path& a_folderPath);

		std::string_view FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, bool a_isFolder) const;

		inline static const std::filesystem::path k_contentRootFolderPath = "Content";
		
		static constexpr std::string_view k_editorName               = "コンテンツブラウザー";
		static constexpr std::string_view k_folderTreeChildString    = "##ContentBrowserFolderTree";
		static constexpr std::string_view k_currentFolderChildString = "##ContentBrowserCurrentFolder";
		static constexpr std::string_view k_folderEntryButtonString  = "##ContentBrowserFolderEntry";
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

		static constexpr std::size_t k_folderEntryNameDisplayCharacterCount = 9U;

		static constexpr std::uint32_t k_minFolderEntryColumnCount     = 1U;
		static constexpr std::uint32_t k_initialFolderEntryColumnCount = 0U;
		
		std::filesystem::path m_currentFolderPath = k_contentRootFolderPath;
		std::filesystem::path m_selectedEntryPath = {};

		ContentBrowserEditorWindowAssetRegistry m_assetRegistry = {};
		ContentBrowserEditorWindowFileSystem    m_fileSystem    = {};

		Converter::ContentBrowserEditorWindowJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(ContentBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ContentBrowserEditorWindow)