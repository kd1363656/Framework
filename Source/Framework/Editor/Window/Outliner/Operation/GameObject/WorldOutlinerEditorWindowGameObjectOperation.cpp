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
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::DestroySelectedGameObjects(EditorGameObjectSelectionState& a_gameObjectSelectionState) const
{
    // 選択リストは先にコピーする
    // 破棄処理中に選択リスト側が変化しても安全にするため
    const auto l_targetList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();
 
    for (const auto& l_targetWeak : l_targetList)
    {
        const auto& l_target = l_targetWeak.lock();
 
        // 親子を同時選択していた場合、親側の再帰で既に破棄済みならスキップ
        if (!l_target ||
            l_target->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 再帰的に子ゲームオブジェクトも削除する
        DestroyGameObjectRecursive(l_target);
    }
 
    // 破棄したGameObjectの選択状態を解除する
    a_gameObjectSelectionState.ClearSelectedGameObjectList();
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::ReparentGameObject(const std::weak_ptr<GameObject>& a_targetGameObject, const std::weak_ptr<GameObject>& a_moveGameObject, Scene& a_scene)
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
 
    // 自分自身・子孫への親付けガード、旧親からの除去、
    // TransformのHierarchical化はApplyParent側で行われる
    if (auto& l_moveGameObjectHierarchy = l_moveGameObject->GetMutableREFHierarchy();
        !l_moveGameObjectHierarchy.ApplyParent(a_targetGameObject)) 
    {
        return; 
    }
 
    // 階層の深さが変わったため実行レベルを再構築する
    a_scene.RebuildGameObjectExecutionLevelList();
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::MoveGameObjectSiblingOrder(const std::weak_ptr<GameObject>& a_targetGameObject,
                                                                                           const std::weak_ptr<GameObject>& a_moveGameObject,
                                                                                                 Scene&                     a_scene, 
                                                                                           const bool                       a_isMoveAfter) const
{
    const auto& l_targetGameObject = a_targetGameObject.lock();
    const auto& l_moveGameObject   = a_moveGameObject.lock  ();

    if (!l_targetGameObject || 
        !l_moveGameObject   ||
        l_targetGameObject == l_moveGameObject)
    {
        return;
    }
 
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
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::DestroyGameObjectRecursive(const std::weak_ptr<GameObject>& a_gameObject) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // 自身を破棄
    // 実際のScene除去はScene::RemoveDestroyedGameObjects(EarlyUpdate)が行う
    l_gameObject->Destroy();
 
    // 子孫も全て破棄する
    // 親だけ消して子を残すと親参照切れの孤児GameObjectが残るため
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
 
        DestroyGameObjectRecursive(l_child);
    }
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