#include "GameObjectComponentContainer.h"

void FWK::GameObjectComponentContainer::INIT()
{

}

void FWK::GameObjectComponentContainer::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectComponentContainer::PostDeserialize()
{

}

void FWK::GameObjectComponentContainer::EarlyUpdate() const
{

}
void FWK::GameObjectComponentContainer::Update() const
{

}
void FWK::GameObjectComponentContainer::LateUpdate() const
{

}
void FWK::GameObjectComponentContainer::PostLateUpdate() const
{

}

void FWK::GameObjectComponentContainer::EditInspector() const
{

}

nlohmann::json FWK::GameObjectComponentContainer::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}