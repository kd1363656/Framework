#include "ComponentBase.h"

void FWK::ComponentBase::DeserializeUUID(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializeUUID(a_rootJson, *this);
}

nlohmann::json FWK::ComponentBase::SerializeUUID()
{
    return m_jsonConverter.SerializeUUID(*this);
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