#include "WorldOutlinerEditorWindowSelectSceneCommand.h"

// 選択されたかどうかのコマンドのRedo,Undoが発生するのは選択されたときだけ
// なのでUndoはtrue,Redoはfalseで固定
FWK::Editor::WorldOutlinerEditorWindowSelectSceneCommand::WorldOutlinerEditorWindowSelectSceneCommand(const std::weak_ptr<WorldOutlinerEditorWindowSelectionState>& a_selectionState) :
    m_selectionState(a_selectionState),
{}
FWK::Editor::WorldOutlinerEditorWindowSelectSceneCommand::~WorldOutlinerEditorWindowSelectSceneCommand() = default;

void FWK::Editor::WorldOutlinerEditorWindowSelectSceneCommand::Undo()
{
    const auto& l_selectionState = m_selectionState.lock();

    if (!l_selectionState) { return; }

    l_selectionState->SetIsSceneSelected(false);
}
void FWK::Editor::WorldOutlinerEditorWindowSelectSceneCommand::Redo()
{
    const auto& l_selectionState = m_selectionState.lock();

    if (!l_selectionState) { return; }

    l_selectionState->SetIsSceneSelected(true);
}