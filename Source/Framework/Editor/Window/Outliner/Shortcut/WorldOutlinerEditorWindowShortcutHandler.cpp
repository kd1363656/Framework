#include "WorldOutlinerEditorWindowShortcutHandler.h"

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::Handle(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    // このフレームで押されたショートカットを取得する
    // キーの割り当てと「押しっぱなしでは繰り返さない」動きはAssetBrowserと共通(EditorWindowShortcutUtilityConstant.h)
    switch (Utility::FetchVALPressedEditorWindowShortcut())
    {
    case Enum::EditorWindowShortcutType::SelectAll:
    {
        // Ctrl + A : シーン内の全GameObjectを選択する(Sceneは選択対象に含めない)
        HandleSelectAllGameObject(a_editorWindow, a_editorManager);

        break;
    }
    case Enum::EditorWindowShortcutType::Rename:
    {
        // F2 : 選択中のGameObject、又はSceneの名前変更を開始する
        HandleRename(a_editorWindow, a_editorManager);

        break;
    }
    case Enum::EditorWindowShortcutType::Delete:
    {
        // Delete : 選択中のGameObjectを全て破棄する(Scene選択は対象外)
        HandleDeleteGameObject(a_editorWindow, a_editorManager);

        break;
    }
    case Enum::EditorWindowShortcutType::Copy:
    {
        // Ctrl + C : 選択中のGameObjectをコピーする
        HandleCopy(a_editorWindow, a_editorManager);

        break;
    }
    case Enum::EditorWindowShortcutType::Cut:
    {
        // Ctrl + X : 選択中のGameObjectを切り取りする
        HandleCut(a_editorWindow, a_editorManager);

        break;
    }
    case Enum::EditorWindowShortcutType::Duplicate:
    {
        // Ctrl + D : 選択中のGameObjectを複製する
        HandleDuplicate(a_editorWindow, a_editorManager);

        break;
    }
    case Enum::EditorWindowShortcutType::Paste:
    {
        // Ctrl + V : クリップボードのGameObjectを張り付ける
        HandlePaste(a_editorWindow, a_editorManager);

        break;
    }
    default:
    {
        // CreateFolderなどWorldOutlinerで使わないショートカットは何もしない
        break;
    }
    }

    // Shift又はCtrlを押しながらの上下キーは範囲選択(AssetBrowserと共通)
    // そうでないなら選択位置の移動
    HandleArrowKey(a_editorWindow, a_editorManager, Utility::IsEditorWindowRangeSelectionKeyDown());
}

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleSelectAllGameObject(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    a_editorWindow.SelectAllGameObjects(a_editorManager);
}
void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleArrowKey(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager, const bool a_isRangeSelection) const
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
          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation             ();

    l_gameObjectOperation.DestroySelectedGameObjects(l_gameObjectSelectionState);
}

void FWK::Editor::WorldOutlinerEditorWindowShortcutHandler::HandleCopy(WorldOutlinerEditorWindow& a_editorWindow, EditorManager& a_editorManager) const
{
    const auto& l_gameObjectSelectionState = a_editorManager.GetREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation     ();

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