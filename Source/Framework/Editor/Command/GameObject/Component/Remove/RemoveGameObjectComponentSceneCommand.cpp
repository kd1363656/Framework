#include "RemoveGameObjectComponentSceneCommand.h"

FWK::Editor::RemoveGameObjectComponentSceneCommand::RemoveGameObjectComponentSceneCommand(const std::weak_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID) :
    // 外した後もUndoで戻せるように、実体の所有をこのコマンドが引き継ぐ
    m_component(a_component.lock()),

    m_gameObjectUUID(a_gameObjectUUID)
{}
FWK::Editor::RemoveGameObjectComponentSceneCommand::~RemoveGameObjectComponentSceneCommand() = default;

void FWK::Editor::RemoveGameObjectComponentSceneCommand::Undo()
{
    if (!m_component) { return; }

    const auto& l_gameObject = Utility::FindVALSceneGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // Prefab由来のコンポーネントは外したときに「Prefabから削除したUUID」として記録されている
    // 記録が残ったまま戻すと、AddComponentが別のUUIDを発行してPrefabとの対応が切れるため、先に記録を消す
    // 例 : Prefabのカメラを削除 → Undo → 同じUUIDのカメラとして戻り、Prefabの値の変更も今までどおり伝わる
    if (m_component->GetVALIsPrefabOrigin())
    {
        l_componentContainer.ErasePrefabRemovedComponentUUID(m_component->GetREFUUID());
    }

    l_componentContainer.AddComponent(m_component);
}

void FWK::Editor::RemoveGameObjectComponentSceneCommand::Redo()
{
    if (!m_component) { return; }

    const auto& l_gameObject = Utility::FindVALSceneGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    l_componentContainer.RemoveComponent(m_component);
}