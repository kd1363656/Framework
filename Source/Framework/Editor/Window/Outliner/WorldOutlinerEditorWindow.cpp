#include "WorldOutlinerEditorWindow.h"

void FWK::Editor::WorldOutlinerEditorWindow::Draw(EditorManager& a_editorManager)
{
    // Outliner用ImGuiウィンドウを開始
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // SceneManagerが現在所有しているSceneをweak_ptrから取得する
    // lock()したshared_ptrはこのDraw()の間だけSceneの生存を保証する
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();
     
    if (!l_scene)
    {
        // TextDisabled()は通常のText()より薄い色で文字を描画する
        // Scene未読み込みはエラーではないため、警告色ではなく補助表示にする
        ImGui::TextDisabled(k_noCurrentSceneLabel.data());

        ImGui::End();

        return;
    }

    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
 
    // 破棄済み、または参照切れのGameObjectを選択状態から取り除く
    l_gameObjectSelectionState.SweepUnavailableGameObjects();

    // ショートカットキー処理
    // このWindowがアクティブでフォーカス中の時のみ有効にする
    // WantTextInput中(リネームInputText編集中)は無効にする
    if (const auto& l_io = ImGui::GetIO();
        a_editorManager.GetVALCurrentActiveWindowStaticTpeID() == WorldOutlinerEditorWindow::GetREFTypeINFO().k_staticTypeID &&
        ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)                                                               &&
        !l_io.WantTextInput)
    {
        m_shortcutHandler.Handle(*this, a_editorManager);
    }

    // シーンノードを描画する
    // 子ノードとしてルートGameObjectが続く
    // ノードの行の高さを規定より大きくして見やすくする
    const auto& l_style = ImGui::GetStyle();

    // シーンノードを描画する
    // 子ノードとしてルートGameObjectが続く
    // ノードの行の高さを規定より大きくして見やすくする
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(l_style.FramePadding.x, k_nodeFramePaddingHeight));

    // ノード間の隙間をドロップゾーンの高さ分だけにするため
    // アイテム間の垂直スペースを0にする
    // (0にしないとノードとドロップゾーン両方にItemSpacingが掛かり間隔が開きすぎる)
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(l_style.ItemSpacing.x, Constant::k_imguiRemainingSize.y));

    DrawSceneNode(*l_scene, a_editorManager);

    ImGui::PopStyleVar(k_nodePopStyleNUM);

    // 何も無い空スペースのクリック判定
    // ImGui::IsWindowHovered  : このWindow上にマウスがあるか
    // ImGui::IsAnyItemHovered : いずれかのアイテム上にマウスがあるか
    if (ImGui::IsWindowHovered() &&
        !ImGui::IsAnyItemHovered())
    {
        // 左クリック : 全ての選択を解除する
        // 選択されているものがあった場合のみUndoRedo履歴へコマンドが登録される
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ClearSelection(l_gameObjectSelectionState);
        }

        // 右クリック : 「空のゲームオブジェクトを作成」のみを持つポップアップを開く
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            m_popupDrawer.BeginPopup(k_emptySpaceContextMenuLabel);
        }
    }

    // 空スペース用ポップアップ描画
    // ポップアップが開いていない場合はDraw内部でreturnする
    m_popupDrawer.DrawEmptySpacePopup(k_emptySpaceContextMenuLabel, *l_scene, *this, a_editorManager);
 
    // アセットブラウザーからのPrefabファイルのドロップ先
    // Window内の空白へドロップされたPrefabはルートGameObjectとして生成する
    // GameObjectノード上はノード側のドロップ先が優先されるためここには届かない
    HandlePrefabFileDropTarget(*l_scene, a_editorManager);

    ImGui::End();
}

void FWK::Editor::WorldOutlinerEditorWindow::MoveSelectionUp(EditorManager& a_editorManager, const bool a_isRangeSelection)
{
    const auto& l_scene = SceneManager::GetInstance().GetVALScene().lock();

    if (!l_scene) { return; }

    // 表示中ノードリストを構築
    // 矢印キー押下時のみ構築するため毎フレームのオーバーヘッドなし
    std::vector<std::weak_ptr<GameObject>> l_displayedList = {};

    BuildDisplayedGameObjectList(l_displayedList, *l_scene);

    if (l_displayedList.empty()) { return; }

          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState   ();
    const auto& l_cursor                   = l_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();

    // 未選択の場合はシーンのGameObjectList先頭から始める
    if (!l_cursor)
    {
        SelectGameObject(l_displayedList.front().lock(),
                         false,
                         false,
                         *l_scene,
                         l_gameObjectSelectionState);
 
        return;
    }

    // weak_ptr同士は直接比較できないためlock()したshared_ptrのアドレスで比較する
    const auto& l_cursorITR = FindDisplayedGameObjectITR(l_displayedList, l_cursor);

    // リストに存在しないか既に先頭なら何もしない
    if (l_cursorITR == l_displayedList.end() ||
        l_cursorITR == l_displayedList.begin())
    {
        return;
    }

    const auto& l_prev = std::prev(l_cursorITR)->lock();

    if (!l_prev) { return; }

    SelectGameObject(l_prev,
                     a_isRangeSelection,
                     false,
                     *l_scene,
                     l_gameObjectSelectionState);

}
void FWK::Editor::WorldOutlinerEditorWindow::MoveSelectionDown(EditorManager& a_editorManager, const bool a_isRangeSelection)
{
    const auto& l_scene = SceneManager::GetInstance().GetVALScene().lock();
 
    if (!l_scene) { return; }
 
    std::vector<std::weak_ptr<GameObject>> l_displayedList = {};
 
    // 表示中ノードリストを構築
    // 矢印キー押下時のみ構築するため毎フレームのオーバーヘッドなし
    BuildDisplayedGameObjectList(l_displayedList, *l_scene);
 
    if (l_displayedList.empty()) { return; }
 
          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState   ();
    const auto& l_cursor                   = l_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();
 
    // 未選択の場合はシーンのGameObjectList先頭から始める
    if (!l_cursor)
    {
        SelectGameObject(l_displayedList.front().lock(),
                         false,
                         false,
                         *l_scene,
                         l_gameObjectSelectionState);
 
        return;
    }
 
    // weak_ptr同士は直接比較できないためlock()したshared_ptrのアドレスで比較する
    const auto& l_cursorITR = FindDisplayedGameObjectITR(l_displayedList, l_cursor);

    if (l_cursorITR == l_displayedList.end()) { return; }
 
    const auto& l_nextITR = std::next(l_cursorITR);
 
    // 既に末尾なら何もしない
    if (l_nextITR == l_displayedList.end()) { return; }
 
    const auto& l_next = l_nextITR->lock();
 
    if (!l_next) { return; }
 
    SelectGameObject(l_next, a_isRangeSelection, false, *l_scene, l_gameObjectSelectionState);
}

