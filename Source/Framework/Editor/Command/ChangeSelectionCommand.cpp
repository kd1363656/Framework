#include "ChangeSelectionCommand.h"

FWK::Editor::ChangeSelectionCommand::ChangeSelectionCommand(const boost::uuids::uuid&               a_beforeAnchorUUID,
                                                            const boost::uuids::uuid&               a_afterAnchorUUID,
                                                            const bool                              a_beforeIsSceneSelected,
                                                            const bool                              a_afterIsSceneSelected,
                                                                  std::vector<boost::uuids::uuid>&& a_beforeUUIDList,
                                                                  std::vector<boost::uuids::uuid>&& a_afterUUIDList) :
    m_beforeUUIDList(std::move(a_beforeUUIDList)),
    m_afterUUIDList (std::move(a_afterUUIDList)),

    m_beforeAnchorUUID(a_beforeAnchorUUID),
    m_afterAnchorUUID (a_afterAnchorUUID),

    m_beforeIsSceneSelected(a_beforeIsSceneSelected),
    m_afterIsSceneSelected (a_afterIsSceneSelected)
{}
FWK::Editor::ChangeSelectionCommand::~ChangeSelectionCommand() = default;

void FWK::Editor::ChangeSelectionCommand::Undo()
{
    // 変更前の選択状態へ戻す
    auto& l_editorManager  = EditorManager::GetInstance                           ();
    auto& l_selectionState = l_editorManager.GetMutableREFGameObjectSelectionState();

    l_selectionState.RestoreState(m_beforeUUIDList, m_beforeAnchorUUID);

    // Scene選択状態はWorldOutlinerEditorWindowが保持しているため
    // EditorManager経由でウィンドウを取得して復元する
    if (const auto& l_outlinerWindow = l_editorManager.FindVALWindowEditor<WorldOutlinerEditorWindow>().lock();
        l_outlinerWindow)
    {
        auto& l_sceneSelectionState = l_outlinerWindow->GetMutableREFSceneSelectionState();

        l_sceneSelectionState.SetIsSceneSelected(m_beforeIsSceneSelected);
    }
}

void FWK::Editor::ChangeSelectionCommand::Redo()
{
    // 変更後の選択状態へ進める
    auto& l_editorManager  = EditorManager::GetInstance                           ();
    auto& l_selectionState = l_editorManager.GetMutableREFGameObjectSelectionState();

    l_selectionState.RestoreState(m_afterUUIDList, m_afterAnchorUUID);

    if (const auto& l_outlinerWindow = l_editorManager.FindVALWindowEditor<WorldOutlinerEditorWindow>().lock();
        l_outlinerWindow)
    {
        auto& l_sceneSelectionState = l_outlinerWindow->GetMutableREFSceneSelectionState();

        l_sceneSelectionState.SetIsSceneSelected(m_afterIsSceneSelected);
    }
}