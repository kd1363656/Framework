#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 AssetBrowserEditorWindow()          = default;
		~AssetBrowserEditorWindow() override = default;

		void CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_directoryPath);
		void CreateSceneFromScene      (const std::weak_ptr<Scene>&      a_scene,      const std::filesystem::path& a_directoryPath);
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
      
		const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

		const auto& GetREFEntryController      () const { return m_entryController; }
		const auto& GetREFFileSystem           () const { return m_fileSystem; } 
		const auto& GetREFPrefabInstanceCreator() const { return m_prefabInstanceCreator; }

		const auto& GetREFCurrentFolderPath() const { return m_currentFolderPath; }

		auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }
		auto& GetMutableREFEntryController      () { return m_entryController; }
		
		auto& GetMutableREFolderCreateNameBuffer() { return m_folderCreateNameBuffer; }

		bool GetVALIsFolderCreateActive             () const { return m_isFolderCreateActive; }
		bool GetVALIsFolderCreateInputFocusRequested() const { return m_isFolderCreateInputFocusRequested; }

	private:

		void ClearFolderCreateState();

		void ApplySelectedEntryDeleteRequest();
		void ApplyFolderCreateRequest       ();
		
		static constexpr std::string_view k_editorName = "アセットブラウザー";

		AssetFilePathRegistry                         m_assetFilePathRegistry = {};
		AssetBrowserEditorWindowEntryController       m_entryController       = {};
		AssetBrowserEditorWindowFileSystem            m_fileSystem            = {};
		AssetBrowserEditorWindowPrefabInstanceCreator m_prefabInstanceCreator = {};
		AssetBrowserEditorWindowPanel                 m_panel                 = {};

		Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

		std::filesystem::path m_currentFolderPath         = Constant::k_assetRootFolderPath;
		std::filesystem::path m_requestedSelectEntryPath  = {};
		std::filesystem::path m_folderCreateParentPath    = {};

		std::string m_folderCreateNameBuffer = {};

		bool m_isFolderCreateActive              = false;
		bool m_isFolderCreateInputFocusRequested = false;
		bool m_isFolderCreateRequested           = false;

		bool m_isSelectedEntryDeleteRequested = false;

		FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)