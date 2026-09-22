#include "ComponentBase.h"

void FWK::ComponentBase::DeserializePrefabUUID(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefabUUID(a_rootJson, *this);
}

nlohmann::json FWK::ComponentBase::SerializePrefabUUID()
{
    return m_jsonConverter.SerializePrefabUUID(*this);
}

void FWK::ComponentBase::Enable()
{
    if (!m_isDisable) { return; }

    m_isDisable = false;
}
void FWK::ComponentBase::Disable()
{
    if (m_isDisable) { return; }

    m_isDisable = true;
}