void FWK::Editor::WorldOutlinerEditorWindow::SelectAllGameObjects(EditorManager& a_editorManager)
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();
 
    if (!l_scene) { return; }
 
    const auto& l_sceneGameObjectList = l_scene->GetREFGameObjectList();

    std::vector<std::weak_ptr<GameObject>> l_allGameObjectList = {};
 
    l_allGameObjectList.reserve(l_sceneGameObjectList.size());
 
    for (const auto& l_gameObject : l_scene->GetREFGameObjectList())
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }
 
        l_allGameObjectList.emplace_back(l_gameObject);
    }
 
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();

    // UndoRedoコマンド登録用に変更前の選択状態をUUIDリストとして取得する
    std::vector<boost::uuids::uuid> l_beforeUUIDList   = {};
    boost::uuids::uuid              l_beforeAnchorUUID = {};

    FetchVALSelectionSnapshot(l_gameObjectSelectionState, l_beforeUUIDList, l_beforeAnchorUUID);

    const bool l_beforeIsSceneSelected = m_sceneSelectionState.GetVALIsSceneSelected();

    // 表示順は関係なくシーン登録順で全て選択する
    l_gameObjectSelectionState.SelectGameObjectRange(l_allGameObjectList);

    // Ctrl + Aの選択対象はGameObjectのみのためScene選択は解除する
    m_sceneSelectionState.SetIsSceneSelected(false);

    // 選択変更後の状態を取得してコマンドをPushする
    PushSelectionChangeCommand(std::move(l_beforeUUIDList),
                               l_beforeAnchorUUID,
                               l_beforeIsSceneSelected,
                               l_gameObjectSelectionState);
}

void FWK::Editor::WorldOutlinerEditorWindow::StartSceneRename(const Scene& a_scene)
{
    m_renameState.m_isActive         = true;
    m_renameState.m_isFocused        = false;
    m_renameState.m_isSceneTarget    = true;
    m_renameState.m_targetGameObject = {};
 
    // 現在の名前を入力バッファへ入れておく
    const auto& l_name     = a_scene.GetREFName();
    const auto  l_copySize = std::min          (l_name.size(), m_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);
 
    std::memcpy(m_renameState.m_inputBuffer.data(), l_name.data(), l_copySize);
 
    m_renameState.m_inputBuffer[l_copySize] = Constant::k_nullCharacter;
}
void FWK::Editor::WorldOutlinerEditorWindow::StartGameObjectRename(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }
 
    m_renameState.m_isActive         = true;
    m_renameState.m_isFocused        = false;
    m_renameState.m_isSceneTarget    = false;
    m_renameState.m_targetGameObject = l_gameObject;
 
    const auto& l_name     = l_gameObject->GetREFName();
    const auto  l_copySize = std::min                (l_name.size(), m_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);
 
    std::memcpy(m_renameState.m_inputBuffer.data(), l_name.data(), l_copySize);
 
    m_renameState.m_inputBuffer[l_copySize] = Constant::k_nullCharacter;
}
void FWK::Editor::WorldOutlinerEditorWindow::StartRenameByCurrentSelection(const EditorManager& a_editorManager)
{
    const auto& l_gameObjectSelectionState = a_editorManager.GetREFGameObjectSelectionState();
 
    // GameObjectが選択されていれば最後に選択したものを対象にする
    if (const auto& l_lastSelected = l_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();
        l_lastSelected)
    {
        StartGameObjectRename(l_lastSelected);
 
        return;
    }
 
    // GameObject未選択でSceneが選択されているならシーン名を対象にする
    if (!m_sceneSelectionState.GetVALIsSceneSelected()) { return; }
 
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();
 
    if (!l_scene) { return; }
 
    StartSceneRename(*l_scene);
}

