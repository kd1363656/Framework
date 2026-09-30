#include "WorldOutlinerEditorWindowGameObjectOperation.h"

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

void FWK::Editor::WorldOutlinerEditorWindowGameObjectOperation::DestroyGameObjectRecursive(const std::shared_ptr<GameObject>& a_gameObject) const
{
    // 自身を破棄
    // 実際のScene除去はScene::RemoveDestroyedGameObjects(EarlyUpdate)が行う
    a_gameObject->Destroy();
 
    // 子孫も全て破棄する
    // 親だけ消して子を残すと親参照切れの孤児GameObjectが残るため
    const auto& l_childDataList = a_gameObject->GetREFHierarchy().GetREFChildSmartPointerVectorList().GetREFElementDataList();
 
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