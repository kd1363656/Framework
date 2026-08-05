#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 ContentBrowserEditorWindow()          = default;
		~ContentBrowserEditorWindow() override = default;

		void Draw() override;

	private:

		void CreatePrefab(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_prefabFilePath);

		std::string CreateVALPrefabName(const GameObject& a_gameObject) const;

		std::filesystem::path CreateVALPrefabFilePath(const std::filesystem::path& a_directoryPath, const std::string& a_prefabName) const;

		void RegisterPrefabAsset(const std::string& a_prefabName, const Struct::PrefabData& a_prefabData);

		void SetupPrefabData(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_prefabFilePath, Struct::PrefabData& a_outPrefabData) const;
		
		void DrawDirectoryTree             ();
		void DrawCurrentDirectoryHeader    ();
		void DrawCurrentDirectoryDropTarget();

		void ReceiveDroppedGameObject(const std::filesystem::path& a_dropDirectoryPath);

		bool CanCreatePrefabAsset(const GameObject& a_gameObject, const std::filesystem::path& a_directoryPath) const;

		static constexpr std::string_view k_editorName = "コンテンツブラウザー";
		
		FWK_DEFINE_TYPE_INFO(ContentBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ContentBrowserEditorWindow)