void FWK::Editor::WorldOutlinerEditorWindow::DrawSceneNode(Scene& a_scene, EditorManager& a_editorManager)
{
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
 
    // SpanAvailWidth    : ノードのクリック範囲をウィンドウ幅いっぱいまで広げる
    // OpenOnArrow       : 矢印部分をクリックした場合のみ開閉する
    // OpenOnDoubleClick : ダブルクリックで開閉する
    ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
                                         ImGuiTreeNodeFlags_OpenOnArrow    |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick;
 
    // シーンが選択されていれば選択されているというフラグを上げる
    if (m_sceneSelectionState.GetVALIsSceneSelected())
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // 有効なルートGameObjectを一つでも持つか
    // 一つも無ければ開閉矢印の無いリーフノードとして描画する
    const bool l_hasChild = std::ranges::any_of(a_scene.GetREFGameObjectList(),
                                                [](const auto& a_gameObject)
                                                {
                                                    if (!a_gameObject ||
                                                        a_gameObject->GetVALIsDestroyed())
                                                    {
                                                        return false;
                                                    }

                                                    const auto& l_hierarchy = a_gameObject->GetREFHierarchy();

                                                    return l_hierarchy.GetREFParent().expired();
                                                });
 
    if (!l_hasChild)
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
                           ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    // リネーム中はノードのテキスト部分をInputTextへ置き換える
    const bool l_isRenaming = m_renameState.m_isActive && 
                              m_renameState.m_isSceneTarget;
 
    // シーン名が空の場合は代替名を表示する
    const auto& l_sceneName = a_scene.GetREFName();
          auto  l_label     = l_sceneName.empty () ? std::string{ k_emptySceneLabel } : l_sceneName;
 
    // 名前変更中なら名前変更中の文字列を描画
    if (l_isRenaming)
    {
        l_label = k_renameInputTextLabel;
    }

    const auto& l_nodeLabel = std::string{ Constant::k_imguiFontAwesomeSceneIcon } + " " + l_label;

    // 前フレームでGameObjectが一つも無かった場合に
    // 初めてGameObjectが追加されたならシーンノードを自動で開く
    // (追加したGameObjectがノード内で見えた方が分かりやすいため)
    if (l_hasChild &&
        !m_wasSceneHasChild)
    {
        m_isSceneNodeOpen = true;
    }

    // 次フレームの比較用に今フレームの状態を保持しておく
    m_wasSceneHasChild = l_hasChild;

    // 保持している開閉状態をImGuiへ反映する
    // GameObjectノードと同じ仕組みでユーザーが閉じた状態も維持される
    ImGui::SetNextItemOpen(m_isSceneNodeOpen);

    // 選択中・ホバー中の背景色をエディタ共通ルールでPushする
    // 他ウィンドウのアイテムと同じ選択色で描画される
    Utility::IMGUIPushItemHighlightColors(m_sceneSelectionState.GetVALIsSceneSelected(), a_editorManager.GetVALCurrentActiveWindowStaticTpeID() == GetREFTypeINFO().k_staticTypeID);

    const bool l_isNodeOpen = ImGui::TreeNodeEx(l_nodeLabel.c_str(), l_treeNodeFlags);

    Utility::IMGUIPopItemHighlightColors();

    // 開閉状態を保持して次フレームへ引き継ぐ
    m_isSceneNodeOpen = l_isNodeOpen;

    // 左クリック : シーンを選択
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        const auto& l_io = ImGui::GetIO();

        // Ctrl + クリック : Scene選択のトグル(GameObjectとの混在選択を許可する)
        SelectScene(l_gameObjectSelectionState, l_io.KeyCtrl);
    }

    // 右クリック : 選択 + ポップアップ
    // 既に選択されている場合は選択状態を維持する
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        if (!m_sceneSelectionState.GetVALIsSceneSelected())
        {
            SelectScene(l_gameObjectSelectionState);
        }
 
        m_popupDrawer.BeginPopup(k_sceneContextMenuLabel);
    }

    // ポップアップ描画
    // OpenPopupと同じIDスタックスコープで呼ぶ必要があるためここで行う
    m_popupDrawer.DrawScenePopup(k_sceneContextMenuLabel, 
                                 a_scene,
                                 *this, 
                                 a_editorManager);

    // アセットブラウザーからのPrefabファイルのドロップ先
    // SceneノードへドロップされたPrefabはルートGameObjectとして生成する

    std::vector<std::filesystem::path> l_droppedFilePathList = {};
 
    if (auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
        l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_imguiAssetBrowserFolderDragAndDropPayloadLabel, l_droppedFilePathList))
    {
        // Prefabとして登録されているファイルのみ生成対象になる
        auto l_createdList = m_assetCreator.CreateGameObjectFromPrefabDrop(l_droppedFilePathList, {}, a_scene);
 
        // 生成したGameObjectを選択状態にする
        if (!l_createdList.empty())
        {
            l_gameObjectSelectionState.ClearSelectedGameObjectList();
 
            for (const auto& l_created : l_createdList)
            {
                l_gameObjectSelectionState.AddSelectedGameObject(l_created);
            }

            auto& l_editorManager = EditorManager::GetInstance                  ();
            auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

            l_undoRedoSystem.PushUndoCommand<CreateGameObjectCommand>(std::move(l_createdList), boost::uuids::uuid{});
        }
    }

    // リネーム中はノードのテキスト位置にInputTextを重ねる
    if (l_isRenaming)
    {
        DrawRenameInputText(a_scene);
    }

    if (!l_isNodeOpen ||
        !l_hasChild) 
    {
        return; 
    }
 
    bool l_isFirstOrder = true;

    // 親を持たないルートGameObjectをシーンの登録順に描画する
    for (const auto& l_gameObject : a_scene.GetREFGameObjectList())
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 親を持つGameObjectは親ノード側の再帰で描画されるためここではスキップ
        if (const auto& l_hierarchy = l_gameObject->GetREFHierarchy();
            !l_hierarchy.GetREFParent().expired())
        {
            continue; 
        }
 
        DrawGameObjectNode(l_gameObject,
                           a_scene,
                           a_editorManager,
                           l_isFirstOrder);

        l_isFirstOrder = false;
    }
 
    // TreeNodeExで一段下がったインデントを戻す
    ImGui::TreePop();   
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNode(const std::weak_ptr<GameObject>& a_gameObject, 
                                                                      Scene&                     a_scene, 
                                                                      EditorManager&             a_editorManager,
                                                                const bool                       a_isFirstOrder)
{
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
 
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // シーンノード直下にゲームオブジェクトの順番を入れ替えるためのドロップゾーン
    if (a_isFirstOrder)
    {
        DrawGameObjectDropZone(a_gameObject, false, a_scene);
    }

    const auto& l_hierarchy                   = l_gameObject->GetREFHierarchy                      ();
    const auto& l_childSmartPointerVectorList = l_hierarchy.GetREFChildSmartPointerVectorList      ();
    const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList();
 
    // 有効な子GameObjectを一つでも持つか
    // 全て無効・破棄済みならリーフノードとして描画する
    const bool l_hasChild = std::ranges::any_of(l_childDataList,
                                                [](const auto& a_childData)
                                                {
                                                    const auto& l_child = a_childData.m_type.lock();
 
                                                    return l_child &&
                                                           !l_child->GetVALIsDestroyed();
                                                });
 
    ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
                                         ImGuiTreeNodeFlags_OpenOnArrow    |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick;

    // 子を一つも持たなければリーフノードとして扱う
    if (!l_hasChild)
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
                           ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    else
    {
        const bool l_isNodeOpen = IsGameObjectNodeOpen(l_gameObject->GetREFSceneInstanceUUID());

        // 保持している開閉状態をImGuiへ反映する
        ImGui::SetNextItemOpen(l_isNodeOpen);
    }

    // 選択中ならハイライト表示
    if (l_gameObjectSelectionState.FindVALIsSelected(a_gameObject))
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // 名前の文字色をPrefab状態に応じて変える
    // 白(デフォルト) : Prefabと無関係
    // 青             : Prefab由来、またはPrefabルート
    // 赤             : PrefabUUIDを持つがPrefabSystemに対応Prefabが見つからない
    const auto& l_prefabUUID = l_gameObject->GetREFPrefabUUID();
 
    const auto& l_style     = ImGui::GetStyle();
          auto  l_textColor = l_style.Colors[ImGuiCol_Text];

    if (!l_prefabUUID.is_nil())
    {
        const auto& l_prefabSystem = a_scene.GetREFGameObjectPrefabSystem();
       
        l_textColor = l_prefabSystem.FindPTRPrefab(l_prefabUUID) ? k_prefabGameObjectTextColor : Constant::k_imguiDangerColor;
    }

    // Cut対象なら文字色を半透明にする
    // AssetBrowserと同じくテキスト色へ半分倍率を掛けて暗くする
    // 左クリックでの選択は可能なまま維持する(選択フラグには影響しない)
    if (m_clipboard.Contains(l_gameObject->GetREFSceneInstanceUUID()) &&
        m_clipboard.GetVALOperationType() == Enum::WorldOutlinerClipboardOperationType::Cut)
    {
        // 透明度含めすべて0.5を掛ける
        l_textColor *= Constant::k_halfMagnification;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, l_textColor);

    // 選択中・ホバー中・Cut対象の背景色をエディタ共通ルールでPushする
    // 他ウィンドウのアイテムと同じ選択色で描画される
    const bool l_isSelected  = l_gameObjectSelectionState.FindVALIsSelected(a_gameObject);
    const bool l_isCutTarget = m_clipboard.Contains(l_gameObject->GetREFSceneInstanceUUID()) &&
                               m_clipboard.GetVALOperationType() == Enum::WorldOutlinerClipboardOperationType::Cut;

    Utility::IMGUIPushItemHighlightColors(l_isSelected, a_editorManager.GetVALCurrentActiveWindowStaticTpeID() == GetREFTypeINFO().k_staticTypeID, l_isCutTarget);

    // このノードがリネーム対象かどうか
    const bool l_isRenaming = m_renameState.m_isActive       &&
                              !m_renameState.m_isSceneTarget &&
                              m_renameState.m_targetGameObject.lock() == l_gameObject;
 
    const auto& l_name  = l_gameObject->GetREFName();
          auto  l_label = l_name.empty            () ? std::string{ Constant::k_gameObjectString } : l_name;
 
    if (l_isRenaming)
    {
        l_label = k_renameInputTextLabel;
    }

    // TreeNodeのIDはラベル文字列から作られるため同名ノードで衝突する
    // 「##」以降はIDのみに使われ描画されないのでSceneInstanceUUIDを埋めて一意にする
    const auto& l_nodeLabel  = l_label + "##" + boost::uuids::to_string(l_gameObject->GetREFSceneInstanceUUID());
    const bool  l_isNodeOpen = ImGui::TreeNodeEx                       (l_nodeLabel.c_str(), l_treeNodeFlags);

    // 後からPushした順に戻す
    // Header系3色分(IMGUIPushItemHighlightColors) → テキスト色の順でPopする
    Utility::IMGUIPopItemHighlightColors();
    ImGui::PopStyleColor               ();

    const auto& l_sceneInstanceUUID = l_gameObject->GetREFSceneInstanceUUID();

    // 開閉状態をMapへ反映する
    // BuildDisplayedGameObjectListが参照するため毎フレーム同期しておく
    if (l_hasChild)
    {
        m_gameObjectOpenStateMap[l_sceneInstanceUUID] = l_isNodeOpen;
    }

    // リネーム中でなければドラッグ元として登録する
    // 直前のアイテム(TreeNodeEx)を対象にするため
    // RenameInputTextなど別アイテムを挟む前に呼ぶ必要がある
    if (!l_isRenaming)
    {
        auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

        // ラムダ式ははDrag開始Frameに一度だけ実行される
        l_imguiDragDropPayloadStorage.DragDropSource(Constant::k_gameObjectDragDropPayloadLabel,
                                                     [&a_gameObject]
                                                     {
                                                         return a_gameObject;
                                                     });
    }

    // ノード本体へのドロップ先
    // ここにドロップするとドロップしたGameObjectがこのノードの子になる
    HandleGameObjectDropTarget(a_gameObject, a_scene);

    // リネーム中はノードのテキスト位置にInputTextを重ねる
    if (l_isRenaming)
    {
        DrawRenameInputText(a_scene);
    }

    // 左クリック         : 選択
    // 左クリック + Shift : 範囲選択
    // 左クリック + Ctrl  : 選択、選択解除のトグル
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        const auto& l_io = ImGui::GetIO();

        // 修飾キーなしで既選択のノードを押下した場合は選択リストを更新しない
        // この時点で単一選択へ潰すとドラッグ対象が1件だけになってしまうため
        // 複数選択を維持したままドラッグ&ドロップできるようにする
        // Shift + Click(範囲選択)・Ctrl + Click(トグル)は従来通り選択を更新する
        if (!l_gameObjectSelectionState.FindVALIsSelected(a_gameObject) ||
            l_io.KeyShift ||
            l_io.KeyCtrl)
        {
            SelectGameObject(a_gameObject,
                             l_io.KeyShift,
                             l_io.KeyCtrl,
                             a_scene,
                             l_gameObjectSelectionState);
        }
    }

    // 左ボタン解放 : 押下したノードの真上で解放された場合のみ選択を確定する
    // 押下時点で選択を潰さないので複数選択を維持したままドラッグ&ドロップできる
    // ドラッグが成立していた場合はGetDragDropPayloadが非nullのため選択を更新しない
    if (ImGui::IsItemDeactivated() &&
        ImGui::IsItemHovered()     &&
        !ImGui::GetDragDropPayload())
    {
        const auto& l_io = ImGui::GetIO();
     
        // Shift/Ctrlでの選択操作は押下時に更新済みのため解放時は無修飾のみ処理する
        // 既選択ノードへの無修飾クリックは単一選択へ潰す
        if (!l_io.KeyShift &&
            !l_io.KeyCtrl  &&
            l_gameObjectSelectionState.FindVALIsSelected(a_gameObject))
        {
            SelectGameObject(a_gameObject,
                             false,
                             false,
                             a_scene,
                             l_gameObjectSelectionState);
        }
    }

    // ノードごとに一意なポップアップラベル
    const auto& l_contextMenuLabel = std::string{ k_gameObjectContextMenuLabel } + boost::uuids::to_string(l_sceneInstanceUUID);
 
    // 右クリック : 選択 + ポップアップ
    // 選択済みノード上での右クリックは複数選択状態を維持する
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        if (!l_gameObjectSelectionState.FindVALIsSelected(a_gameObject))
        {
            SelectGameObject(a_gameObject,
                             false,
                             false,
                             a_scene,
                             l_gameObjectSelectionState);
        }
 
        m_popupDrawer.BeginPopup(l_contextMenuLabel);
    }

    // ポップアップ描画
    // OpenPopupと同じIDスタックスコープで呼ぶ必要があるためここで行う
    m_popupDrawer.DrawGameObjectPopup(l_gameObject, 
                                      l_contextMenuLabel, 
                                      a_scene, 
                                      *this, 
                                      a_editorManager);

    // ノード下の隙間ドロップゾーン
    // ここにドロップするとこのノードより下(同じ階層)に挿入される
    // ノードが開いている場合は子孫の描画が終わりインデントが戻った後に配置するため
    // 子ノードの位置ではなくこのノードと同じ階層の「下」として機能する
    DrawGameObjectDropZone(a_gameObject, true, a_scene);

    // 子GameObjectを再帰的に描画
    if (l_isNodeOpen &&
        l_hasChild)
    {
        for (const auto& l_childData : l_childDataList)
        {
            const auto& l_child = l_childData.m_type.lock();
 
            if (!l_child ||
                l_child->GetVALIsDestroyed())
            {
                continue;
            }
 
            DrawGameObjectNode(l_child, a_scene, a_editorManager);
        }

        ImGui::TreePop();
    }
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawRenameInputText(Scene& a_scene)
{
    // TreeNodeExのテキスト位置にInputTextを重ねる
    ImGui::SameLine();

    // TreeNodeExはテキストベースで描画されるがInputTextはフレーム付きで高さが異なる
    // AlignTextToFramePaddingで垂直位置をテキストベースラインへ合わせる
    ImGui::AlignTextToFramePadding();

    // リネーム開始直後の1フレーム目のみフォーカスを当てる
    if (!m_renameState.m_isFocused)
    {
        ImGui::SetKeyboardFocusHere(k_keyboardFocusNextItem);
    }

    const auto& l_style = ImGui::GetStyle();

    // InputTextのフレームパディングを小さくしてTreeNodeExのテキスト高さに近づける
    ImGui::PushStyleVar   (ImGuiStyleVar_FramePadding, ImVec2(l_style.FramePadding.x, Constant::k_imguiInputTextHightPaddingAlignHight));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

    const bool l_isEnterPressed = ImGui::InputText(k_renameInputTextLabel.data(),
                                                   m_renameState.m_inputBuffer.data(),
                                                   m_renameState.m_inputBuffer.size(),
                                                   ImGuiInputTextFlags_EnterReturnsTrue |
                                                   ImGuiInputTextFlags_AutoSelectAll);
 
    ImGui::PopStyleVar();
 
    if (ImGui::IsItemFocused())
    {
        m_renameState.m_isFocused = true;
    }
 
    // 空白クリックでリネームを確定する
    const bool l_isEmptySpaceClick = ImGui::IsWindowHovered()   &&
                                     !ImGui::IsAnyItemHovered() &&
                                     ImGui::IsMouseClicked(ImGuiMouseButton_Left);
 
    // 確定条件 : Enter押下かフォーカス取得後にフォーカス消失かフォーカス取得後の空白クリック
    if (l_isEnterPressed          ||
       (m_renameState.m_isFocused && 
       (!ImGui::IsItemFocused()   ||
        l_isEmptySpaceClick)))
    {
        CommitRename(a_scene);
    }
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectDropZone(const std::weak_ptr<GameObject>& a_targetGameObject, const bool a_isDropAfter, Scene& a_scene) const
{
    const auto& l_targetGameObject = a_targetGameObject.lock();
 
    if (!l_targetGameObject) { return; }
 
    // ノード間の隙間を表す薄いドロップゾーン
    // InvisibleButtonは描画しないがサイズ分のレイアウトと当たり判定を持つ
    // 「##」以降はIDのみに使われ描画されないため
    // SceneInstanceUUIDと上下どちらのゾーンかを埋めて一意にする
    const auto& l_dropZoneLabel = std::string{ k_gameObjectDragDropZoneLabel } + boost::uuids::to_string(l_targetGameObject->GetREFSceneInstanceUUID()) + (a_isDropAfter ? std::string{ k_gameObjectDragDropZoneAfterLabel } : std::string{ k_gameObjectDragDropZoneBeforeLabel });

    ImGui::InvisibleButton(l_dropZoneLabel.c_str(), ImVec2(Constant::k_imguiRemainingSize.x, k_dropZoneHeight));
 
    // ドラッグ中のPayloadを取得
    // 非ドラッグ中はnullptr、別種別のPayloadならIsDataTypeがfalseになる
    // ドラッグ中にホバーされたらドロップ位置が分かるようゾーンをハイライトする
    // ImGuiHoveredFlags_AllowWhenBlockedByActiveItemを指定する
    // ドラッグ中はg.ActiveIDがドラッグ元アイテムのIDになり
    // デフォルトのIsItemHovered()は別アイテムがActiveの間falseを返すため
    // このフラグを付けないとドロップ先のホバー判定が取れない
    if (const auto* l_imguiDragDropPayload = ImGui::GetDragDropPayload();
        l_imguiDragDropPayload                                                                &&
        l_imguiDragDropPayload->IsDataType(Constant::k_gameObjectDragDropPayloadLabel.data()) &&
        ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
              auto* l_drawList = ImGui::GetWindowDrawList();
        const auto& l_color    = Constant::k_imguiAccentColor * Constant::k_imguiImVec4ToImU32;
 
        // InvisibleButtonの矩形を取得
        const auto& l_itemMIN = ImGui::GetItemRectMin();
        const auto& l_itemMAX = ImGui::GetItemRectMax();

        const float l_centerY = (l_itemMIN.y + l_itemMAX.y) * Constant::k_halfMagnification;

        const ImVec2& l_highlightMIN = { l_itemMIN.x, l_centerY - k_dropZoneHighlightHeight * Constant::k_halfMagnification };
        const ImVec2& l_highlightMAX = { l_itemMAX.x, l_centerY + k_dropZoneHighlightHeight * Constant::k_halfMagnification };

        l_drawList->AddRectFilled(l_highlightMIN,
                                  l_highlightMAX,
                                  IM_COL32(l_color.x,
                                           l_color.y,
                                           l_color.z,
                                           l_color.w));
    }
 
    std::weak_ptr<GameObject> l_droppedGameObject = {};
 
    // Drop成立時のみtrueを返す
    // weak_ptrの参照先が既に破棄されている場合も内部で弾かれる
    if (auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
        !l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_gameObjectDragDropPayloadLabel, l_droppedGameObject))
    {
        return;
    }
 
    // 隙間ドロップはターゲットと同じ階層の兄弟として上下へ挿入する
    // a_isDropAfter = true ならターゲットの下、false なら上
    // ターゲットがルートならドロップ側もルート化され
    // Scene::m_gameObjectList内の順序が入れ替わる
    m_gameObjectOperation.MoveGameObjectSiblingOrder(a_targetGameObject,
                                                     l_droppedGameObject,
                                                     a_isDropAfter,
                                                     a_scene);
}

