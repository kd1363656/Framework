#include "AddGameObjectComponentCommand.h"

FWK::Editor::AddGameObjectComponentCommand::AddGameObjectComponentCommand(const std::shared_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID) :
    m_component(a_component),

    m_gameObjectUUID(a_gameObjectUUID)
{}
FWK::Editor::AddGameObjectComponentCommand::~AddGameObjectComponentCommand() = default;

void FWK::Editor::AddGameObjectComponentCommand::Undo()
{
    const auto& l_gameObject = FindVALTargetGameObject().lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // 追加したコンポーネントをGameObjectから取り外す
    // このコマンドがshared_ptrを持っているため、取り外しても実体は消えず、Redoで同じものを戻せる
    l_componentContainer.RemoveComponent(m_component);
}

void FWK::Editor::AddGameObjectComponentCommand::Redo()
{
    const auto& l_gameObject = FindVALTargetGameObject().lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // 取り外したコンポーネントをもう一度追加する
    // 取り外したときにUUIDの登録も外れているため、同じUUIDのまま戻る
    l_componentContainer.AddComponent(m_component);
}

std::weak_ptr<FWK::GameObject> FWK::Editor::AddGameObjectComponentCommand::FindVALTargetGameObject() const
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