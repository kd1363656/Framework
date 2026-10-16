#include "WorldOutlinerEditorWindowSceneOperation.h"

void FWK::Editor::WorldOutlinerEditorWindowSceneOperation::RenameScene(const std::string& a_newName, Scene& a_scene) const
{
    // 空文字列への変更は行わない
    if (a_newName.empty()) { return; }

    // 同じ名前のままなら履歴に積まない
    const auto l_beforeName = a_scene.GetREFName();

    if (l_beforeName == a_newName) { return; }

    a_scene.SetName(a_newName);

    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<RenameSceneCommand>(l_beforeName, a_newName);
}