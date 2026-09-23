#include "WorldOutlinerEditorWindowSelectionState.h"

void FWK::Editor::WorldOutlinerEditorWindowSelectionState::SelectSingleGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    // 無効なGameObjectへの選択は受け付けない
    if (!l_gameObject) { return; }

    // 選択を単一GameObjectへ置き換え
    m_selectedGameObjectUUIDList.clear       ();
    m_selectedGameObjectUUIDList.emplace_back(l_gameObject->GetREFSceneInstanceUUID());
    m_isSceneSelected = false;
}
void FWK::Editor::WorldOutlinerEditorWindowSelectionState::SelectSingleScene()
{
    // Scene選択時はGameObject選択はすべて解除される
    m_selectedGameObjectUUIDList.clear();

    m_isSceneSelected = true;
}

void FWK::Editor::WorldOutlinerEditorWindowSelectionState::ClearSelection()
{
    m_selectedGameObjectUUIDList.clear();
}