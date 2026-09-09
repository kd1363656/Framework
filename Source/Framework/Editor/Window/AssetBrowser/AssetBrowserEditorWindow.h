#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow final : public EditorWindowBase
	{
	private:

		struct AssetEntryData final
		{
			std::filesystem::path m_filePath = {};

			std::vector<AssetEntryData> m_childAssetEntryDataList = {};

			bool m_isDirectory = false;
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

		void RefreshAssetEntryTree();
		
		void BuildAssetEntryDataTree(AssetEntryData& a_assetEntryData);

		void DrawFolderPane();
		void DrawFolderTree(const AssetEntryData& a_assetEntryData);
		void DrawAssetPane ();

		bool DrawCurrentDirectoryAssetEntryList(const AssetEntryData& a_assetEntryData);

		bool DrawAssetEntryCard(const AssetEntryData& a_assetEntryData) const;
		
		static bool CompareAssetEntryData(const AssetEntryData& a_leftAssetEntryData, const AssetEntryData& a_rightAssetEntryData);

		std::string FetchVALAssetEntryDisplayName(const AssetEntryData& a_assetEntryData) const;
		std::string FetchVALAssetEntryIcon       (const AssetEntryData& a_assetEntryData) const;

		static constexpr std::string_view k_folderPaneChildLabel = "##AssetBrowserFolderPane";
		static constexpr std::string_view k_assetPaneChildLabel  = "##AssetBrowserAssetPane";
		static constexpr std::string_view k_assetCardButtonLabel = "##AssetCard";

		static constexpr std::string_view k_editorName                 = "アセットブラウザー";
		static constexpr std::string_view k_folderPaneTitleLabel       = "フォルダ";
		static constexpr std::string_view k_assetPaneTitleLabel        = "アセット";
		static constexpr std::string_view k_assetRootUnavailableLabel  = "Assetフォルダを読み込めません。";
		static constexpr std::string_view k_emptyDirectoryLabel        = "このフォルダは空です。";
		static constexpr std::string_view k_thisWindowExplanationLabel = "アセットブラウザーでは使用したいFBXファイルをモデル描画コンポーネントに\nドラッグ&ドロップしてロードするモデルとして扱ったり、使用したいゲームオブジェクトのプレハブをドラッグ&ドロップ\nでシーンに追加したりすることができるウィンドウ。";

		static constexpr std::wstring_view k_assetEntryNameEllipsis = L"...";

		static constexpr ImVec2 k_assetCardSize = { 104.0F, 92.0F };

		static constexpr std::size_t k_assetEntryNameVisibleCharacterCount      = 8ULL;
		static constexpr std::size_t k_assetEntryNameAbbreviationCharacterCount = k_assetEntryNameVisibleCharacterCount + 1ULL;
		static constexpr std::size_t k_minAssetCardColumnCount                  = 1ULL;
		static constexpr std::size_t k_nextAssetEntryIndexOffset                = 1ULL;
		static constexpr std::size_t k_assetCardRowEndRemainder                 = 0ULL;

		static constexpr float k_folderPaneWidth = 240.0F;

		static constexpr float k_fileRemainingArea = 0.0F;

		static constexpr float k_assetCardsSameLineOffsetX  = 0.0F;
		static constexpr float k_assetCardSpacing           = 8.0F;
		static constexpr float k_assetCardIconFontSize      = 48.0F;
		static constexpr float k_assetCardIconTopPadding    = 8.0F;
		static constexpr float k_assetCardTextBottomPadding = 8.0F;
		static constexpr float k_assetCardCenterRate        = 0.5F;

		AssetFilePathRegistry m_assetFilePathRegistry = {};

		AssetBrowserEditorWindowDirectoryWatcher m_directoryWatcher = {};

		Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

		std::filesystem::path m_currentDirectoryPath = Constant::k_assetRootFolderPath;

		AssetEntryData m_rootAssetEntryData = {};

		FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)