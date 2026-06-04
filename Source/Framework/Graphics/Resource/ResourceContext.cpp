#include "ResourceContext.h"

void FWK::Graphics::ResourceContext::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Graphics::ResourceContext::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}