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
        const auto& GetREFFolderPane           () const { return m_folderPane; }

        auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }
        auto& GetMutableREFPaneSplitter         () { return m_paneSplitter; }
        auto& GetMutableREFFolderPane           () { return m_folderPane; }

    private:

        static constexpr std::string_view k_paneSplitterLabel = "##AssetBrowserEditorWindowPaneSplitter";

        static constexpr std::string_view k_editorName                 = "アセットブラウザー";
        static constexpr std::string_view k_thisWindowExplanationLabel = "アセットブラウザーでは使用したいFBXファイルをモデル描画コンポーネントに\nドラッグ&ドロップしてロードするモデルとして扱ったり、使用したいゲームオブジェクトのプレハブをドラッグ&ドロップ\nでシーンに追加したりすることができるウィンドウ。";

        AssetFilePathRegistry m_assetFilePathRegistry = {};

        AssetBrowserEditorWindowDirectoryWatcher m_directoryWatcher = {};

        AssetBrowserEditorWindowFolderPane m_folderPane = {};
        AssetBrowserEditorWindowAssetPane  m_assetPane  = {};

        AssetBrowserEditorWindowClipboard       m_clipboard       = {};
        AssetBrowserEditorWindowFileOperation   m_fileOperation   = {};
        AssetBrowserEditorWindowShortcutHandler m_shortcutHandler = {};
        AssetBrowserEditorWindowPopupDrawer     m_popupDrawer     = {};
        AssetBrowserEditorWindowAssetCreator    m_assetCreator    = {};

        Struct::AssetBrowserEditorWindowRenameState m_renameState = {};

        Enum::AssetBrowserActivePaneType m_activePane = Enum::AssetBrowserActivePaneType::Invalid;

        EditorWindowPaneSplitter m_paneSplitter = {};

        Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

        FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)