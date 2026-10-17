#include "CreateGameObjectSceneCommand.h"

FWK::Editor::CreateGameObjectSceneCommand::CreateGameObjectSceneCommand(const std::vector<std::weak_ptr<GameObject>>& a_createdGameObjectList) :
    m_recordList()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 作成した直後の親と並び順を記録しておく
    // Undoでシーンから外しても実体は記録が持ち続けるため、Redoで同じGameObjectを戻せる
    m_recordList.reserve(a_createdGameObjectList.size());

    for (const auto& l_createdGameObject : a_createdGameObjectList)
    {
        if (l_createdGameObject.expired()) { continue; }

        auto& l_record = m_recordList.emplace_back();

        l_record.Capture(l_createdGameObject, *l_scene);
    }
}
FWK::Editor::CreateGameObjectSceneCommand::~CreateGameObjectSceneCommand() = default;

void FWK::Editor::CreateGameObjectSceneCommand::Undo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 作成したときと逆の順番で子孫ごとシーンから外す
    for (auto l_itr = m_recordList.rbegin(); l_itr != m_recordList.rend(); ++l_itr)
    {
        l_itr->Remove(*l_scene);
    }
}

void FWK::Editor::CreateGameObjectSceneCommand::Redo()
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 作成した順番で子孫ごとシーンへ戻し、親と並び順も記録どおりにする
    for (const auto& l_record : m_recordList)
    {
        l_record.Restore(*l_scene);
    }
}