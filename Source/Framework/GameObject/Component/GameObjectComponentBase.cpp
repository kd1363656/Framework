#include "GameObjectComponentBase.h"

void FWK::GameObjectComponentBase::INIT()
{
    m_owner = {};

    m_jsonConverter = {};

    m_uuid = {};

    m_isDisable = false;
}

void FWK::GameObjectComponentBase::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::GameObjectComponentBase::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

nlohmann::json FWK::GameObjectComponentBase::Serialize() const
{
    return nlohmann::json();
}