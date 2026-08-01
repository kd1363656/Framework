#include "AssetFilePathUtility.h"

void FWK::Utility::AssetFilePath::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Utility::AssetFilePath::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}