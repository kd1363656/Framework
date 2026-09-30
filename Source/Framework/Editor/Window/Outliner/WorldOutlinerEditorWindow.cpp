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
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, k_nodeFramePaddingHeight));
    DrawSceneNode      (*l_scene, a_editorManager);
    ImGui::PopStyleVar ();

    // 何も無い空スペースのクリック判定
    // ImGui::IsWindowHovered  : このWindow上にマウスがあるか
    // ImGui::IsAnyItemHovered : いずれかのアイテム上にマウスがあるか
    if (ImGui::IsWindowHovered() &&
        !ImGui::IsAnyItemHovered())
    {
        // 左クリック : 全ての選択を解除する
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            l_gameObjectSelectionState.ClearSelectedGameObjectList();

            m_sceneSelectionState.SetIsSceneSelected(false);
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


          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState   ();
    const auto& l_cursor                   = l_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();

    // 未選択の場合はシーンのGameObjectList先頭から始める
    if (!l_cursor)
    {
        SelectGameObject(l_displayedList.front().lock(),
                         *l_scene,
                         l_gameObjectSelectionState,
                         false, 
                         false);
 
        return;
    }

    // weak_ptr同士は直接比較できないためlock()したshared_ptrのアドレスで比較する
    const auto& l_cursorITR = std::find_if(l_displayedList.begin(), l_displayedList.end(),
                                           [&l_cursor](const auto& a_gameObjectWeak)
                                           {
                                               return a_gameObjectWeak.lock() == l_cursor;
                                           });

    // リストに存在しないか既に先頭なら何もしない
    if (l_cursorITR == l_displayedList.end()   ||
        l_cursorITR == l_displayedList.begin())
    {
        return;
    }

    const auto& l_prev = std::prev(l_cursorITR)->lock();

    if (!l_prev) { return; }

    SelectGameObject(l_prev, 
                    *l_scene,
                    l_gameObjectSelectionState,
                    a_isRangeSelection, 
                    false);

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
                         *l_scene, 
                         l_gameObjectSelectionState,
                         false, 
                         false);
 
        return;
    }
 
    const auto& l_cursorITR = std::find_if(l_displayedList.begin(), l_displayedList.end(),
                                           [&l_cursor](const auto& a_gameObjectWeak)
                                           {
                                               return a_gameObjectWeak.lock() == l_cursor;
                                           });
 
    if (l_cursorITR == l_displayedList.end()) { return; }
 
    const auto& l_nextITR = std::next(l_cursorITR);
 
    // 既に末尾なら何もしない
    if (l_nextITR == l_displayedList.end()) { return; }
 
    const auto& l_next = l_nextITR->lock();
 
    if (!l_next) { return; }
 
    SelectGameObject(l_next, *l_scene, l_gameObjectSelectionState, a_isRangeSelection, false);
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
 
    // 表示順は関係なくシーン登録順で全て選択する
    l_gameObjectSelectionState.SelectGameObjectRange(l_allGameObjectList);
 
    // Ctrl + Aの選択対象はGameObjectのみのためScene選択は解除する
    m_sceneSelectionState.SetIsSceneSelected(false);
}

