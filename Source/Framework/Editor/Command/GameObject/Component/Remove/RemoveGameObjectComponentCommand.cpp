#include "RemoveGameObjectComponentCommand.h"

FWK::Editor::RemoveGameObjectComponentCommand::RemoveGameObjectComponentCommand(const std::vector<boost::uuids::uuid>& a_beforeComponentUUIDList, const std::shared_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID) :
    m_beforeComponentUUIDList(a_beforeComponentUUIDList),

    m_component(a_component),

    m_gameObjectUUID(a_gameObjectUUID)
{}
FWK::Editor::RemoveGameObjectComponentCommand::~RemoveGameObjectComponentCommand() = default;

void FWK::Editor::RemoveGameObjectComponentCommand::Undo()
{
    const auto& l_gameObject = FindVALTargetGameObject().lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // Prefab由来のコンポーネントを削除すると、削除済みUUIDとして記録される
    // 同じUUIDのまま戻すため、先に記録を消す(残っていると、AddComponentが新しいUUIDを発行してしまう)
    l_componentContainer.RemovePrefabRemovedComponentUUID(m_component->GetREFUUID());

    if (!l_componentContainer.AddComponent(m_component)) { return; }

    // AddComponentは末尾に追加するため、削除する前の並びに戻す
    // 位置は添字ではなくUUIDの並びで覚えているので、削除後にほかのコンポーネントが変わっていても位置がずれない
    l_componentContainer.ApplyComponentOrder(m_beforeComponentUUIDList);
}

void FWK::Editor::RemoveGameObjectComponentCommand::Redo()
{
    const auto& l_gameObject = FindVALTargetGameObject().lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // もう一度削除する
    // このコマンドがshared_ptrを持っているため、削除しても実体は消えず、Undoで同じものを戻せる
    l_componentContainer.RemoveComponent(m_component);
}

std::weak_ptr<FWK::GameObject> FWK::Editor::RemoveGameObjectComponentCommand::FindVALTargetGameObject() const
{
    // コマンドはGameObjectを直接保持しないため、SceneInstanceUUIDから現在のGameObjectを検索する
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return {}; }

    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return {};
    }

    return l_gameObject;
}