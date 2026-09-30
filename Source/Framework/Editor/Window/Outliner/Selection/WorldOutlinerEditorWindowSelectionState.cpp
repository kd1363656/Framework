#include "WorldOutlinerEditorWindowSelectionState.h"

void FWK::Editor::WorldOutlinerEditorWindowSelectionState::SelectSingleScene(EditorGameObjectSelectionState& a_gameObjectSelectionState)
{
    m_isSceneSelected = true;

    // 通常クリックによるScene選択はGameObjectの選択を排他する
    // Ctrl + クリックによる混在選択はToggleSceneSelect側で扱う
    a_gameObjectSelectionState.ClearSelectedGameObjectList();
}

void FWK::Editor::WorldOutlinerEditorWindowSelectionState::ToggleSceneSelect()
{
    // Ctrl + クリック時の選択 / 選択解除切替
    // GameObject選択は維持されるためSceneとGameObjectの混在選択が可能になる
    m_isSceneSelected = m_isSceneSelected ? false : true;
}