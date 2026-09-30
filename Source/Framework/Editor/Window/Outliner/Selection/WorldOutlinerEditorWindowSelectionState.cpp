#include "WorldOutlinerEditorWindowSelectionState.h"

void FWK::Editor::WorldOutlinerEditorWindowSelectionState::SelectSingleScene(EditorGameObjectSelectionState& a_gameObjectSelectionState)
{
    m_isSceneSelected = true;

    // 通常クリックによるScene選択はGameObjectの選択を排他する
    // Ctrl + クリックによる混在選択はToggleSceneSelect側で扱う
    a_gameObjectSelectionState.ClearSelectedGameObjectList();
}

void FWK::Editor::WorldOutlinerEditorWindowSelectionState::ToggleSceneSelect(EditorGameObjectSelectionState& a_gameObjectSelectionState)
{
    m_isSceneSelected = m_isSceneSelected ? false : true;

    // Scene側を選択状態にした場合はGameObjectの選択を解除する
    // (SceneかGameObjectのどちらか一方のみ選択できる仕様のため)
    if (m_isSceneSelected)
    {
        a_gameObjectSelectionState.ClearSelectedGameObjectList();
    }
}