#include "GameObjectComponentContainer.h"

bool FWK::GameObjectComponentContainer::DeserializePrefab(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return false; }

    return m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}
bool FWK::GameObjectComponentContainer::DeserializeScene(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return false; }

    return m_jsonConverter.DeserializeScene (a_rootJson, *this);;
}

void FWK::GameObjectComponentContainer::Add(const std::shared_ptr<ComponentBase>& a_component)
{
    if (!a_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントが無効となっており割り当てに失敗しました。");

        return;
    }

    // 派生クラスの静的IDを取得(このコンポーネントを取得時に使用)
    const auto l_staticTypeID = a_component->GetREFRuntimeTypeINFO().k_staticTypeID;
          bool l_canAdd       = false;

    // 複数持てるコンポーネントかどうかを判断して
    // 適切なstd::unordered_mapに割り当てる
    if (!a_component->IsAllowMultiple())
    {
        l_canAdd = m_uniqueComponentMap.try_emplace(l_staticTypeID, a_component).second;
    }
    else
    {
        m_multiComponentMap[l_staticTypeID].emplace_back(a_component);

        l_canAdd = true;
    }

    if (!l_canAdd)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Component : {}\nコンポーネントの格納に失敗しました。", a_component->GetREFTypeINFO().k_name);

        return;
    }

    const std::weak_ptr<ComponentBase> l_component = a_component;

    // コンポーネントに割り当てられたUUIDを格納
    m_componentUUIDRegistry.Add          (l_component, a_component->GetMutableREFUUID());
    m_componentSmartPointerVectorList.Add(a_component);
}

void FWK::GameObjectComponentContainer::Remove(const std::weak_ptr<ComponentBase>& a_component)
{
    const auto& l_component = a_component.lock();

    if (!l_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "削除対象Componentが無効のため、GameObjectから削除することができませんでした。");

        return;
    }

    const auto l_staticTypeID         = l_component->GetREFRuntimeTypeINFO().k_staticTypeID;
          bool l_isRemovedFromTypeMap = false;

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

    // Prefab由来Componentを削除した場合アh削除記録へUUIDを追加する
    if (l_component->GetVALIsPrefabOrigin())
    {
        m_removedComponentUUIDSet.emplace(l_component->GetREFUUID());
    }

    m_componentSmartPointerVectorList.RemoveSameElement(l_component);
}

void FWK::GameObjectComponentContainer::MarkForRemoval(const std::weak_ptr<ComponentBase>& a_component)
{
    const auto& l_component = a_component.lock();

    if (!l_component) { return; }

    // 削除されたコンポーネントとしてマーク
    l_component->SetMarkedForRemoval(true);

    if (l_component->GetVALIsPrefabOrigin())
    {
        m_removedComponentUUIDSet.emplace(l_component->GetREFUUID());
    }
}

void FWK::GameObjectComponentContainer::SweepRemoved()
{
    std::vector<std::weak_ptr<ComponentBase>> l_markedList = {};

    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        if (l_componentData.m_type &&
            l_componentData.m_type->GetVALIsMarkedForRemoval())
        {
            l_markedList.emplace_back(l_componentData.m_type);
        }
    }

    // 走査中の削除を避けるため、対象を集めてから一括で削除
    for (const auto& l_component : l_markedList)
    {
        Remove(l_component);
    }
}

void FWK::GameObjectComponentContainer::Clear()
{
    m_uniqueComponentMap.clear();
    m_multiComponentMap.clear ();

    m_removedComponentUUIDSet.clear();

    m_componentSmartPointerVectorList.Clear();

    m_componentUUIDRegistry.Clear();
}

nlohmann::json FWK::GameObjectComponentContainer::SerializePrefab() const
{
    return m_jsonConverter.SerializePrefab(*this);
}
nlohmann::json FWK::GameObjectComponentContainer::SerializeScene() const
{
    return m_jsonConverter.SerializeScene (*this);;
}