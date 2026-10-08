#include "WorldOutlinerEditorWindowShortcutHandler.h"

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::Handle(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    // 全てのゲームオブジェクトを選択する
    const auto& l_io = ImGui::GetIO();

    // Ctrl + Aでシーン内の全GameObjectを選択する
    // Sceneは選択対象に含めない
    if (l_io.KeyCtrl   &&
        !l_io.KeyShift &&
        ImGui::IsKeyPressed(ImGuiKey_A))
    {
        HandleSelectAllGameObject(a_editorWindow, a_editorManager);
    }

    // ShiftかCtrlを押しながらの上下キーは範囲選択
    // そうでないなら選択位置の移動
    HandleArrowKey(l_io.KeyShift ||
                   l_io.KeyCtrl,
                   a_editorWindow,
                   a_editorManager);

    // F2で選択中のGameObject、またはSceneの名前変更を開始する
    if (ImGui::IsKeyPressed(ImGuiKey_F2))
    {
        HandleRename(a_editorWindow, a_editorManager);
    }

    // Deleteキーで選択中のGameObjectを全て破棄する
    // Scene選択は対象外(GameObjectのみが削除対象)
    if (ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        HandleDeleteGameObject(a_editorWindow, a_editorManager);
    }

    // 基本的なショートカットキー処理
    // Ctrl + Cで選択中のGameObjectをコピーする
    if (l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_C))
    {
        HandleCopy(a_editorWindow, a_editorManager);
    }

    // Ctrl + Xで選択中のGameObjectを切り取りする
    if (l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_X))
    {
        HandleCut(a_editorWindow, a_editorManager);
    }

    // Ctrl + Dで選択中のGameObjectを複製する
    if (l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_D))
    {
        HandleDuplicate(a_editorWindow, a_editorManager);
    }

    // Ctrl + VでクリップボードのGameObjectを張り付ける
    if (l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_V))
    {
        HandlePaste(a_editorWindow, a_editorManager);
    }
}

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleSelectAllGameObject(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    a_editorWindow.SelectAllGameObjects(a_editorManager);
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleArrowKey(const bool a_isRangeSelection, WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    // ImGui::IsKeyPressedは押された瞬間trueを返す
    // repeatはデフォルトtrueのため押し続ければ連続で移動する
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        a_editorWindow.MoveSelectionUp(a_editorManager, a_isRangeSelection);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        a_editorWindow.MoveSelectionDown(a_editorManager, a_isRangeSelection);
    }
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleRename(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    a_editorWindow.StartRenameByCurrentSelection(a_editorManager);
}

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleDeleteGameObject(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation             ();

    l_gameObjectOperation.DestroySelectedGameObjects(l_gameObjectSelectionState, *l_scene);
}

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleCopy(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    const auto& l_gameObjectSelectionState = a_editorManager.GetREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation      ();

    l_gameObjectOperation.CopySelectedGameObjects(l_gameObjectSelectionState, a_editorWindow.GetMutableREFClipboard());
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleCut(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation             ();

    l_gameObjectOperation.CutSelectedGameObjects(a_editorWindow.GetMutableREFClipboard(), l_gameObjectSelectionState);
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleDuplicate(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation             ();

    l_gameObjectOperation.DuplicateSelectedGameObjects(l_gameObjectSelectionState, *l_scene);
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandlePaste(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation             ();

    l_gameObjectOperation.PasteGameObjects(a_editorWindow.GetMutableREFClipboard(), l_gameObjectSelectionState, *l_scene);
}