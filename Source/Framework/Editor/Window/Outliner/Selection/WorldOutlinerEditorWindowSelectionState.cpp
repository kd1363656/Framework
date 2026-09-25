#include "WorldOutlinerEditorWindowSelectionState.h"

void FWK::Editor::WorldOutlinerEditorWindowSelectionState::SelectSingleGameObject()
{
    // TODO
    // 選択を単一GameObjectへ置き換え
    m_selectedGameObjectUUIDList.clear();

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