void FWK::Editor::WorldOutlinerEditorWindow::HandleGameObjectDropTarget(const std::weak_ptr<GameObject>& a_targetGameObject, Scene& a_scene)
{
    auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
 
    // ノード本体へのDrop成立時のみtrueを返す
    if (std::weak_ptr<GameObject> l_droppedGameObject = {};
        l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_gameObjectDragDropPayloadLabel, l_droppedGameObject))
    {
        const auto& l_dropped = l_droppedGameObject.lock();
 
        if (!l_dropped) { return; }
 
        // UndoRedo用に変更前の親UUIDを保存する
        const auto&              l_beforeHierarchy  = l_dropped->GetREFHierarchy    ();
        const auto&              l_beforeParent     = l_beforeHierarchy.GetREFParent().lock();
              boost::uuids::uuid l_beforeParentUUID = {};
        const auto&              l_gameObjectUUID   = l_dropped->GetREFSceneInstanceUUID();
 
        if (l_beforeParent)
        {
            l_beforeParentUUID = l_beforeParent->GetREFSceneInstanceUUID();
        }
 
        // ドロップ先ノードを親、ドロップしたGameObjectを子として親子関係を結ぶ
        // 自分自身・子孫への親付けはApplyParent側で弾かれる
        // 実行レベルの再構築もReparentGameObject内で行われる
        m_gameObjectOperation.ReparentGameObject(a_targetGameObject, l_droppedGameObject, a_scene);
 
        const auto&               l_afterHierarchy  = l_dropped->GetREFHierarchy   ();
        const auto&               l_afterParent     = l_afterHierarchy.GetREFParent().lock();
              boost::uuids::uuid  l_afterParentUUID = {};
 
        if (l_afterParent)
        {
            l_afterParentUUID = l_afterParent->GetREFSceneInstanceUUID();
        }
 
        // 親子関係が実際に変更された場合のみコマンドをPushする
        if (l_beforeParentUUID != l_afterParentUUID)
        {
            // EditorManager経由でUndoRedoSystemへコマンドをPushする
            auto& l_editorManager  = EditorManager::GetInstance                 ();
            auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();
 
            l_undoRedoSystem.PushUndoCommand<ReparentGameObjectCommand>(l_gameObjectUUID, l_beforeParentUUID, l_afterParentUUID);
        }
 
        // 子が追加されたのでドロップ先ノードを開いた状態にして結果を見せる
        if (const auto& l_targetGameObject = a_targetGameObject.lock();
            l_targetGameObject)
        {
            m_gameObjectOpenStateMap[l_targetGameObject->GetREFSceneInstanceUUID()] = true;
        }
 
        return;
    }

    // アセットブラウザーからのPrefabファイルのドロップを受け付ける
    // ノード本体へドロップされたPrefabはこのGameObjectの子として生成する
    std::vector<std::filesystem::path> l_droppedFilePathList = {};

    if (!l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_imguiAssetBrowserFolderDragAndDropPayloadLabel, l_droppedFilePathList)) { return; }

    auto l_createdList = m_assetCreator.CreateGameObjectFromPrefabDrop(l_droppedFilePathList, a_targetGameObject, a_scene);

    if (l_createdList.empty()) { return; }

    auto& l_editorManager            = EditorManager::GetInstance                           ();
    auto& l_gameObjectSelectionState = l_editorManager.GetMutableREFGameObjectSelectionState();
 
    l_gameObjectSelectionState.ClearSelectedGameObjectList();
 
    for (const auto& l_created : l_createdList)
    {
        // プレハブファイルから作成したゲームオブジェクトを選択状態にする
        l_gameObjectSelectionState.AddSelectedGameObject(l_created);
    }

    // 生成をUndoRedo履歴へ登録する
    // 親はドロップ先のGameObject
    boost::uuids::uuid l_parentSceneInstanceUUID = {};

    if (const auto& l_targetGameObject = a_targetGameObject.lock();
        l_targetGameObject)
    {
        l_parentSceneInstanceUUID = l_targetGameObject->GetREFSceneInstanceUUID();
    }

    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<CreateGameObjectCommand>(std::move(l_createdList), l_parentSceneInstanceUUID);

    // 子が追加されたのでドロップ先ノードを開いた状態にして結果を見せる
    if (const auto& l_targetGameObject = a_targetGameObject.lock();
        l_targetGameObject)
    {
        const auto& l_sceneInstanceUUID = l_targetGameObject->GetREFSceneInstanceUUID();

        m_gameObjectOpenStateMap[l_sceneInstanceUUID] = true;
    }
}
void FWK::Editor::WorldOutlinerEditorWindow::HandlePrefabFileDropTarget(Scene& a_scene, EditorManager& a_editorManager)
{
    const auto* l_currentWindow = ImGui::GetCurrentWindow();

    if (!l_currentWindow) { return; }

    std::vector<std::filesystem::path> l_droppedFilePathList = {};

    auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

    // Window全体の矩形をドロップ先にする
    // GameObjectペイロード用のDropZoneやノードのドロップ先とは
    // ペイロードラベルが異なるため干渉しない
    // GameObjectノード上はノード側(小さい矩形)のドロップ先が優先されるため
    // ここに届くのは空白部分へのドロップのみになる
    if (!l_imguiDragDropPayloadStorage.DragDropTargetCustom(l_currentWindow->Rect(),
                                                            Constant::k_imguiAssetBrowserFolderDragAndDropPayloadLabel,
                                                            ImGui::GetID(k_prefabFileDropTargetLabel.data()),
                                                            l_droppedFilePathList))
    {
        return;
    }

    // Prefabとして登録されているファイルのみ生成対象になる
    auto l_createdList = m_assetCreator.CreateGameObjectFromPrefabDrop(l_droppedFilePathList, {}, a_scene);
 
    if (l_createdList.empty()) { return; }
 
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
 
    l_gameObjectSelectionState.ClearSelectedGameObjectList();
 
    for (const auto& l_created : l_createdList)
    {
        // プレハブファイルから作成したゲームオブジェクトを選択状態にする
        l_gameObjectSelectionState.AddSelectedGameObject(l_created);
    }

    // 生成をUndoRedo履歴へ登録する
    // 空白へのドロップなので親はなし(NilUUID)
    auto& l_undoRedoSystem = a_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<CreateGameObjectCommand>(std::move(l_createdList), boost::uuids::uuid{});
}

