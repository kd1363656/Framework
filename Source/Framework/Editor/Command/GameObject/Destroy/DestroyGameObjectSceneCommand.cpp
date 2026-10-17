#include "DestroyGameObjectSceneCommand.h"

FWK::Editor::DestroyGameObjectSceneCommand::DestroyGameObjectSceneCommand(const std::vector<std::weak_ptr<GameObject>>& a_destroyGameObjectList) :
    m_recordList()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 削除する前の親と並び順を記録しておく
    // 削除はEditorUndoRedoSystem::ExecuteUndoCommandから最初のRedoで行われるため、
    // ここではまだシーンから外れていない
    m_recordList.reserve(a_destroyGameObjectList.size());

    for (const auto& l_destroyGameObject : a_destroyGameObjectList)
    {
        if (l_destroyGameObject.expired()) { continue; }

        auto& l_record = m_recordList.emplace_back();

        l_record.Capture(l_destroyGameObject, *l_scene);
    }
}
FWK::Editor::DestroyGameObjectSceneCommand::~DestroyGameObjectSceneCommand() = default;

void FWK::Editor::DestroyGameObjectSceneCommand::Undo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 削除したときと逆の順番で戻す
    // 例 : 兄弟[A, B, C]のAとBを削除 → Bを戻して[B, C]、Aを戻して[A, B, C]
    for (auto l_itr = m_recordList.rbegin(); l_itr != m_recordList.rend(); ++l_itr)
    {
        l_itr->Restore(*l_scene);
    }
}

void FWK::Editor::DestroyGameObjectSceneCommand::Redo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 破棄フラグ(GameObject::Destroy)は使わずにシーンから外すだけにする
    // 破棄フラグを立てると元に戻せないため
    for (const auto& l_record : m_recordList)
    {
        l_record.Remove(*l_scene);
    }
}