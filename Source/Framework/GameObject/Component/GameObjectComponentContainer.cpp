#include "GameObjectComponentContainer.h"

void FWK::GameObjectComponentContainer::INIT()
{
    m_uniqueComponentMap.clear();
    m_multiComponentMap.clear ();
}

void FWK::GameObjectComponentContainer::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectComponentContainer::PostDeserialize()
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { return; }

        l_component->PostDeserialize();
    }
}

void FWK::GameObjectComponentContainer::EarlyUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { return; }

        l_component->EarlyUpdate();
    }
}
void FWK::GameObjectComponentContainer::Update() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { return; }

        l_component->Update();
    }
}
void FWK::GameObjectComponentContainer::LateUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { return; }

        l_component->LateUpdate();
    }
}
void FWK::GameObjectComponentContainer::PostLateUpdate() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { return; }

        l_component->PostDeserialize();
    }
}

void FWK::GameObjectComponentContainer::EditInspector() const
{
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { return; }

        l_component->EditInspector();
    }
}

nlohmann::json FWK::GameObjectComponentContainer::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::GameObjectComponentContainer::CloneTo(GameObjectComponentContainer& a_cloneTarget, const std::weak_ptr<GameObject> a_gloneOwner) const
{

}