void FWK::Editor::WorldOutlinerEditorWindow::FetchVALSelectionSnapshot(const EditorGameObjectSelectionState& a_gameObjectSelectionState, std::vector<boost::uuids::uuid>& a_outUUIDList, boost::uuids::uuid& a_outAnchorUUID) const
{
    // 現在の選択状態をUUIDリストとして取得する
    const auto& l_selectedList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();
    const auto& l_anchor       = a_gameObjectSelectionState.GetREFRangeSelectionAnchor  ().lock();

    a_outUUIDList.clear  ();
    a_outUUIDList.reserve(l_selectedList.size());

    for (const auto& l_selectedWeak : l_selectedList)
    {
        const auto& l_selectedGameObject = l_selectedWeak.lock();

        if (!l_selectedGameObject) { continue; }

        a_outUUIDList.emplace_back(l_selectedGameObject->GetREFSceneInstanceUUID());
    }

    a_outAnchorUUID = {};

    if (l_anchor)
    {
        a_outAnchorUUID = l_anchor->GetREFSceneInstanceUUID();
    }
}

void FWK::Editor::WorldOutlinerEditorWindow::PushSelectionChangeCommand(      std::vector<boost::uuids::uuid>&& a_beforeUUIDList,
                                                                        const boost::uuids::uuid&               a_beforeAnchorUUID,
                                                                        const bool                              a_beforeIsSceneSelected,
                                                                              EditorGameObjectSelectionState&   a_gameObjectSelectionState) const
{
    // 変更後の選択状態をUUIDリストとして取得する
    std::vector<boost::uuids::uuid> l_afterUUIDList   = {};
    boost::uuids::uuid              l_afterAnchorUUID = {};

    FetchVALSelectionSnapshot(a_gameObjectSelectionState, l_afterUUIDList, l_afterAnchorUUID);

    const bool l_afterIsSceneSelected = m_sceneSelectionState.GetVALIsSceneSelected();

    // 変更前後で選択状態が同じならコマンドをPushしない
    if (l_afterIsSceneSelected == a_beforeIsSceneSelected &&
        l_afterUUIDList        == a_beforeUUIDList        &&
        l_afterAnchorUUID      == a_beforeAnchorUUID)
    {
        return;
    }

    // EditorManager経由でUndoRedoSystemへコマンドをPushする
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<ChangeSelectionCommand>(std::move(a_beforeUUIDList),
                                                             std::move(l_afterUUIDList),
                                                             a_beforeAnchorUUID,
                                                             l_afterAnchorUUID,
                                                             a_beforeIsSceneSelected,
                                                             l_afterIsSceneSelected);
}

