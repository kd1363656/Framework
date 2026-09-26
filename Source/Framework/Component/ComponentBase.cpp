#include "ComponentBase.h"

void FWK::ComponentBase::INITBase()
{
    m_owner = {};

    m_jsonConverter = {};

    m_uuid = {};

    m_isDisable       = false;
    m_isSerializeSkip = false;
}

void FWK::ComponentBase::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::ComponentBase::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}