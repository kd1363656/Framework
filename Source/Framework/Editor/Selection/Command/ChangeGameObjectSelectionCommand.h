#pragma once
 
namespace FWK
{
    class GameObject;

    namespace Editor
    {
        class EditorGameObjectSelectionState;
    }
}
 
namespace FWK::Editor
{
    // 選択状態変更をUndoRedo履歴へ登録するコマンド
    // 変更前と変更後の選択リスト・アンカーを保持し
    // Undo/Redo時にそれぞれの状態をEditorGameObjectSelectionStateへ復元する
    // EditorManagerが統括管理するEditorGameObjectSelectionStateへ
    // EditorManager::GetInstance()経由でアクセスする
    class ChangeGameObjectSelectionCommand final : public ICommand
    {
    public:
 
         ChangeGameObjectSelectionCommand(std::vector<boost::uuids::uuid>&& a_beforeUUIDList,
                                          std::vector<boost::uuids::uuid>&& a_afterUUIDList,
                                          boost::uuids::uuid                a_beforeAnchorUUID,
                                          boost::uuids::uuid                a_afterAnchorUUID);

        ~ChangeGameObjectSelectionCommand() override;
 
        void Undo() override;
        void Redo() override;
 
    private:

        std::vector<boost::uuids::uuid> m_beforeUUIDList = {};
        std::vector<boost::uuids::uuid> m_afterUUIDList  = {};

        boost::uuids::uuid m_beforeAnchorUUID = {};
        boost::uuids::uuid m_afterAnchorUUID  = {};
    };
}