void FWK::Editor::WorldOutlinerEditorWindow::StartSceneRename(Scene& a_scene)
{
    m_renameState.m_isActive         = true;
    m_renameState.m_isFocused        = false;
    m_renameState.m_isSceneTarget    = true;
    m_renameState.m_targetGameObject = {};
 
    // 現在の名前を入力バッファへ入れておく
    const auto& l_name     = a_scene.GetREFName();
    const auto  l_copySize = std::min(l_name.size(), m_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);
 
    std::memcpy(m_renameState.m_inputBuffer.data(), l_name.data(), l_copySize);
 
    m_renameState.m_inputBuffer[l_copySize] = Constant::k_nullCharacter;
}
void FWK::Editor::WorldOutlinerEditorWindow::StartGameObjectRename(const std::shared_ptr<GameObject>& a_gameObject)
{
    if (!a_gameObject) { return; }
 
    m_renameState.m_isActive         = true;
    m_renameState.m_isFocused        = false;
    m_renameState.m_isSceneTarget    = false;
    m_renameState.m_targetGameObject = a_gameObject;
 
    const auto& l_name     = a_gameObject->GetREFName();
    const auto  l_copySize = std::min(l_name.size(), m_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);
 
    std::memcpy(m_renameState.m_inputBuffer.data(), l_name.data(), l_copySize);
 
    m_renameState.m_inputBuffer[l_copySize] = Constant::k_nullCharacter;
}
void FWK::Editor::WorldOutlinerEditorWindow::StartRenameByCurrentSelection(EditorManager& a_editorManager)
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
                                                    const auto& l_hierarchy = a_gameObject->GetREFHierarchy();

                                                    return a_gameObject                       &&
                                                           !a_gameObject->GetVALIsDestroyed() &&
                                                           l_hierarchy.GetREFParent().expired();
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
          auto  l_label     = l_sceneName.empty() ? std::string{ k_emptySceneLabel } : l_sceneName;
 
    // 名前変更中なら名前変更中の文字列を描画
    if (l_isRenaming)
    {
        l_label = k_renameInputTextLabel;
    }

    const auto& l_nodeLabel  = std::string{ Constant::k_imguiFontAwesomeSceneIcon } + " " + l_label;
    const bool  l_isNodeOpen = ImGui::TreeNodeEx(l_nodeLabel.c_str(), l_treeNodeFlags);

    // 左クリック : シーンを選択
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        // Ctrl + クリック : Scene選択のトグル(GameObjectとの混在選択を許可する)
        if (const auto& l_io = ImGui::GetIO();
            l_io.KeyCtrl)
        {
            m_sceneSelectionState.ToggleSceneSelect(l_gameObjectSelectionState);
        }
        else
        {
            m_sceneSelectionState.SelectSingleScene(l_gameObjectSelectionState);
        }
    }

    // 右クリック : 選択 + ポップアップ
    // 既に選択されている場合は選択状態を維持する
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        if (!m_sceneSelectionState.GetVALIsSceneSelected())
        {
            m_sceneSelectionState.SelectSingleScene(l_gameObjectSelectionState);
        }
 
        m_popupDrawer.BeginPopup(k_sceneContextMenuLabel);
    }

    // ポップアップ描画
    // OpenPopupと同じIDスタックスコープで呼ぶ必要があるためここで行う
    m_popupDrawer.DrawScenePopup(k_sceneContextMenuLabel, 
                                 a_scene,
                                 *this, 
                                 a_editorManager);

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
 
        DrawGameObjectNode(l_gameObject, a_scene, a_editorManager);
    }
 
    // TreeNodeExで一段下がったインデントを戻す
    ImGui::TreePop();   
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNode(const std::shared_ptr<GameObject>& a_gameObject, Scene& a_scene, EditorManager& a_editorManager)
{
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
 
    const auto& l_hierarchy                   = a_gameObject->GetREFHierarchy                      ();
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
        const bool l_isNodeOpen = IsGameObjectNodeOpen(a_gameObject->GetREFSceneInstanceUUID());

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
    const auto& l_prefabUUID      = a_gameObject->GetREFPrefabUUID();
          bool  l_isPushTextColor = false;
 
    if (!l_prefabUUID.is_nil())
    {
        const auto& l_prefabSystem = a_scene.GetREFGameObjectPrefabSystem();
        const auto& l_textColor    = l_prefabSystem.FindPTRPrefab        (l_prefabUUID) ? Constant::k_imguiWorldOutlinerPrefabGameObjectTextColor : Constant::k_imguiWorldOutlinerMissingPrefabTextColor;

        ImGui::PushStyleColor(ImGuiCol_Text, l_textColor);
 
        l_isPushTextColor = true;
    }
 
    // このノードがリネーム対象かどうか
    const bool l_isRenaming = m_renameState.m_isActive       &&
                              !m_renameState.m_isSceneTarget &&
                              m_renameState.m_targetGameObject.lock() == a_gameObject;
 
    const auto& l_name  = a_gameObject->GetREFName();
          auto  l_label = l_name.empty            () ? std::string{ Constant::k_gameObjectString } : l_name;
 
    if (l_isRenaming)
    {
        l_label = k_renameInputTextLabel;
    }

    // TreeNodeのIDはラベル文字列から作られるため同名ノードで衝突する
    // 「##」以降はIDのみに使われ描画されないのでSceneInstanceUUIDを埋めて一意にする
    const auto& l_nodeLabel  = l_label + "##" + boost::uuids::to_string(a_gameObject->GetREFSceneInstanceUUID());
    const bool  l_isNodeOpen = ImGui::TreeNodeEx                       (l_nodeLabel.c_str(), l_treeNodeFlags);

    // テキストカラーをPushした分Popする
    if (l_isPushTextColor)
    {
        ImGui::PopStyleColor();
    }

    const auto& l_sceneInstanceUUID = a_gameObject->GetREFSceneInstanceUUID();

    // 開閉状態をMapへ反映する
    // BuildDisplayedGameObjectListが参照するため毎フレーム同期しておく
    if (l_hasChild)
    {
        m_gameObjectOpenStateMap[l_sceneInstanceUUID] = l_isNodeOpen;
    }
 
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
 
        SelectGameObject(a_gameObject,
                         a_scene,
                         l_gameObjectSelectionState,
                         l_io.KeyShift,
                         l_io.KeyCtrl);
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
                             a_scene,
                             l_gameObjectSelectionState,
                             false,
                             false);
        }
 
        m_popupDrawer.BeginPopup(l_contextMenuLabel);
    }

    // ポップアップ描画
    // OpenPopupと同じIDスタックスコープで呼ぶ必要があるためここで行う
    m_popupDrawer.DrawGameObjectPopup(a_gameObject, 
                                      l_contextMenuLabel, 
                                      a_scene, 
                                      *this, 
                                      a_editorManager);

    // 子GameObjectを再帰的に描画
    if (!l_isNodeOpen ||
        !l_hasChild)
    {
        return;
    }
 
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