void FWK::Editor::WorldOutlinerEditorWindow::SelectScene(EditorGameObjectSelectionState& a_gameObjectSelectionState, const bool a_isToggleSelection)
{
    // UndoRedoコマンド登録用に変更前の選択状態をUUIDリストとして取得する
    std::vector<boost::uuids::uuid> l_beforeUUIDList   = {};
    boost::uuids::uuid              l_beforeAnchorUUID = {};

    FetchVALSelectionSnapshot(a_gameObjectSelectionState, l_beforeUUIDList, l_beforeAnchorUUID);

    const bool l_beforeIsSceneSelected = m_sceneSelectionState.GetVALIsSceneSelected();

    // Ctrl + クリック : Scene選択のトグル(GameObjectとの混在選択を許可する)
    if (a_isToggleSelection)
    {
        m_sceneSelectionState.ToggleSceneSelect(a_gameObjectSelectionState);
    }
    else
    {
        m_sceneSelectionState.SelectSingleScene(a_gameObjectSelectionState);
    }

    // 選択変更後の状態を取得してコマンドをPushする
    PushSelectionChangeCommand(std::move(l_beforeUUIDList),
                               l_beforeAnchorUUID,
                               l_beforeIsSceneSelected,
                               a_gameObjectSelectionState);
}

