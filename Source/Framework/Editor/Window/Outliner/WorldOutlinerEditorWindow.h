#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
    class WorldOutlinerEditorWindow final : public EditorWindowBase
    {
    public:

         WorldOutlinerEditorWindow()          = default;
        ~WorldOutlinerEditorWindow() override = default;

        void Draw(EditorManager& a_editorManager) override;

        void MoveSelectionUp  (EditorManager& a_editorManager, const bool a_isRangeSelection = false);
        void MoveSelectionDown(EditorManager& a_editorManager, const bool a_isRangeSelection = false);
 
        void SelectAllGameObjects(EditorManager& a_editorManager);
 
        void StartSceneRename             (      Scene&                       a_scene);
        void StartGameObjectRename        (const std::shared_ptr<GameObject>& a_gameObject);
        void StartRenameByCurrentSelection(      EditorManager&               a_editorManager);
 
        const auto& GetREFSceneSelectionState() const { return m_sceneSelectionState; }
        const auto& GetREFAssetCreator       () const { return m_assetCreator; }
        const auto& GetREFClipboard          () const { return m_clipboard; }
        const auto& GetREFRenameState        () const { return m_renameState; }
 
        auto& GetMutableREFSceneSelectionState() { return m_sceneSelectionState; }
        auto& GetMutableREFRenameState        () { return m_renameState; }
        auto& GetMutableREFClipboard          () { return m_clipboard; }

    private:
       
        void DrawSceneNode           (Scene& a_scene, EditorManager& a_editorManager);
        void DrawGameObjectNode      (const std::shared_ptr<GameObject>& a_gameObject, Scene& a_scene, EditorManager& a_editorManager);
        void DrawRenameInputText     (      Scene&                       a_scene);
 
        void SelectGameObject(const std::shared_ptr<GameObject>&    a_gameObject,
                                    Scene&                          a_scene,
                                    EditorGameObjectSelectionState& a_gameObjectSelectionState,
                              const bool                            a_isRangeSelection,
                              const bool                            a_isToggleSelection);
 
        void BuildDisplayedGameObjectList(std::vector<std::weak_ptr<GameObject>>& a_displayedList, Scene& a_scene) const;

        void CollectDisplayedGameObject(const std::shared_ptr<GameObject>& a_gameObject, std::vector<std::weak_ptr<GameObject>>& a_displayedList) const;
 
        void CommitRename(Scene& a_scene);
 
        bool IsGameObjectNodeOpen(const boost::uuids::uuid& a_sceneInstanceUUID) const;
 
         
        static constexpr std::string_view k_editorName                 = "アウトライナー";
        static constexpr std::string_view k_emptySceneLabel            = "Untitled";
        static constexpr std::string_view k_thisWindowExplanationLabel = "アウトライナーでは現在読み込んでいるシーン、シーンに含まれるゲームオブジェクトを見ることができ\n親子関係を結ぶ、名前を変える、シーンからゲームオブジェクトを削除することができるウィンドウ。";
        static constexpr std::string_view k_noCurrentSceneLabel        = "現在読み込まれているシーンはありません。";
 
        static constexpr std::string_view k_sceneContextMenuLabel      = "##WorldOutlinerSceneContextMenu";
        static constexpr std::string_view k_gameObjectContextMenuLabel = "##WorldOutlinerGameObjectContextMenu";
        static constexpr std::string_view k_emptySpaceContextMenuLabel = "##WorldOutlinerEmptySpaceContextMenu";
        static constexpr std::string_view k_renameInputTextLabel       = "##WorldOutlinerRenameInputText";
 
        static constexpr std::string_view k_directionalLightLabel = "Directional Light";
 
        static constexpr int k_keyboardFocusNextItem = 0;
 
        std::unordered_map<boost::uuids::uuid, bool> m_gameObjectOpenStateMap = {};
 
        WorldOutlinerEditorWindowSelectionState  m_sceneSelectionState = {};
        WorldOutlinerEditorWindowClipboard       m_clipboard           = {};
        WorldOutlinerEditorWindowAssetCreator    m_assetCreator        = {};
        WorldOutlinerEditorWindowPopupDrawer     m_popupDrawer         = {};
        WorldOutlinerEditorWindowShortcutHandler m_shortcutHandler     = {};
 
        Struct::WorldOutlinerEditorWindowRenameState m_renameState = {};
        
        FWK_DEFINE_TYPE_INFO(WorldOutlinerEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::WorldOutlinerEditorWindow)