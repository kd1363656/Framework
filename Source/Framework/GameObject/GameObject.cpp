#include "GameObject.h"

void FWK::GameObject::INIT()
{

}

void FWK::GameObject::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, weak_from_this());
}

void FWK::GameObject::PostDeserialize()
{
    
}

void FWK::GameObject::EarlyUpdate() const
{

}
void FWK::GameObject::Update() const
{

}
void FWK::GameObject::LateUpdate() const
{

}
void FWK::GameObject::PostLateUpdate() const
{

}

void FWK::GameObject::Destroy()
{

}

void FWK::GameObject::EditInspector()
{

}

nlohmann::json FWK::GameObject::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}