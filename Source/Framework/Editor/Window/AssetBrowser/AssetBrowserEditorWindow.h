#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 AssetBrowserEditorWindow()          = default;
		~AssetBrowserEditorWindow() override = default;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		void Draw() override;

		nlohmann::json Serialize() override;

	private:

		void DrawFolderPane() const;
		void DrawAssetPane () const;

		static constexpr std::string_view k_editorName = "アセットブラウザー";

		static constexpr std::string_view k_folderPaneChildLabel = "##AssetBrowserFolderPane";
		static constexpr std::string_view k_assetPaneChildLabel  = "##AssetBrowserAssetPane";

		static constexpr float k_folderPaneWidth = 240.0F;

		static constexpr float k_fileRemainingArea = 0.0F;

		Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)