void FWK::Editor::WorldOutlinerEditorWindow::ClearSelection(EditorGameObjectSelectionState& a_gameObjectSelectionState)
{
    // UndoRedoコマンド登録用に変更前の選択状態をUUIDリストとして取得する
    std::vector<boost::uuids::uuid> l_beforeUUIDList   = {};
    boost::uuids::uuid              l_beforeAnchorUUID = {};

    FetchVALSelectionSnapshot(a_gameObjectSelectionState, l_beforeUUIDList, l_beforeAnchorUUID);

    const bool l_beforeIsSceneSelected = m_sceneSelectionState.GetVALIsSceneSelected();

    a_gameObjectSelectionState.ClearSelectedGameObjectList();

    m_sceneSelectionState.SetIsSceneSelected(false);

    // 選択されているものがあった場合のみコマンドがPushされる
    PushSelectionChangeCommand(std::move(l_beforeUUIDList),
                               l_beforeAnchorUUID,
                               l_beforeIsSceneSelected,
                               a_gameObjectSelectionState);
}

void FWK::Editor::WorldOutlinerEditorWindow::SelectGameObject(const std::weak_ptr<GameObject>&      a_gameObject,
                                                              const bool                            a_isRangeSelection,
                                                              const bool                            a_isToggleSelection,
                                                                    Scene&                          a_scene,
                                                                    EditorGameObjectSelectionState& a_gameObjectSelectionState)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // UndoRedoコマンド登録用に変更前の選択状態をUUIDリストとして取得する
    std::vector<boost::uuids::uuid> l_beforeUUIDList   = {};
    boost::uuids::uuid              l_beforeAnchorUUID = {};

    FetchVALSelectionSnapshot(a_gameObjectSelectionState, l_beforeUUIDList, l_beforeAnchorUUID);

    const bool l_beforeIsSceneSelected = m_sceneSelectionState.GetVALIsSceneSelected();

    // SceneとGameObjectは排他選択のため
    // GameObjectが選択された時点でScene選択は解除する
    m_sceneSelectionState.SetIsSceneSelected(false);

    // Shift + クリック : アンカーからクリック点までを範囲選択する
    if (a_isRangeSelection)
    {
        // アンカーが未設定の場合は単一選択として扱う
        if (const auto& l_anchor = a_gameObjectSelectionState.GetREFRangeSelectionAnchor().lock();
            !l_anchor)
        {
            a_gameObjectSelectionState.SelectSingleGameObject(a_gameObject);
        }
        else
        {
            // 表示中ノードリストを構築して表示順の範囲を決定する
            std::vector<std::weak_ptr<GameObject>> l_displayedList = {};
 
            BuildDisplayedGameObjectList(l_displayedList, a_scene);

            const auto& l_startITR = FindDisplayedGameObjectITR(l_displayedList, l_anchor);
            const auto& l_endITR   = FindDisplayedGameObjectITR(l_displayedList, l_gameObject);
 
            if (l_startITR != l_displayedList.end() &&
                l_endITR   != l_displayedList.end())
            {
                // アンカーがクリック点より後ろにある場合は入れ替える
                auto l_beginITR = l_startITR;
                auto l_lastITR  = l_endITR;
 
                if (l_beginITR > l_lastITR)
                {
                    std::swap(l_beginITR, l_lastITR);
                }
 
                // 両端を含む範囲を選択リストへ渡す
                a_gameObjectSelectionState.SelectGameObjectRange({ l_beginITR, std::next(l_lastITR) });
            }
            else
            {
                // 閉じたノード内など表示されていない場合のフォールバック
                // 簡易的にアンカーとクリック点の二つだけを選択する
                a_gameObjectSelectionState.SelectGameObjectRange({ l_anchor, a_gameObject });
            }
        }

        // 選択変更後の状態を取得してコマンドをPushする
        PushSelectionChangeCommand(std::move(l_beforeUUIDList),
                                   l_beforeAnchorUUID,
                                   l_beforeIsSceneSelected,
                                   a_gameObjectSelectionState);
 
        return;
    }


    // Ctrl + クリック : 選択 / 選択解除のトグル
    if (a_isToggleSelection)
    {
        a_gameObjectSelectionState.ToggleSelectedGameObject(a_gameObject);

        // 選択変更後の状態を取得してコマンドをPushする
        PushSelectionChangeCommand(std::move(l_beforeUUIDList),
                                   l_beforeAnchorUUID,
                                   l_beforeIsSceneSelected,
                                   a_gameObjectSelectionState);

        return;
    }

    // 通常クリック : 選択をクリアして単一選択
    a_gameObjectSelectionState.SelectSingleGameObject(a_gameObject);

    // 選択変更後の状態を取得してコマンドをPushする
    PushSelectionChangeCommand(std::move(l_beforeUUIDList),
                               l_beforeAnchorUUID,
                               l_beforeIsSceneSelected,
                               a_gameObjectSelectionState);
}

