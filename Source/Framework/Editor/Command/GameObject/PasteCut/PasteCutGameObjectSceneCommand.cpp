#include "PasteCutGameObjectSceneCommand.h"

FWK::Editor::PasteCutGameObjectSceneCommand::PasteCutGameObjectSceneCommand(const std::vector<std::weak_ptr<GameObject>>& a_pastedGameObjectList, const std::vector<std::weak_ptr<GameObject>>& a_cutGameObjectList) :
    m_pastedRecordList(),
    m_cutRecordList   ()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 切り取り→貼り付けは「貼り付けたものを作る」と「切り取った元を消す」の2つを1回の操作として扱う
    // Ctrl + Zを1回押すだけで両方とも元へ戻るようにするため、1つのコマンドにまとめている
    m_pastedRecordList.reserve(a_pastedGameObjectList.size());
    m_cutRecordList.reserve   (a_cutGameObjectList.size());

    for (const auto& l_pastedGameObject : a_pastedGameObjectList)
    {
        if (l_pastedGameObject.expired()) { continue; }

        auto& l_record = m_pastedRecordList.emplace_back();

        l_record.Capture(l_pastedGameObject, *l_scene);
    }

    // 切り取った元は、まだシーンから外す前の親と並び順を記録する
    // 外すのはEditorUndoRedoSystem::ExecuteUndoCommandから呼ばれる最初のRedo
    for (const auto& l_cutGameObject : a_cutGameObjectList)
    {
        if (l_cutGameObject.expired()) { continue; }

        auto& l_record = m_cutRecordList.emplace_back();

        l_record.Capture(l_cutGameObject, *l_scene);
    }
}
FWK::Editor::PasteCutGameObjectSceneCommand::~PasteCutGameObjectSceneCommand() = default;

void FWK::Editor::PasteCutGameObjectSceneCommand::Undo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 貼り付けたものを外してから、切り取った元を戻す
    for (auto l_itr = m_pastedRecordList.rbegin(); l_itr != m_pastedRecordList.rend(); ++l_itr)
    {
        l_itr->Remove(*l_scene);
    }

    for (auto l_itr = m_cutRecordList.rbegin(); l_itr != m_cutRecordList.rend(); ++l_itr)
    {
        l_itr->Restore(*l_scene);
    }
}

void FWK::Editor::PasteCutGameObjectSceneCommand::Redo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 貼り付けたものを戻してから、切り取った元を外す
    // 最初の実行では貼り付けたものは既にシーンにあるため、Restoreは何もしない
    for (const auto& l_record : m_pastedRecordList)
    {
        l_record.Restore(*l_scene);
    }

    for (const auto& l_record : m_cutRecordList)
    {
        l_record.Remove(*l_scene);
    }
}