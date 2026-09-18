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

        void SetActivePane(const Enum::AssetBrowserActivePaneType a_set) { m_activePane = a_set; }

        void SetCurrentSelectFolderPath(const std::filesystem::path& a_set) { m_currentSelectFolderPath = a_set; }

        const auto& GetREFFolderHierarchyMap() const { return m_folderHierarchyMap; }

        const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

        const auto& GetREFClipboard   () const { return m_clipboard; }
        const auto& GetREFFolderPane  () const { return m_folderPane; }
        const auto& GetREFAssetPane   () const { return m_assetPane; }
        const auto& GetREFPaneSplitter() const { return m_paneSplitter; }

        const auto& GetREFFileOperation     () const { return m_fileOperation; }
        const auto& GetREFPopupDrawer       () const { return m_popupDrawer; }
        const auto& GetREFAssetCreator      () const { return m_assetCreator; }
        const auto& GetREFDeleteConfirmPopup() const { return m_deleteConfirmPopup; }
        
        const auto& GetREFCurrentSelectFolderPath() const { return m_currentSelectFolderPath; }

        const auto& GetREFRenameState       () const { return m_renameState; }
        const auto& GetREFDeleteConfirmState() const { return m_deleteConfirmState; }

        auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }

        auto& GetMutableREFFolderPane  () { return m_folderPane; }
        auto& GetMutableREFAssetPane   () { return m_assetPane; }
        auto& GetMutableREFPaneSplitter() { return m_paneSplitter; }

        auto& GetMutableREFClipboard     () { return m_clipboard; }
        auto& GetMutableREFFileOperation () { return m_fileOperation; }
        auto& GetMutableREFPopupDrawer   () { return m_popupDrawer; }
        
        auto& GetMutableREFRenameState       () { return m_renameState; }
        auto& GetMutableREFDeleteConfirmState() { return m_deleteConfirmState; }

        auto GetVALActivePane() const { return m_activePane; }

    private:

        void BuildFolderHierarchyMap(const std::filesystem::path& a_folderPath);

        static constexpr std::string_view k_paneSplitterLabel = "##AssetBrowserEditorWindowPaneSplitter";

        static constexpr std::string_view k_editorName                 = "アセットブラウザー";
        static constexpr std::string_view k_thisWindowExplanationLabel = "アセットブラウザーでは使用したいFBXファイルをモデル描画コンポーネントに\nドラッグ&ドロップしてロードするモデルとして扱ったり、使用したいゲームオブジェクトのプレハブをドラッグ&ドロップ\nでシーンに追加したりすることができるウィンドウ。";

        std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>> m_folderHierarchyMap = {};

        AssetFilePathRegistry m_assetFilePathRegistry = {};

        AssetBrowserEditorWindowDirectoryWatcher m_directoryWatcher = {};

        AssetBrowserEditorWindowFolderPane m_folderPane   = {};
        AssetBrowserEditorWindowAssetPane  m_assetPane    = {};
        EditorWindowPaneSplitter           m_paneSplitter = {};

        AssetBrowserEditorWindowClipboard          m_clipboard          = {};
        AssetBrowserEditorWindowFileOperation      m_fileOperation      = {};
        AssetBrowserEditorWindowShortcutHandler    m_shortcutHandler    = {};
        AssetBrowserEditorWindowPopupDrawer        m_popupDrawer        = {};
        AssetBrowserEditorWindowAssetCreator       m_assetCreator       = {};
        AssetBrowserEditorWindowDeleteConfirmPopup m_deleteConfirmPopup = {};

        std::filesystem::path m_currentSelectFolderPath = {};

        Converter::AssetBrowserEditorWindowJsonConverter m_jsonConverter = {};

        Struct::AssetBrowserEditorWindowRenameState        m_renameState        = {};
        Struct::AssetBrowserEditorWindowDeleteConfirmState m_deleteConfirmState = {};

        Enum::AssetBrowserActivePaneType m_activePane = Enum::AssetBrowserActivePaneType::Invalid;

        FWK_DEFINE_TYPE_INFO(AssetBrowserEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::AssetBrowserEditorWindow)