void FWK::Editor::WorldOutlinerEditorWindow::BuildDisplayedGameObjectList(std::vector<std::weak_ptr<GameObject>>& a_displayedList, Scene& a_scene) const
{
    // ルートGameObjectから再帰的に表示順のリストを構築する
    for (const auto& l_gameObject : a_scene.GetREFGameObjectList())
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 親を持つGameObjectは親ノード側の再帰で収集されるためここではスキップ
        if (const auto& l_hierarchy = l_gameObject->GetREFHierarchy();
            !l_hierarchy.GetREFParent().expired())
        {
            continue; 
        }
 
        CollectDisplayedGameObject(l_gameObject, a_displayedList);
    }
}

void FWK::Editor::WorldOutlinerEditorWindow::CollectDisplayedGameObject(const std::weak_ptr<GameObject>& a_gameObject, std::vector<std::weak_ptr<GameObject>>& a_displayedList) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    a_displayedList.emplace_back(a_gameObject);
 
    // 閉じているノードの子は表示されないため収集しない
    if (const auto& l_sceneInstanceUUID = l_gameObject->GetREFSceneInstanceUUID();
        l_sceneInstanceUUID.is_nil() ||
        !IsGameObjectNodeOpen(l_sceneInstanceUUID)) 
    {
        return; 
    }
 
    const auto& l_hierarchy                    = l_gameObject->GetREFHierarchy                       ();
    const auto& l_childSmartPointerVectorArray = l_hierarchy.GetREFChildSmartPointerVectorList       ();
    const auto& l_childDataList                = l_childSmartPointerVectorArray.GetREFElementDataList();
 
    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        if (!l_child ||
            l_child->GetVALIsDestroyed())
        {
            continue;
        }
 
        CollectDisplayedGameObject(l_child, a_displayedList);
    }
}

void FWK::Editor::WorldOutlinerEditorWindow::CommitRename(Scene& a_scene)
{
    // 先にフラグを下ろす
    // 確定処理中に再度この関数が呼ばれないようにするため
    m_renameState.m_isActive  = false;
    m_renameState.m_isFocused = false;
 
    const auto& l_newName = std::string{ m_renameState.m_inputBuffer.data() };

    // 空文字列なら変更しない
    if (l_newName.empty()) { return; }

    // シーン名の変更
    if (m_renameState.m_isSceneTarget)
    {
        m_sceneOperation.RenameScene(l_newName, a_scene);
        
        return;
    }

    const auto& l_targetGameObject = m_renameState.m_targetGameObject.lock();

    // ゲームオブジェクトにポインタが格納されていたらゲームオブジェクトの名前をリネーム
    if (!l_targetGameObject) { return; }

    m_gameObjectOperation.RenameGameObject(l_targetGameObject, l_newName);
}

bool FWK::Editor::WorldOutlinerEditorWindow::IsGameObjectNodeOpen(const boost::uuids::uuid& a_sceneInstanceUUID) const
{
    const auto& l_itr = m_gameObjectOpenStateMap.find(a_sceneInstanceUUID);

    // 未登録のノードは開いた状態として扱う
    if (l_itr == m_gameObjectOpenStateMap.end()) { return true; }

    return l_itr->second;
}

std::vector<std::weak_ptr<FWK::GameObject>>::const_iterator FWK::Editor::WorldOutlinerEditorWindow::FindDisplayedGameObjectITR(const std::vector<std::weak_ptr<GameObject>>& a_displayedList, const std::shared_ptr<GameObject>& a_target) const
{
    // 無効なターゲットは「見つからない」扱いにする
    if (!a_target) { return a_displayedList.end(); }
 
    // weak_ptr同士は直接比較できないためlock()したshared_ptrのアドレスで比較する
    return std::find_if(a_displayedList.begin(), a_displayedList.end(),
                        [&a_target](const auto& a_gameObjectWeak)
                        {
                            return a_gameObjectWeak.lock() == a_target;
                        });
}