#include "WorldOutlinerEditorWindowSceneOperation.h"

void FWK::Editor::WorldOutlinerEditorWindowSceneOperation::RenameScene(const std::string& a_newName, Scene& a_scene) const
{
    // 空文字列への変更は行わない
    if (a_newName.empty()) { return; }

    a_scene.SetName(a_newName);
}