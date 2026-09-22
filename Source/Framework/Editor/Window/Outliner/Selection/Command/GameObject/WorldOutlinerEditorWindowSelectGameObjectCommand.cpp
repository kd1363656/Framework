#include "WorldOutlinerEditorWindowSelectGameObjectCommand.h"

FWK::Editor::WorldOutlinerEditorWindowSelectGameObjectCommand::WorldOutlinerEditorWindowSelectGameObjectCommand(const std::vector<boost::uuids::uuid>& a_prevSelectedGameObjectUUIDList, const std::vector<boost::uuids::uuid> a_nextSelectedGameObjectUUIDList, const std::weak_ptr<WorldOutlinerEditorWindowSelectionState>& a_selectionState) :
    m_selectionState(a_selectionState),

    m_prevSelectedGameObjectUUIDList(a_prevSelectedGameObjectUUIDList),
    m_nextSelectedGameObjectUUIDList(a_nextSelectedGameObjectUUIDList)
{}
FWK::Editor::WorldOutlinerEditorWindowSelectGameObjectCommand::~WorldOutlinerEditorWindowSelectGameObjectCommand() = default;

void FWK::Editor::WorldOutlinerEditorWindowSelectGameObjectCommand::Undo()
{
    ApplySelectedGameObjectUUIDListHistory(m_prevSelectedGameObjectUUIDList);
}
void FWK::Editor::WorldOutlinerEditorWindowSelectGameObjectCommand::Redo()
{
    ApplySelectedGameObjectUUIDListHistory(m_nextSelectedGameObjectUUIDList);
}

void FWK::Editor::WorldOutlinerEditorWindowSelectGameObjectCommand::ApplySelectedGameObjectUUIDListHistory(const std::vector<boost::uuids::uuid>& a_selectedGameObjectUUIDList)
{
    if (a_selectedGameObjectUUIDList.empty()) { return; }

    const auto& l_selectionState = m_selectionState.lock();

    if (!l_selectionState) { return; }

    l_selectionState->SetSelectedGameObjectUUIDList(a_selectedGameObjectUUIDList);
}