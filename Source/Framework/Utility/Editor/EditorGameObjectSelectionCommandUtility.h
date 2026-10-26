#pragma once

namespace FWK::Utility
{
    // 現在の選択状態をUUIDのリストとして取り出す(Undo / Redoコマンドに積むための形)
    // WorldOutlinerとViewportのどちらで選択しても同じ形で記録する
    inline void FetchEditorGameObjectSelectionSnapshot(const Editor::EditorGameObjectSelectionState& a_gameObjectSelectionState, std::vector<boost::uuids::uuid>& a_outUUIDList, boost::uuids::uuid& a_outAnchorUUID)
    {
        const auto& l_selectedList = a_gameObjectSelectionState.GetREFSelectedGameObjectList();
        const auto& l_anchor       = a_gameObjectSelectionState.GetREFRangeSelectionAnchor  ().lock();

        a_outUUIDList.clear  ();
        a_outUUIDList.reserve(l_selectedList.size());

        for (const auto& l_selectedWeak : l_selectedList)
        {
            const auto& l_selectedGameObject = l_selectedWeak.lock();

            if (!l_selectedGameObject) { continue; }

            a_outUUIDList.emplace_back(l_selectedGameObject->GetREFSceneInstanceUUID());
        }

        a_outAnchorUUID = {};

        if (l_anchor)
        {
            a_outAnchorUUID = l_anchor->GetREFSceneInstanceUUID();
        }
    }

    // 選択を変える前の状態(a_before～)と、変えた後の状態(a_gameObjectSelectionStateの今の中身)を比べ、
    // 変わっていればChangeSelectionCommandをUndo / Redoへ積む
    inline void PushEditorGameObjectSelectionChangeCommand(const bool                                    a_beforeIsSceneSelected,
                                                           const bool                                    a_afterIsSceneSelected,
                                                                 std::vector<boost::uuids::uuid>&&       a_beforeUUIDList,
                                                                 boost::uuids::uuid&&                    a_beforeAnchorUUID,
                                                           const Editor::EditorGameObjectSelectionState& a_gameObjectSelectionState)
    {
        std::vector<boost::uuids::uuid> l_afterUUIDList   = {};
        boost::uuids::uuid              l_afterAnchorUUID = {};

        FetchEditorGameObjectSelectionSnapshot(a_gameObjectSelectionState, l_afterUUIDList, l_afterAnchorUUID);

        // 変更前後で選択状態が同じならコマンドをPushしない
        if (a_afterIsSceneSelected == a_beforeIsSceneSelected &&
            l_afterUUIDList        == a_beforeUUIDList        &&
            l_afterAnchorUUID      == a_beforeAnchorUUID)
        {
            return;
        }

        auto& l_editorManager  = Editor::EditorManager::GetInstance         ();
        auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

        l_undoRedoSystem.PushUndoCommand<Editor::ChangeSelectionCommand>(a_beforeIsSceneSelected,
                                                                         a_afterIsSceneSelected,
                                                                         std::move(a_beforeUUIDList),
                                                                         std::move(l_afterUUIDList),
                                                                         a_beforeAnchorUUID,
                                                                         l_afterAnchorUUID);
    }
}
