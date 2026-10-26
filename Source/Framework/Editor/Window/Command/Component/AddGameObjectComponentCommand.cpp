#include "AddGameObjectComponentCommand.h"

FWK::Editor::AddGameObjectComponentCommand::AddGameObjectComponentCommand(const boost::uuids::uuid& a_gameObjectUUID, const std::shared_ptr<GameObjectComponentBase>& a_component) :
    m_component     (a_component),
    m_gameObjectUUID(a_gameObjectUUID)
{}
FWK::Editor::AddGameObjectComponentCommand::~AddGameObjectComponentCommand() = default;

void FWK::Editor::AddGameObjectComponentCommand::Undo()
{
    if (!m_component) { return; }

    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    // 削除されたGameObjectは戻せないので何もしない
    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // シーンが作り直されているとm_componentとは別の実体になっているため、アドレスではなくコンポーネントのUUIDで探して取り外す
    l_componentContainer.RemoveComponent(l_componentContainer.FindVALComponentByUUID(m_component->GetREFUUID()));
}
void FWK::Editor::AddGameObjectComponentCommand::Redo()
{
    if (!m_component) { return; }

    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // Undoで取り外した同じ実体を付け直すので、Undoする前に変えた値もそのまま戻る
    // 持ち主の設定とUUIDの登録はAddComponentが行う
    l_componentContainer.AddComponent(m_component);
}