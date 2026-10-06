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
 
        void StartSceneRename             (const Scene&                     a_scene);
        void StartGameObjectRename        (const std::weak_ptr<GameObject>& a_gameObject);
        void StartRenameByCurrentSelection(const EditorManager&             a_editorManager);

        void OpenSceneNode     ();
        void OpenGameObjectNode(const std::weak_ptr<GameObject>& a_gameObject);
 
        const auto& GetREFSceneSelectionState() const { return m_sceneSelectionState; }
        const auto& GetREFSceneOperation     () const { return m_sceneOperation; }
        const auto& GetREFGameObjectOperation() const { return m_gameObjectOperation; }
        const auto& GetREFAssetCreator       () const { return m_assetCreator; }
        const auto& GetREFClipboard          () const { return m_clipboard; }
        const auto& GetREFRenameState        () const { return m_renameState; }
 
        auto& GetMutableREFSceneSelectionState() { return m_sceneSelectionState; }
        auto& GetMutableREFRenameState        () { return m_renameState; }
        auto& GetMutableREFClipboard          () { return m_clipboard; }

    private:
       
        void DrawSceneNode(Scene& a_scene, EditorManager& a_editorManager);

        void DrawGameObjectNode(const std::weak_ptr<GameObject>& a_gameObject,
                                      Scene&                     a_scene, 
                                      EditorManager&             a_editorManager,
                                const bool                       a_isFirstOrder = false);

        void DrawRenameInputText   (      Scene&                     a_scene);
        void DrawGameObjectDropZone(const std::weak_ptr<GameObject>& a_targetGameObject, const bool a_isDropAfter, Scene& a_scene) const;
        
        void HandleGameObjectDropTarget(const std::weak_ptr<GameObject>& a_targetGameObject, Scene&         a_scene);
        void HandlePrefabFileDropTarget(      Scene&                     a_scene,            EditorManager& a_editorManager);

        void PushSelectionChangeCommand(const boost::uuids::uuid&               a_beforeAnchorUUID,
                                        const bool                              a_beforeIsSceneSelected,
                                              EditorGameObjectSelectionState&   a_gameObjectSelectionState,
                                              std::vector<boost::uuids::uuid>&& a_beforeUUIDList) const;

        void SelectScene(EditorGameObjectSelectionState& a_gameObjectSelectionState, const bool a_isToggleSelection = false);

        void ClearSelection(EditorGameObjectSelectionState& a_gameObjectSelectionState);

        void FetchVALSelectionSnapshot(const EditorGameObjectSelectionState& a_gameObjectSelectionState, std::vector<boost::uuids::uuid>& a_outUUIDList, boost::uuids::uuid& a_outAnchorUUID) const;

        void SelectGameObject(const std::weak_ptr<GameObject>&      a_gameObject,
                              const bool                            a_isRangeSelection,
                              const bool                            a_isToggleSelection,
                                    Scene&                          a_scene,
                                    EditorGameObjectSelectionState& a_gameObjectSelectionState);
 
        void BuildDisplayedGameObjectList(std::vector<std::weak_ptr<GameObject>>& a_displayedList, Scene& a_scene) const;

        void CollectDisplayedGameObject(const std::weak_ptr<GameObject>& a_gameObject, std::vector<std::weak_ptr<GameObject>>& a_displayedList) const;
 
        void CommitRename(Scene& a_scene);
 
        bool IsGameObjectNodeOpen(const boost::uuids::uuid& a_sceneInstanceUUID) const;
        
        std::vector<std::weak_ptr<GameObject>>::const_iterator FindDisplayedGameObjectITR(const std::vector<std::weak_ptr<GameObject>>& a_displayedList, const std::shared_ptr<GameObject>& a_target) const;
         
        static constexpr std::string_view k_editorName                        = "アウトライナー";
        static constexpr std::string_view k_emptySceneLabel                   = "Untitled";
        static constexpr std::string_view k_thisWindowExplanationLabel        = "アウトライナーでは現在読み込んでいるシーン、シーンに含まれるゲームオブジェクトを見ることができ\n親子関係を結ぶ、名前を変える、シーンからゲームオブジェクトを削除することができるウィンドウ。";
        static constexpr std::string_view k_noCurrentSceneLabel               = "現在読み込まれているシーンはありません。";
        static constexpr std::string_view k_gameObjectDragDropZoneBeforeLabel = "Before";
        static constexpr std::string_view k_gameObjectDragDropZoneAfterLabel  = "After";
 
        static constexpr std::string_view k_sceneContextMenuLabel       = "##WorldOutlinerSceneContextMenu";
        static constexpr std::string_view k_gameObjectContextMenuLabel  = "##WorldOutlinerGameObjectContextMenu";
        static constexpr std::string_view k_emptySpaceContextMenuLabel  = "##WorldOutlinerEmptySpaceContextMenu";
        static constexpr std::string_view k_renameInputTextLabel        = "##WorldOutlinerRenameInputText";
        static constexpr std::string_view k_gameObjectDragDropZoneLabel = "##WorldOutlinerGameObjectDropZone";
        static constexpr std::string_view k_prefabFileDropTargetLabel   = "##WorldOutlinerPrefabFileDropTarget";

        static constexpr ImVec4 k_prefabGameObjectTextColor = { 0.40F, 0.70F, 1.00F, 1.00F };

        static constexpr float k_nodeFramePaddingHeight  = 3.0F;
        static constexpr float k_dropZoneHeight          = 3.0F;
        static constexpr float k_dropZoneHighlightHeight = 2.0F;
        
        static constexpr int k_keyboardFocusNextItem = 0;
        static constexpr int k_nodePopStyleNUM       = 2;
 
        static constexpr bool k_initialIsSceneNodeOpenValue = true;

        std::unordered_map<boost::uuids::uuid, bool> m_gameObjectOpenStateMap = {};
 
        WorldOutlinerEditorWindowSelectionState      m_sceneSelectionState = {};
        WorldOutlinerEditorWindowGameObjectOperation m_gameObjectOperation = {};
        WorldOutlinerEditorWindowSceneOperation      m_sceneOperation      = {};
        WorldOutlinerEditorWindowClipboard           m_clipboard           = {};
        WorldOutlinerEditorWindowAssetCreator        m_assetCreator        = {};
        WorldOutlinerEditorWindowPopupDrawer         m_popupDrawer         = {};
        WorldOutlinerEditorWindowShortcutHandler     m_shortcutHandler     = {};

        Struct::WorldOutlinerEditorWindowRenameState m_renameState = {};

        bool m_isSceneNodeOpen  = k_initialIsSceneNodeOpenValue;
        bool m_wasSceneHasChild = false;
        
        FWK_DEFINE_TYPE_INFO(WorldOutlinerEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::WorldOutlinerEditorWindow)