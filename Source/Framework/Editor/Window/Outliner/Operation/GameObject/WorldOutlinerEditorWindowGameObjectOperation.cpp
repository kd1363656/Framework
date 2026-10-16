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

    // 同じ名前のままなら履歴に積まない
    const auto l_beforeName = l_gameObject->GetREFName();

    if (l_beforeName == a_newName) { return; }

    l_gameObject->SetName(a_newName);

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<RenameGameObjectCommand>(l_beforeName, a_newName, l_gameObject->GetREFSceneInstanceUUID());
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::DestroySelectedGameObjects(EditorGameObjectSelectionState& a_gameObjectSelectionState) const
{
    const auto& l_selectedList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();

    std::vector<std::weak_ptr<GameObject>> l_destroyList = {};

    l_destroyList.reserve(l_selectedList.size());

    for (const auto& l_selectedWeak : l_selectedList)
    {
        const auto& l_selected = l_selectedWeak.lock();

        if (!l_selected ||
            l_selected->GetVALIsDestroyed())
        {
            continue;
        }

        // 親子を同時選択していた場合は親だけを対象にする
        // 親をシーンから外すと子孫も一緒に外れるため
        if (Utility::HasAncestorInList(l_selectedList, l_selectedWeak)) { continue; }

        l_destroyList.emplace_back(l_selectedWeak);
    }

    if (l_destroyList.empty()) { return; }

    // 削除したGameObjectの選択状態を先に解除する
    a_gameObjectSelectionState.ClearSelectedGameObjectList();

    // 削除前の親と並び順を記録してからシーンから外す
    // 破棄フラグは立てずにコマンドが実体を持ち続けるため、Ctrl + Zで元の場所へ戻せる
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.ExecuteUndoCommand<DestroyGameObjectCommand>(l_destroyList);
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
 
    // Undo用に変更前の親と兄弟の並び順を記録する
    const auto& l_beforePlacement = Utility::FetchVALGameObjectPlacement(l_moveGameObject, a_scene);

    // 自分自身・子孫への親付けガード、旧親からの除去、
    // TransformのHierarchical化はApplyParent側で行われる
    if (auto& l_moveGameObjectHierarchy = l_moveGameObject->GetMutableREFHierarchy();
        !l_moveGameObjectHierarchy.ApplyParent(a_targetGameObject))
    {
        return;
    }

    // 階層の深さが変わったため実行レベルを再構築する
    a_scene.RebuildGameObjectExecutionLevelList();

    PushReparentCommand(l_moveGameObject, a_scene, l_beforePlacement);
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

    // Undo用に変更前の親と兄弟の並び順を記録する
    const auto& l_beforePlacement = Utility::FetchVALGameObjectPlacement(l_moveGameObject, a_scene);

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

        // ドロップした対象のリストの位置を取得
        if (const auto& l_moveITR = FindChildGameObjectITR(a_moveGameObject, l_childDataList);
            l_moveITR != l_childDataList.end())
        {
            // eraseするとイテレータが無効化されるため
            // 一度要素を取り出し、ターゲット位置を探し直す
            const auto l_moveData = *l_moveITR;

            l_childDataList.erase(l_moveITR);

            // a_isMoveAfter = trueでターゲットの後、そうでないならターゲットの前
            if (const auto& l_targetITR = FindChildGameObjectITR(a_targetGameObject, l_childDataList);
                l_targetITR != l_childDataList.end())
            {
                l_childDataList.insert(a_isMoveAfter ? std::next(l_targetITR) : l_targetITR, l_moveData);
            }
            else
            {
                // ターゲットが見つからない場合は取り出した要素を末尾へ戻して失わないようにする
                l_childDataList.emplace_back(l_moveData);
            }
        }
    }
    // 親がいなければルートリストの方の並びを変更する
    else
    {
        auto& l_gameObjectList = a_scene.GetMutableREFGameObjectList();

        // ドロップしたものがまだ保留中の追加リストにある場合はルートリストに無いため並べ替えない
        if (const auto& l_moveITR = std::ranges::find(l_gameObjectList, l_moveGameObject);
            l_moveITR != l_gameObjectList.end())
        {
            const auto l_moveData = *l_moveITR;

            l_gameObjectList.erase(l_moveITR);

            if (const auto& l_targetITR = std::ranges::find(l_gameObjectList, l_targetGameObject);
                l_targetITR != l_gameObjectList.end())
            {
                l_gameObjectList.insert(a_isMoveAfter ? std::next(l_targetITR) : l_targetITR, l_moveData);
            }
            else
            {
                l_gameObjectList.emplace_back(l_moveData);
            }
        }
    }

    // 階層が変わり得るため実行レベルを再構築する
    a_scene.RebuildGameObjectExecutionLevelList();

    PushReparentCommand(l_moveGameObject, a_scene, l_beforePlacement);
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

    // 複製はGameObjectの作成としてUndoRedo履歴へ登録する
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
 
    // PasteしたGameObjectを選択状態にするためのリストと、
    // 切り取りの場合にシーンから外す元のGameObjectのリスト
    std::vector<std::weak_ptr<GameObject>> l_pastedList = {};
    std::vector<std::weak_ptr<GameObject>> l_cutList    = {};

    // UUIDListのサイズ分貼り付け用ゲームオブジェクトリストを予約
    l_pastedList.reserve(l_uuidList.size());
    l_cutList.reserve   (l_uuidList.size());

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
 
        // Cut操作なら元のGameObjectは後でまとめてシーンから外す
        // 破棄フラグを立てるとUndoで戻せないため、ここでは集めるだけにする
        if (l_operationType == Enum::WorldOutlinerClipboardOperationType::Cut)
        {
            l_cutList.emplace_back(l_sourceGameObject);
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

    // コピーの貼り付けはGameObjectの作成として登録する
    if (l_cutList.empty())
    {
        l_undoRedoSystem.PushUndoCommand<CreateGameObjectCommand>(l_pastedList);

        return;
    }

    // 切り取りの貼り付けは、元をシーンから外す処理までをコマンドに任せて
    // 「作成」と「削除」を1回のUndoで戻せるようにする
    l_undoRedoSystem.ExecuteUndoCommand<PasteCutGameObjectCommand>(l_pastedList, l_cutList);
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::PushReparentCommand(const std::shared_ptr<GameObject>& a_moveGameObject, const Scene& a_scene, const Struct::GameObjectPlacement& a_beforePlacement) const
{
    // 変更後の親と兄弟の並び順を記録する
    const auto& l_afterPlacement = Utility::FetchVALGameObjectPlacement(a_moveGameObject, a_scene);

    // 親も並び順も変わっていなければ履歴に積まない
    // 例 : 同じ場所へドロップし直しただけの場合
    if (l_afterPlacement == a_beforePlacement) { return; }

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<ReparentGameObjectCommand>(a_moveGameObject->GetREFSceneInstanceUUID(), a_beforePlacement, l_afterPlacement);
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