#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow final : public EditorWindowBase
	{
	private:

		struct FolderTreeData final
		{
			std::filesystem::path m_folderPath = {};

			std::vector<FolderTreeData> m_childFolderDataList = {};
		};

	public:

		 AssetBrowserEditorWindow()          = default;
		~AssetBrowserEditorWindow() override = default;

		void Deserialize    (const nlohmann::json& a_rootJson) override;
		void PostDeserialize()                                 override;

		void Draw() override;

		nlohmann::json Serialize() override;

		const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

		auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }

	private:

		void RefreshFolderTree();

		void DrawFolderPane();
		void DrawFolderTree(const FolderTreeData& a_folderTreeData, bool a_isRootFolder);
		void DrawAssetPane () const;

		static constexpr std::string_view k_editorName = "アセットブラウザー";

		static constexpr std::string_view k_folderPaneChildLabel = "##AssetBrowserFolderPane";
		static constexpr std::string_view k_assetPaneChildLabel  = "##AssetBrowserAssetPane";

		static constexpr float k_folderPaneWidth = 240.0F;

		static constexpr float k_fileRemainingArea = 0.0F;

		AssetFilePathRegistry m_assetFilePathRegistry = {};

		Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

		FolderTreeData m_rootFolderTreeData = {};

		std::filesystem::path m_currentDirectoryPath = Constant::k_assetRootFolderPath;

		FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)