void FWK::Editor::WorldOutlinerEditorWindow::SelectGameObject(const std::shared_ptr<GameObject>&    a_gameObject,
                                                                    Scene&                          a_scene, 
                                                                    EditorGameObjectSelectionState& a_gameObjectSelectionState, 
                                                              const bool                            a_isRangeSelection, 
                                                              const bool                            a_isToggleSelection)
{
    if (!a_gameObject) { return; }

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
 
            const auto l_findITR = [&l_displayedList](const std::shared_ptr<GameObject>& a_target)
            {
                return std::find_if(l_displayedList.begin(), l_displayedList.end(),
                                    [&a_target](const auto& a_gameObjectWeak)
                                    {
                                        return a_gameObjectWeak.lock() == a_target;
                                    });
            };
 
            const auto& l_startITR = l_findITR(l_anchor);
            const auto& l_endITR   = l_findITR(a_gameObject);
 
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
 
        return;
    }


    // Ctrl + クリック : 選択 / 選択解除のトグル
    if (a_isToggleSelection)
    {
        a_gameObjectSelectionState.ToggleSelectedGameObject(a_gameObject);

        return;
    }

    // 通常クリック : 選択をクリアして単一選択
    a_gameObjectSelectionState.SelectSingleGameObject(a_gameObject);
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
 
        const auto& l_hierarchy = l_gameObject->GetREFHierarchy();

        // 親を持つGameObjectは親ノード側の再帰で収集されるためここではスキップ
        if (!l_hierarchy.GetREFParent().expired()) { continue; }
 
        CollectDisplayedGameObject(l_gameObject, a_displayedList);
    }
}

void FWK::Editor::WorldOutlinerEditorWindow::CollectDisplayedGameObject(const std::shared_ptr<GameObject>& a_gameObject, std::vector<std::weak_ptr<GameObject>>& a_displayedList) const
{
    a_displayedList.emplace_back(a_gameObject);
 
    // 閉じているノードの子は表示されないため収集しない
    if (const auto& l_sceneInstanceUUID = a_gameObject->GetREFSceneInstanceUUID();
        l_sceneInstanceUUID.is_nil() ||
        !IsGameObjectNodeOpen(l_sceneInstanceUUID)) 
    {
        return; 
    }
 
    const auto& l_childDataList = a_gameObject->GetREFHierarchy().GetREFChildSmartPointerVectorList().GetREFElementDataList();
 
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