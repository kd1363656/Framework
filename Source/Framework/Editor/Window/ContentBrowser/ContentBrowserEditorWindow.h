#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 ContentBrowserEditorWindow()          = default;
		~ContentBrowserEditorWindow() override = default;

		void CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_directoryPath);
		void Deserialize               (const nlohmann::json&            a_rootJson) override;

		void Draw() override;

		nlohmann::json Serialize() override;

		void RefreshCurrentFolderEntries();

		void ApplyCurrentFolderPath(const std::filesystem::path& a_folderPath);

		void RequestSelectedEntryDelete();

		const auto& GetREFAssetRegistry() const { return m_assetRegistry; }
		const auto& GetREFFileSystem   () const { return m_fileSystem; }

		const auto& GetREFCurrentFolderPath() const { return m_currentFolderPath; }

		auto& GetMutableREFEntryController() { return m_entryController; }
		auto& GetMutableREFAssetRegistry  () { return m_assetRegistry; }

	private:

		void DrawFolderTree                    ();
		void DrawFolderTreeNode                (const std::filesystem::path& a_folderPath);
		void DrawCurrentFolder                 ();
		bool DrawFolderEntry                   (const Struct::ContentBrowserEntryData& a_entryData);
		void DrawGameObjectPrefabDragDropTarget(const std::filesystem::path&           a_folderPath);
		void DrawFolderCreateEntry             ();
		void DrawCurrentFolderContextMenu      ();
		void DrawFolderEntryContextMenu        (const Struct::ContentBrowserEntryData& a_entryData);
		
		void RequestFolderCreate(const std::filesystem::path& a_parentFolderPath);

		void ConfirmFolderCreate();

		void CancelFolderCreate();

		void ClearFolderCreateState();

		void ApplyFolderCreateShortcut      ();
		void ApplyEntrySelectionShortcut    ();
		void ApplySelectedEntryDeleteRequest();
		void ApplyFolderCreateRequest       ();
		void ApplyFolderDeleteRequest       ();

		std::filesystem::path FetchVALFolderCreateParentPath() const;

		std::string_view FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, bool a_isFolder) const;

		ContentBrowserEditorWindowAssetRegistry   m_assetRegistry   = {};
		ContentBrowserEditorWindowEntryController m_entryController = {};
		ContentBrowserEditorWindowFileSystem      m_fileSystem      = {};

		Converter::ContentBrowserEditorWindowJsonConverter m_jsonConverter = {};

		std::filesystem::path m_currentFolderPath         = Constant::k_contentRootFolderPath;
		std::filesystem::path m_requestedSelectEntryPath  = {};
		std::filesystem::path m_folderCreateParentPath    = {};
		std::filesystem::path m_requestedDeleteFolderPath = {};

		std::string m_folderCreateNameBuffer = {};

		bool m_isFolderCreateActive              = false;
		bool m_isFolderCreateInputFocusRequested = false;
		bool m_isFolderCreateRequested           = false;

		bool m_isSelectedEntryDeleteRequested = false;

		FWK_DEFINE_TYPE_INFO(ContentBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ContentBrowserEditorWindow)