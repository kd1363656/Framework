#include "GameObject.h"

void FWK::GameObject::INIT()
{
    if (!m_transformComponent)
    {
        m_transformComponent = std::make_shared<GameObjectTransformComponent>();
    }

    m_transformComponent->SetOwner(weak_from_this());

    m_hierarchy.INIT    ();
    m_hierarchy.SetOwner(weak_from_this());

    m_componentContainer.INIT    ();
    m_componentContainer.SetOwner(weak_from_this());
}

void FWK::GameObject::DeserializeScene(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializeScene(a_rootJson, weak_from_this());
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json & a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefab(a_rootJson, weak_from_this());
}

void FWK::GameObject::PostDeserialize()
{
    if (m_transformComponent) 
    {
        m_transformComponent->PostDeserialize();
    }

    m_componentContainer.PostDeserialize();
}

void FWK::GameObject::EarlyUpdate() const
{
    m_componentContainer.EarlyUpdate();
}
void FWK::GameObject::Update() const
{
    m_componentContainer.Update();
}
void FWK::GameObject::LateUpdate() const
{
    m_componentContainer.LateUpdate();
}
void FWK::GameObject::PostLateUpdate() const
{
    if (m_transformComponent)
    {
        m_transformComponent->PostLateUpdate();
    }
}

void FWK::GameObject::Destroy()
{
    m_isDestroyed = true;
}

void FWK::GameObject::EditInspector()
{
    if (m_transformComponent)
    {
        m_transformComponent->EditInspector();
    }

    m_componentContainer.EditInspector();
}

nlohmann::json FWK::GameObject::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::GameObject::ConvertToPrefab(const boost::uuids::uuid& a_prefabUUID)
{
    // 自身のプレハブ情報を設定する
    m_prefabUUID     = a_prefabUUID;
    m_isPrefabOrigin = true;
 
    // PrefabHierarchyNodeUUIDがnilなら新規生成する
    // プレハブ内での識別に使うため必須
    if (m_prefabHierarchyNodeUUID.is_nil())
    {
        auto& l_uuidManager = Utility::UUIDManager::GetInstance();
 
        m_prefabHierarchyNodeUUID = l_uuidManager.GenerateVALUUID();
    }
 
    // 子も再帰的にプレハブ化する
    const auto& l_childSmartPointerVectorList = m_hierarchy.GetREFChildSmartPointerVectorList      ();
    const auto& l_childList                   = l_childSmartPointerVectorList.GetREFElementDataList();
 
    for (const auto& l_childData : l_childList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        if (!l_child) { continue; }
 
        // 既に別のPrefabのインスタンスである子には伝播しない
        // その子は別のPrefabとして独自に更新伝播を受ける
        const auto& l_childPrefabUUID = l_child->GetREFPrefabUUID();

        // PrefabUUIDに何かしら入っている場合はPrefab
        // なので処理を飛ばすこのゲームオブジェクトのプレハブの一部として扱わない
        if (!l_childPrefabUUID.is_nil() &&
            l_childPrefabUUID != a_prefabUUID)
        {
            continue;
        }

        l_child->ConvertToPrefab(a_prefabUUID);
    }
}

void FWK::GameObject::ClearAllPrefabRemovedUUIDSet()
{
    // 自身のHierarchyの削除済み子UUID集合をクリアする
    m_hierarchy.ClearPrefabRemovedChildUUIDSet();

    // 自身のComponentContainerの削除済みコンポーネントUUID集合をクリアする
    m_componentContainer.ClearPrefabRemovedComponentUUIDSet();
 
    // 子も再帰的にクリアする
    const auto& l_childSmartPointerVectorList = m_hierarchy.GetREFChildSmartPointerVectorList      ();
    const auto& l_childList                   = l_childSmartPointerVectorList.GetREFElementDataList();
 
    for (const auto& l_childData : l_childList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        if (!l_child) { continue; }
 
        l_child->ClearAllPrefabRemovedUUIDSet();
    }
}