#include "GameObjectComponentContainer.h"

void FWK::GameObjectComponentContainer::INIT()
{
    m_uniqueComponentMap.clear();
    m_multiComponentMap.clear ();

    m_prefabRemovedComponentUUIDSet.clear();

    m_owner = {};

    m_componentUUIDRegistry.Clear();

    m_componentSmartPointerVectorList.Clear();

    m_jsonConverter = {};
}

void FWK::GameObjectComponentContainer::DeserializeScene(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializeScene(a_rootJson, *this);
}
void FWK::GameObjectComponentContainer::DeserializePrefab(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

void FWK::GameObjectComponentContainer::PostDeserialize()
{
    // ListやMapから無効なコンポーネントを削除
    SweepExpiredComponents();

    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component ||
            l_component->GetVALIsDisable())
        {
            continue; 
        }

        l_component->PostDeserialize();
    }
}

void FWK::GameObjectComponentContainer::EarlyUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component ||
            l_component->GetVALIsDisable())
        {
            continue;
        }

        l_component->EarlyUpdate();
    }
}
void FWK::GameObjectComponentContainer::Update() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component ||
            l_component->GetVALIsDisable())
        {
            continue;
        }

        l_component->Update();
    }
}
void FWK::GameObjectComponentContainer::LateUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component ||
            l_component->GetVALIsDisable())
        {
            continue;
        }

        l_component->LateUpdate();
    }
}

void FWK::GameObjectComponentContainer::PostLateUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component ||
            l_component->GetVALIsDisable())
        {
            continue;
        }

        l_component->PostLateUpdate();
    }
}

void FWK::GameObjectComponentContainer::EditInspector() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->EditInspector();
    }
}

nlohmann::json FWK::GameObjectComponentContainer::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::GameObjectComponentContainer::CloneTo(GameObjectComponentContainer& a_cloneTarget, const std::weak_ptr<GameObject>& a_cloneOwner) const
{
    a_cloneTarget.m_owner = a_cloneOwner;

    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        // コンポーネントごとに定義されたClone()を経由する
        // unique_ptrメンバもSerialize/Deserialize往復で再構築される
        auto l_cloneComponent = l_component->Clone();

        if (!l_cloneComponent)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントのCloneに失敗しました。");

            continue;
        }

        // UUIDはコピー元と同じものを保持する
        // Prefab由来コンポーネントとのリンクが維持されるようになる
        // 複製先のレジストリは別物なので衝突せずそのまま登録される
        a_cloneTarget.AddComponent(l_cloneComponent);
    }

    // 削除済みUUIDも引き継ぐ
    // Prefabインスタンスの複製でも差分の整合性が保たれるようにするため
    a_cloneTarget.m_prefabRemovedComponentUUIDSet = m_prefabRemovedComponentUUIDSet;
}

bool FWK::GameObjectComponentContainer::AddComponent(const std::shared_ptr<GameObjectComponentBase>& a_component)
{
    if (!a_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "登録対象のコンポーネントが無効なため、ComponentContainerへの登録に失敗しました。");

        return false;
    }

    const auto  l_staticTypeID = a_component->GetREFRuntimeTypeINFO().k_staticTypeID;
    
    // 未発行、または既に使用履歴のあるUUIDなら新規発行する
    // Prefabで消されたUUIDと同じUUIDを持つシーン側コンポーネントが
    // Prefabコンポーネントと誤認されないようにするため
    if (const auto& l_uuid = a_component->GetREFUUID();
        l_uuid.is_nil()                          ||
        m_componentUUIDRegistry.Contains(l_uuid) ||
        m_prefabRemovedComponentUUIDSet.contains(l_uuid))
    {
        const auto& l_generatedUUID = GenerateVALComponentUUID();
        
        //  m_componentUUIDRegistry、m_removedUUIDSetどちらともかぶっていないUUIDをセット
        a_component->SetUUID(l_generatedUUID);
    }

    a_component->SetOwner(m_owner);

    // 本体配列へ登録(重複アドレスはSmartPointerVectorList側ではじかれる)
    m_componentSmartPointerVectorList.Add(a_component);

    if (!m_componentUUIDRegistry.Add(a_component, a_component->GetREFUUID()))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントUUIDの登録に失敗したため、ComponentContainerへの登録に失敗しました。");

        return false;
    }

    // 複製が許可されているかどうかで高速アクセスするために適したマップに登録する
    if (a_component->IsAllowMultiple())
    {
        m_multiComponentMap[l_staticTypeID].emplace_back(a_component);
    }
    else
    {
        m_uniqueComponentMap.try_emplace(l_staticTypeID, a_component);
    }

    return true;
}
void FWK::GameObjectComponentContainer::AddPrefabRemovedComponentUUID(const boost::uuids::uuid& a_uuid)
{
    if (a_uuid.is_nil()) { return; }

    m_prefabRemovedComponentUUIDSet.emplace(a_uuid);
}

