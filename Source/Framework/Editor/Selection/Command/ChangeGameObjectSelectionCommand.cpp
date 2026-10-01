#include "ChangeGameObjectSelectionCommand.h"

FWK::Editor::ChangeGameObjectSelectionCommand::ChangeGameObjectSelectionCommand(std::vector<boost::uuids::uuid>&& a_beforeUUIDList, 
                                                                                std::vector<boost::uuids::uuid>&& a_afterUUIDList, 
                                                                                boost::uuids::uuid                a_beforeAnchorUUID, 
                                                                                boost::uuids::uuid                a_afterAnchorUUID) :
    m_beforeUUIDList  (std::move(a_beforeUUIDList)),
    m_afterUUIDList   (std::move(a_afterUUIDList)),
    m_beforeAnchorUUID(std::move(a_beforeAnchorUUID)),
    m_afterAnchorUUID (std::move(a_afterAnchorUUID))
{}
FWK::Editor::ChangeGameObjectSelectionCommand::~ChangeGameObjectSelectionCommand() = default;

void FWK::Editor::ChangeGameObjectSelectionCommand::Undo()
{
    // 変更前の選択状態へ戻す
    // EditorManagerが統括管理するSelectionStateへアクセスする
    auto& l_editorManager  = EditorManager::GetInstance                           ();
    auto& l_selectionState = l_editorManager.GetMutableREFGameObjectSelectionState();

    l_selectionState.RestoreState(m_beforeUUIDList, m_beforeAnchorUUID);
}

void FWK::Editor::ChangeGameObjectSelectionCommand::Redo()
{
    // 変更後の選択状態へ進める
    // EditorManagerが統括管理するSelectionStateへアクセスする
    auto& l_editorManager  = EditorManager::GetInstance();
    auto& l_selectionState = l_editorManager.GetMutableREFGameObjectSelectionState();
 
    l_selectionState.RestoreState(m_afterUUIDList, m_afterAnchorUUID);
}