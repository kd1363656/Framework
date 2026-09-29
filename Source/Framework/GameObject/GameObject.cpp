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

void FWK::GameObject::DeserializeScene(const nlohmann::json&              a_rootJson, 
                                       const nlohmann::json&              a_baseJson, 
                                       const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                             Scene&                       a_scene)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializeScene(weak_from_this(), 
                                     a_rootJson,
                                     a_baseJson, 
                                     a_prefabSystem, 
                                     a_scene);
}
void FWK::GameObject::DeserializePrefab(const nlohmann::json& a_rootJson, const SceneGameObjectPrefabSystem& a_prefabSystem, Scene& a_scene)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefab(weak_from_this(),
                                      a_rootJson,
                                      a_prefabSystem,
                                      a_scene);
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

    m_componentContainer.PostLateUpdate();
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

nlohmann::json FWK::GameObject::Serialize(SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.Serialize(*this, a_prefabSystem);
}
nlohmann::json FWK::GameObject::SerializeScene(SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.SerializeScene(*this, a_prefabSystem);
}
nlohmann::json FWK::GameObject::SerializeDiff(const nlohmann::json& a_baseJson, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    return m_jsonConverter.SerializeDif(a_baseJson, *this, a_prefabSystem);
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
 
    // コンポーネントコンテナのコンポーネントもプレハブ化
    m_componentContainer.ConvertToPrefab();

    // 子も再帰的にプレハブ化する
    m_hierarchy.ConvertToPrefab(a_prefabUUID);
}

void FWK::GameObject::DetachFromPrefab()
{
    // 子への再帰判定で使うため、先に元のPrefabUUIDを保持する
    const auto& l_oldPrefabUUID = m_prefabUUID;
 
    // Prefabとの紐付けを断つ
    // 以降このGameObjectはPrefab由来ではない通常のGameObjectとして
    // フル形式でシリアライズされる
    m_prefabUUID     = {};
    m_isPrefabOrigin = Constant::l_gameObjectInitialValueIsPrefabOrigin;
 
    // PrefabHierarchyNodeUUIDはあえて変更しない
    // 既に親のChildUUIDRegistryへこのUUIDで登録済みであり
    // 変更するとRegistryのキーと不整合になる
 
    // ComponentContainerとHierarchyが持つPrefab追跡情報も剥がす
    m_componentContainer.DetachFromPrefab();
    m_hierarchy.DetachFromPrefab         ();
 
    // 子もPrefab由来ではなくなるため再帰的に剥がす
    // 子がPrefabUUIDを持ち続けると差分形式でシリアライズされ
    // PrefabルートJsonを基底とした誤った差分が作られてしまう
    const auto& l_childList = m_hierarchy.GetREFChildSmartPointerVectorList().GetREFElementDataList();
 
    for (const auto& l_childData : l_childList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        if (!l_child) { continue; }
 
        // 別Prefabのインスタンスである子は剥がさない
        // (ネストしたPrefabインスタンスとしての紐付けは維持する)
        if (l_child->GetREFPrefabUUID() != l_oldPrefabUUID) { continue; }
 
        l_child->DetachFromPrefab();
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

bool FWK::GameObject::FetchVALIsPrefabInternalChild() const
{
    // PrefabUUIDが無い = Prefab由来ではないので内部子ではない
    if (m_prefabUUID.is_nil()) { return false; }

    const auto& l_parent = m_hierarchy.GetREFParent().lock();

    // 親がいない = Prefabインスタンスのルートか通常GameObject
    if (!l_parent) { return false; }

    // 親が同じPrefabUUIDを持つ = このGameObjectはPrefab内部の子
    // 親のPrefabUUIDが違う     = Prefabインスタンスのルート
    return l_parent->GetREFPrefabUUID() == m_prefabUUID;
}