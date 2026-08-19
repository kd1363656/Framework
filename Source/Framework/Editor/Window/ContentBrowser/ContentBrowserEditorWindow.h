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

		void RequestFolderCreate(const std::filesystem::path& a_parentFolderPath);

		void RefreshCurrentFolderEntries();

		void ApplyCurrentFolderPath(const std::filesystem::path& a_folderPath);

		void RequestSelectedEntryDelete();

		void ConfirmFolderCreate();

		void CancelFolderCreate();

		void SetFolderCreateInputFocusRequested(const bool a_set) { m_isFolderCreateInputFocusRequested = a_set; }

		const auto& GetREFAssetRegistry  () const { return m_assetRegistry; }
		const auto& GetREFEntryController() const { return m_entryController; }
		const auto& GetREFFileSystem     () const { return m_fileSystem; }

		const auto& GetREFCurrentFolderPath() const { return m_currentFolderPath; }

		auto& GetMutableREFEntryController() { return m_entryController; }
		auto& GetMutableREFAssetRegistry  () { return m_assetRegistry; }
		
		auto& GetMutableREFolderCreateNameBuffer() { return m_folderCreateNameBuffer; }

		bool GetVALIsFolderCreateActive             () const { return m_isFolderCreateActive; }
		bool GetVALIsFolderCreateInputFocusRequested() const { return m_isFolderCreateInputFocusRequested; }

	private:

		void ClearFolderCreateState();

		void ApplySelectedEntryDeleteRequest();
		void ApplyFolderCreateRequest       ();
		
		static constexpr std::string_view k_editorName = "コンテンツブラウザー";

		ContentBrowserEditorWindowAssetRegistry   m_assetRegistry   = {};
		ContentBrowserEditorWindowEntryController m_entryController = {};
		ContentBrowserEditorWindowFileSystem      m_fileSystem      = {};
		ContentBrowserEditorWindowPanel           m_panel           = {};

		Converter::ContentBrowserEditorWindowJsonConverter m_jsonConverter = {};

		std::filesystem::path m_currentFolderPath         = Constant::k_contentRootFolderPath;
		std::filesystem::path m_requestedSelectEntryPath  = {};
		std::filesystem::path m_folderCreateParentPath    = {};

		std::string m_folderCreateNameBuffer = {};

		bool m_isFolderCreateActive              = false;
		bool m_isFolderCreateInputFocusRequested = false;
		bool m_isFolderCreateRequested           = false;

		bool m_isSelectedEntryDeleteRequested = false;

		FWK_DEFINE_TYPE_INFO(ContentBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ContentBrowserEditorWindow)