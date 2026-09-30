#include "WorldOutlinerEditorWindowShortcutHandler.h"

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::Handle(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    // 矢印キーやリネーム用ショートカットの処理を行う
    HandleSelectAllGameObject(a_editorWindow, a_editorManager);
    HandleArrowKey           (a_editorWindow, a_editorManager);
    HandleRename             (a_editorWindow, a_editorManager);
}

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleSelectAllGameObject(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    const auto& l_io = ImGui::GetIO();

    // Ctrl + Aでシーン内の全GameObjectを選択する
    // Sceneは選択対象に含めない
    if (l_io.KeyCtrl &&
        !l_io.KeyShift &&
        ImGui::IsKeyPressed(ImGuiKey_A))
    {
        a_editorWindow.SelectAllGameObjects(a_editorManager);
    }
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleArrowKey(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    const auto& l_io = ImGui::GetIO();
 
    // ShiftかCtrlを押しながらの上下キーは範囲選択
    const bool l_isRangeSelection = l_io.KeyShift || l_io.KeyCtrl;
 
    // ImGui::IsKeyPressedは押された瞬間trueを返す
    // repeatはデフォルトtrueのため押し続ければ連続で移動する
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        a_editorWindow.MoveSelectionUp(a_editorManager, l_isRangeSelection);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        a_editorWindow.MoveSelectionDown(a_editorManager, l_isRangeSelection);
    }
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleRename(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    // F2で選択中のGameObject、またはSceneの名前変更を開始する
    if (!ImGui::IsKeyPressed(ImGuiKey_F2)) { return; }

    a_editorWindow.StartRenameByCurrentSelection(a_editorManager);
}