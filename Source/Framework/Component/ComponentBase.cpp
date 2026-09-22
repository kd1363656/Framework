#include "ComponentBase.h"

void FWK::ComponentBase::DeserializePrefab(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.DeserializePrefab(a_rootJson);
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