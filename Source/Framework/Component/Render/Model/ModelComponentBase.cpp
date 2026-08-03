#include "ModelComponentBase.h"

void FWK::ModelComponentBase::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_assetFilePathHelper.Deserialize(a_rootJson);
}

nlohmann::json FWK::ModelComponentBase::SerializePrefab()
{
	return m_assetFilePathHelper.Serialize();
}