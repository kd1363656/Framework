#include "WorldOutlinerEditorWindowGameObjectOperation.h"

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::RenameGameObject(const std::weak_ptr<GameObject>& a_gameObject, const std::string& a_newName) const
{
    const auto& l_gameObject = a_gameObject.lock();

    // 無効な対象、空文字列への変更は行わない
    if (!l_gameObject ||
        a_newName.empty())
    {
        return;
    }

    l_gameObject->SetName(a_newName);
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::DestroySelectedGameObjects(EditorGameObjectSelectionState& a_gameObjectSelectionState, Scene& a_scene) const
{
    const auto& l_selectedList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();

    if (l_selectedList.empty()) { return; }

    // 選択されているルートごとに、取り外すサブツリーと元の親を記録する
    std::vector<Struct::DestroyedGameObjectRecord> l_destroyedGameObjectRecordList = {};

    l_destroyedGameObjectRecordList.reserve(l_selectedList.size());

    for (const auto& l_targetWeak : l_selectedList)
    {
        const auto& l_target = l_targetWeak.lock();

        if (!l_target ||
            l_target->GetVALIsDestroyed())
        {
            continue;
        }

        // 親子を同時選択していた場合、子は親のサブツリーに含まれているためスキップ
        if (Utility::HasAncestorInList(l_selectedList, l_targetWeak)) { continue; }

        auto l_record = FetchVALDestroyedGameObjectRecord(l_targetWeak);

        // 破棄ではなくシーン管理から取り外す
        // 実体はコマンド側のshared_ptrが保持し続けるためUndoで復元できる
        Utility::DetachGameObjectSubtree(l_record.m_detachedSubtreeGameObjectList, a_scene);

        l_destroyedGameObjectRecordList.emplace_back(std::move(l_record));
    }

    if (l_destroyedGameObjectRecordList.empty()) { return; }

    // 取り外しをUndoRedo履歴へ登録する
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<DestroyGameObjectCommand>(std::move(l_destroyedGameObjectRecordList));

    // 取り外したGameObjectの選択状態を解除する
    a_gameObjectSelectionState.ClearSelectedGameObjectList();
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::ReparentGameObject(const std::weak_ptr<GameObject>& a_targetGameObject, const std::weak_ptr<GameObject>& a_moveGameObject, Scene& a_scene) const
{
    const auto& l_targetGameObject = a_targetGameObject.lock();
    const auto& l_moveGameObject   = a_moveGameObject.lock  ();

    // 無効な対象、自分自身への親付けは行わない
    if (!l_targetGameObject || 
        !l_moveGameObject   ||
        l_targetGameObject == l_moveGameObject)
    {
        return;
    }
 
    // UndoRedo用に変更前の親と兄弟の並びを記録する
    const auto& l_beforeState = Utility::FetchVALReparentGameObjectState(l_moveGameObject, a_scene);

    // 自分自身・子孫への親付けガード、旧親からの除去、
    // TransformのHierarchical化はApplyParent側で行われる
    if (auto& l_moveGameObjectHierarchy = l_moveGameObject->GetMutableREFHierarchy();
        !l_moveGameObjectHierarchy.ApplyParent(a_targetGameObject)) 
    {
        return; 
    }
 
    // 階層の深さが変わったため実行レベルを再構築する
    a_scene.RebuildGameObjectExecutionLevelList();

    // 親や兄弟の並びが変わっていればUndoRedo履歴へ登録する
    PushReparentGameObjectCommand(l_moveGameObject, a_scene, l_beforeState);
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::MoveGameObjectSiblingOrder(const std::weak_ptr<GameObject>& a_targetGameObject,
                                                                                           const std::weak_ptr<GameObject>& a_moveGameObject,
                                                                                           const bool                       a_isMoveAfter,
                                                                                                 Scene&                     a_scene) const
{
    const auto& l_targetGameObject = a_targetGameObject.lock();
    const auto& l_moveGameObject   = a_moveGameObject.lock  ();

    if (!l_targetGameObject || 
        !l_moveGameObject   ||
        l_targetGameObject == l_moveGameObject)
    {
        return;
    }
 
    // UndoRedo用に変更前の親と兄弟の並びを記録する
    const auto& l_beforeState = Utility::FetchVALReparentGameObjectState(l_moveGameObject, a_scene);

    // ドロップ先と同じ階層に合わせる
    // ターゲットがルートならmove側もルート化、子ならその親の子になる
    const auto& l_targetHierarchy = l_targetGameObject->GetREFHierarchy();
    const auto& l_targetParent    = l_targetHierarchy.GetREFParent     ();
 
    // 上か下にドロップしたときのターゲットの親をドロップしたゲームオブジェクトの親にすることで
    // 兄弟関係を結ぶ
    // (既に同じ親ならApplyParentは早期returnで何もしない)
    if (auto& l_moveGameObjectHierarchy = l_moveGameObject->GetMutableREFHierarchy();
        !l_moveGameObjectHierarchy.ApplyParent(l_targetParent))
    {
        return; 
    }
 
    if (const auto& l_parent = l_targetParent.lock())
    {
        auto& l_parentHierarchy = l_parent->GetMutableREFHierarchy();

        // 親の子リスト内で並べ替える
        // ApplyParentで末尾に追加済みなので目的位置へ移動させる
        auto& l_childSmartPointerVectorList = l_parentHierarchy.GetMutableREFChildSmartPointerVectorList();
        auto& l_childDataList               = l_childSmartPointerVectorList.GetMutableREFElementDataList();
 
        // ドロップした対象と上か下にドロップされたターゲットのリストの位置を取得
        const auto& l_moveITR   = FindChildGameObjectITR(a_moveGameObject,   l_childDataList);
        const auto& l_targetITR = FindChildGameObjectITR(a_targetGameObject, l_childDataList);
 
        if (l_moveITR   == l_childDataList.end() ||
            l_targetITR == l_childDataList.end())
        {
            return;
        }
 
        // eraseするとイテレータが無効化されるため
        // 一度要素を取り出し、ターゲット位置を探し直す
        const auto l_moveData = *l_moveITR;
 
        l_childDataList.erase(l_moveITR);
 
        // 探し出したターゲット位置はeraseで無効化されるので再度探しなおす
        const auto& l_newTargetITR = FindChildGameObjectITR(a_targetGameObject, l_childDataList);
 
        // m_isMoveAfter = trudeでターゲットの後、そうじゃないならターゲットの前
        l_childDataList.insert(a_isMoveAfter ? std::next(l_newTargetITR) : l_newTargetITR, l_moveData);
    }
    // 親がいなければルートリストの方の並びを変更する
    else
    {
        // ルートリスト内で並べ替える
        auto& l_gameObjectList = a_scene.GetMutableREFGameObjectList();
 
        const auto& l_moveITR   = std::ranges::find(l_gameObjectList, l_moveGameObject);
        const auto& l_targetITR = std::ranges::find(l_gameObjectList, l_targetGameObject);
 
        if (l_moveITR   == l_gameObjectList.end() ||
            l_targetITR == l_gameObjectList.end())
        {
            return;
        }
 
        const auto l_moveData = *l_moveITR;
 
        l_gameObjectList.erase(l_moveITR);
 
        const auto& l_newTargetITR = std::ranges::find(l_gameObjectList, l_targetGameObject);
 
        l_gameObjectList.insert(a_isMoveAfter ? std::next(l_newTargetITR) : l_newTargetITR, l_moveData);
    }
 
    // 階層が変わり得るため実行レベルを再構築する
    a_scene.RebuildGameObjectExecutionLevelList();

    // 親や兄弟の並びが変わっていればUndoRedo履歴へ登録する
    PushReparentGameObjectCommand(l_moveGameObject, a_scene, l_beforeState);
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::CopySelectedGameObjects(const EditorGameObjectSelectionState& a_gameObjectSelectionState, WorldOutlinerEditorWindowClipboard& a_clipboard) const
{
    // 選択中のGameObjectのSceneInstanceUUIDを集める
    const auto& l_selectedList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();
 
    std::vector<boost::uuids::uuid> l_uuidList = {};
 
    // 選択されているゲームオブジェクト数分UUIDListのサイズを確保しておく
    l_uuidList.reserve(l_selectedList.size());
 
    for (const auto& l_gameObjectWeak : l_selectedList)
    {
        const auto& l_gameObject = l_gameObjectWeak.lock();
 
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 親子同時選択時の重複コピーを回避する
        // 選択リスト内に自身の祖先タイル場合、祖先のCloneにこのGameObjectも含まれているためスキップ
        if (Utility::HasAncestorInList(l_selectedList, l_gameObjectWeak)) { continue; }

        l_uuidList.emplace_back(l_gameObject->GetREFSceneInstanceUUID());
    }
 
    if (l_uuidList.empty()) { return; }
 
    // ClipboardへCopy種別で格納
    a_clipboard.Apply(l_uuidList, Enum::WorldOutlinerClipboardOperationType::Copy);
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::CutSelectedGameObjects(WorldOutlinerEditorWindowClipboard& a_clipboard, EditorGameObjectSelectionState& a_gameObjectSelectionState) const
{
    // 選択中のGameObjectのSceneInstanceUUIDを集める
    const auto& l_selectedList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();
 
    std::vector<boost::uuids::uuid> l_uuidList = {};
 
    // 選択されているゲームオブジェクト数分UUIDListのサイズを確保しておく
    l_uuidList.reserve(l_selectedList.size());
 
    for (const auto& l_gameObjectWeak : l_selectedList)
    {
        const auto& l_gameObject = l_gameObjectWeak.lock();
 
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 親子同時選択時の重複コピーを回避する
        // 選択リスト内に自身の祖先タイル場合、祖先のCloneにこのGameObjectも含まれているためスキップ
        if (Utility::HasAncestorInList(l_selectedList, l_gameObjectWeak)) { continue; }

        l_uuidList.emplace_back(l_gameObject->GetREFSceneInstanceUUID());
    }
 
    if (l_uuidList.empty()) { return; }
 
    // ClipboardへCut種別で格納
    // CutはPaste時に元を破棄するため、ここでは破棄しない
    a_clipboard.Apply(l_uuidList, Enum::WorldOutlinerClipboardOperationType::Cut);
 
    // CutしたGameObjectの選択を解除する
    // Paste時に別の場所へ移動した体になるため元の選択は意味を成さない
    a_gameObjectSelectionState.ClearSelectedGameObjectList();
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::DuplicateSelectedGameObjects(EditorGameObjectSelectionState& a_gameObjectSelectionState, Scene& a_scene) const
{
    const auto& l_selectedList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();
 
    if (l_selectedList.empty()) { return; }
 
    // 複製したGameObjectを選択状態にするためのリスト
    std::vector<std::weak_ptr<GameObject>> l_duplicatedList = {};
 
    // 複製するゲームオブジェクトリストを選択されているゲームオブジェクト数分確保しておく
    l_duplicatedList.reserve(l_selectedList.size());
 
    for (const auto& l_gameObjectWeak : l_selectedList)
    {
        const auto& l_gameObject = l_gameObjectWeak.lock();
 
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 親子同時選択時の重複Clone回避
        // 選択リスト内に自分の祖先がいる場合、祖先のCloneにこのGameObjectも含まれるためスキップ
        if (Utility::HasAncestorInList(l_selectedList, l_gameObjectWeak)) { continue; }
 
        // 元のGameObjectと同じ親の末尾にCloneを追加する
        // ルートならa_newParentを{}にしてルートリスト末尾へ追加する
        const auto& l_hierarchy = l_gameObject->GetREFHierarchy();
        const auto& l_parent    = l_hierarchy.GetREFParent     ();
 
        const auto& l_clone = l_gameObject->Clone(l_parent, a_scene);
 
        if (!l_clone) { continue; }
 
        l_duplicatedList.emplace_back(l_clone);
    }
 
    if (l_duplicatedList.empty()) { return; }
 
    // 複製したGameObjectを選択状態にする
    // 元の選択をクリアしてから複製したGameObjectを選択する
    a_gameObjectSelectionState.ClearSelectedGameObjectList();
 
    for (const auto& l_duplicated : l_duplicatedList)
    {
        a_gameObjectSelectionState.AddSelectedGameObject(l_duplicated);
    }

    // 複製を生成としてUndoRedo履歴へ登録する
    // 複製ごとに親が異なっていてもコマンド側がUndo時にそれぞれの親を記録する
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<CreateGameObjectCommand>(l_duplicatedList);
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::PasteGameObjects(WorldOutlinerEditorWindowClipboard& a_clipboard, EditorGameObjectSelectionState& a_gameObjectSelectionState, Scene& a_scene) const
{
     // クリップボードにゲームオブジェクトのUUIDがコピーされていなければreturn
    if (a_clipboard.IsEmpty()) { return; }
 
    const auto& l_uuidList      = a_clipboard.GetREFGameObjectUUIDList();
    const auto  l_operationType = a_clipboard.GetVALOperationType     ();
 
    // PasteしたGameObjectを選択状態にするためのリスト
    std::vector<std::weak_ptr<GameObject>> l_pastedList = {};
 
    // UUIDListのサイズ分貼り付け用ゲームオブジェクトリストを予約
    l_pastedList.reserve(l_uuidList.size());

    // Cut操作で取り外した元のGameObjectの記録
    std::vector<Struct::DestroyedGameObjectRecord> l_cutSourceRecordList = {};
 
    for (const auto& l_uuid : l_uuidList)
    {
        // ClipboardのUUIDはSceneInstanceUUID
        const auto& l_sourceGameObject = a_scene.FindVALGameObject(l_uuid).lock();
 
        if (!l_sourceGameObject ||
            l_sourceGameObject->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 常にシーンのルートとしてCloneする
        // 選択中のGameObjectがあっても親にはしない
        // これにより自分自身を親にする循環参照を確実に防ぐ
        const auto& l_clone = l_sourceGameObject->Clone({}, a_scene);
 
        if (!l_clone) { continue; }
 
        l_pastedList.emplace_back(l_clone);
 
        // Cut操作なら元のGameObjectをシーンから取り外す
        // Undoで戻せるよう破棄はせず、Clone後に取り外すためCloneには影響しない
        if (l_operationType == Enum::WorldOutlinerClipboardOperationType::Cut)
        {
            auto l_record = FetchVALDestroyedGameObjectRecord(l_sourceGameObject);

            Utility::DetachGameObjectSubtree(l_record.m_detachedSubtreeGameObjectList, a_scene);

            l_cutSourceRecordList.emplace_back(std::move(l_record));
        }
    }
 
    // Cut操作の場合はClipboardをクリアする
    // Copy操作の場合は繰り返しPasteできるようクリアしない
    if (l_operationType == Enum::WorldOutlinerClipboardOperationType::Cut)
    {
        a_clipboard.Clear();
    }
 
    if (l_pastedList.empty()) { return; }
 
    // PasteしたGameObjectを選択状態にする
    a_gameObjectSelectionState.ClearSelectedGameObjectList();
 
    for (const auto& l_pasted : l_pastedList)
    {
        a_gameObjectSelectionState.AddSelectedGameObject(l_pasted);
    }

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    // Cut操作の貼り付けは「複製の生成」と「元の取り外し」を1回のUndoで戻せるようまとめて登録する
    if (l_operationType == Enum::WorldOutlinerClipboardOperationType::Cut)
    {
        l_undoRedoSystem.PushUndoCommand<PasteCutGameObjectCommand>(l_pastedList, std::move(l_cutSourceRecordList));

        return;
    }

    // Copy操作の貼り付けを生成としてUndoRedo履歴へ登録する
    l_undoRedoSystem.PushUndoCommand<CreateGameObjectCommand>(l_pastedList);
}

FWK::Struct::DestroyedGameObjectRecord FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::FetchVALDestroyedGameObjectRecord(const std::weak_ptr<GameObject>& a_gameObject) const
{
    Struct::DestroyedGameObjectRecord l_record = {};

    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return l_record; }

    // 先頭を取り外すルート、その後ろに子孫を親→子の順で並べる
    auto& l_subtreeList = l_record.m_detachedSubtreeGameObjectList;

    l_subtreeList.emplace_back(l_gameObject);

    Utility::CollectDescendantGameObjectList(l_gameObject, l_subtreeList);

    // 親がいなければnilのまま(ルート)
    const auto& l_hierarchy = l_gameObject->GetREFHierarchy();

    if (const auto& l_parent = l_hierarchy.GetREFParent().lock();
        l_parent)
    {
        l_record.m_parentUUID = l_parent->GetREFSceneInstanceUUID();
    }

    return l_record;
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::PushReparentGameObjectCommand(const std::weak_ptr<GameObject>& a_gameObject, const Scene& a_scene, const Struct::ReparentGameObjectState& a_beforeState) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // 変更後の親と兄弟の並びを記録する
    const auto& l_afterState = Utility::FetchVALReparentGameObjectState(l_gameObject, a_scene);

    // 親も兄弟の並びも変わらなかった場合はコマンドをPushしない
    if (a_beforeState.m_parentUUID      == l_afterState.m_parentUUID      &&
        a_beforeState.m_siblingUUIDList == l_afterState.m_siblingUUIDList)
    {
        return;
    }

    // EditorManager経由でUndoRedoSystemへコマンドをPushする
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<ReparentGameObjectCommand>(l_gameObject->GetREFSceneInstanceUUID(), a_beforeState, l_afterState);
}

FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::ChildGameObjectDataList::iterator FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::FindChildGameObjectITR(const std::weak_ptr<GameObject>& a_gameObject, ChildGameObjectDataList& a_childDataList) const
{
   // 同じアドレスを持つゲームオブジェクトのイテレータ位置を返す
   return std::find_if(a_childDataList.begin(), a_childDataList.end(),
                      [&a_gameObject](const auto& a_childData)
                      {
                          return a_childData.m_type.lock() == a_gameObject.lock();
                      });
}