void FWK::GameObjectComponentContainer::SweepExpiredComponents()
{
    m_componentSmartPointerVectorList.RemoveExpiredElements();

    std::erase_if(m_uniqueComponentMap, 
                 [](const auto& a_pair)
                 {
                     return a_pair.second.expired();
                 });
 
    for (auto& [l_staticTypeID, l_componentList] : m_multiComponentMap)
    {
        std::erase_if(l_componentList, 
                      [](const auto& a_weakComponent)
                      {
                          return a_weakComponent.expired();
                      });
    }
 
    std::erase_if(m_multiComponentMap, 
                  [](const auto& a_pair)
                  {
                      return a_pair.second.empty();
                  });
}

void FWK::GameObjectComponentContainer::RemoveComponent(const std::weak_ptr<GameObjectComponentBase>& a_component)
{
    const auto& l_component = a_component.lock();

    if (!l_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "削除対象Componentが無効のため、GameObjectから削除することができませんでした。");

        return;
    }

    const auto  l_staticTypeID         = l_component->GetREFRuntimeTypeINFO().k_staticTypeID;
    const auto& l_uuid                 = l_component->GetREFUUID           ();
          bool  l_isRemovedFromTypeMap = false;

    if (!l_component->IsAllowMultiple())
    {
        auto l_itr = m_uniqueComponentMap.find(l_staticTypeID);

        // イテレータらから探索して、もしなければreturn
        if (l_itr == m_uniqueComponentMap.end()) { return; }

        auto l_registeredComponent = l_itr->second.lock();

        if (!l_registeredComponent)
        {
            m_uniqueComponentMap.erase(l_itr);

            return;
        }

        // 同一アドレスでない場合return
        if (l_registeredComponent != l_component) { return; }

        m_uniqueComponentMap.erase(l_itr);

        l_isRemovedFromTypeMap = true;
    }
    else
    {
        auto l_componentListITR = m_multiComponentMap.find(l_staticTypeID);

        if (l_componentListITR == m_multiComponentMap.end()) { return; }

        // コンポーネントリストをMapから取得
        auto& l_componentList = l_componentListITR->second;
        auto  l_componentITR  = l_componentList.begin();

        // リストから同じアドレスのコンポーネントを見つけ削除する
        while (l_componentITR != l_componentList.end())
        {
            auto l_registeredComponent = l_componentITR->lock();

            if (!l_registeredComponent)
            {
                l_componentITR = l_componentList.erase(l_componentITR);

                continue;
            }

            if (l_registeredComponent != l_component)
            {
                ++l_componentITR;

                continue;
            }

            l_componentList.erase(l_componentITR);

            l_isRemovedFromTypeMap = true;

            break;
        }

        if (l_componentList.empty())
        {
            m_multiComponentMap.erase(l_componentListITR);
        }
    }

    if (!l_isRemovedFromTypeMap) { return; }

    // 先にコンポーネントを削除するがl_componentはローカル変数なのでスコープ内で生きており
    // l_uuidはl_componentのメンバへの参照のため問題なし
    m_componentSmartPointerVectorList.RemoveSameElement(l_component);

    // Prefab由来のComponentを削除した場合は削除済みUUIDとして記録する
    // Prefab更新伝播時に削除意図を保持するため
    if (l_component->GetVALIsPrefabOrigin())
    {
        AddPrefabRemovedComponentUUID(l_uuid);
    }

    // Registryからも削除する
    // （削除したUUIDが「生きている」と誤認されないようにするため）
    m_componentUUIDRegistry.Erase(l_uuid);
}

void FWK::GameObjectComponentContainer::ClearPrefabRemovedComponentUUIDSet()
{
    m_prefabRemovedComponentUUIDSet.clear();
}

bool FWK::GameObjectComponentContainer::IsPrefabRemovedComponentUUID(const boost::uuids::uuid& a_uuid) const
{
    if (a_uuid.is_nil()) { return false; }

    return m_prefabRemovedComponentUUIDSet.contains(a_uuid);
}

std::weak_ptr<FWK::GameObjectComponentBase> FWK::GameObjectComponentContainer::FindVALComponentByUUID(const boost::uuids::uuid& a_uuid) const
{
    if (a_uuid.is_nil()) { return {}; }

    return m_componentUUIDRegistry.FindVALRegisteredType(a_uuid);
}

boost::uuids::uuid FWK::GameObjectComponentContainer::GenerateVALComponentUUID() const
{
    boost::uuids::uuid l_uuid        = {};
    auto&              l_uuidManager = Utility::UUIDManager::GetInstance();

    while (l_uuid.is_nil())
    {
        l_uuid = l_uuidManager.GenerateVALUUID();

        // もしm_componentUUIDRegistryまたはm_removedComponentsUUIDSetに含まれているUUID
        // なら意図的にnil値にしてもう一度UUIDを生成する
        if (m_componentUUIDRegistry.Contains(l_uuid) ||
            m_prefabRemovedComponentUUIDSet.contains(l_uuid))
        {
            l_uuid = {};
        }
    }

    return l_uuid;
}