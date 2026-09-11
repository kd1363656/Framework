#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 AssetBrowserEditorWindow()          = default;
		~AssetBrowserEditorWindow() override = default;

		void Deserialize    (const nlohmann::json& a_rootJson) override;
		void PostDeserialize()                                 override;

		void Draw() override;

		nlohmann::json Serialize() override;

		const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }
		const auto& GetREFPaneSplitter         () const { return m_paneSplitter; }

		auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }
		auto& GetMutableREFPaneSplitter         () { return m_paneSplitter; }

	private:

		static constexpr std::string_view k_paneSplitterLabel = "##AssetBrowserEditorWindowPaneSplitter";

		static constexpr std::string_view k_editorName                 = "アセットブラウザー";
		static constexpr std::string_view k_thisWindowExplanationLabel = "アセットブラウザーでは使用したいFBXファイルをモデル描画コンポーネントに\nドラッグ&ドロップしてロードするモデルとして扱ったり、使用したいゲームオブジェクトのプレハブをドラッグ&ドロップ\nでシーンに追加したりすることができるウィンドウ。";

		AssetFilePathRegistry m_assetFilePathRegistry = {};

		AssetBrowserEditorWindowDirectoryWatcher m_directoryWatcher = {};

		AssetBrowserEditorWindowFolderPane m_folderPane = {};
		AssetBrowserEditorWindowAssetPane  m_assetPane  = {};

		EditorWindowPaneSplitter m_paneSplitter = {};

		Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)