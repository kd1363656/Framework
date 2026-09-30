#include "AssetFilePath.h"

void FWK::AssetFilePath::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::AssetFilePath::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::AssetFilePath::EditInspector()
{
    m_inspector.EditInspector(*this);
}