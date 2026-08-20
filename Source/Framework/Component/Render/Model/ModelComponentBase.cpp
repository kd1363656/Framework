#include "ModelComponentBase.h"

void FWK::ModelComponentBase::INIT()
{
	if (!m_assetFilePathHelper) { return; }

	m_assetFilePathHelper->SetAllowedFileExtension(Constant::k_lowerFBXExtension);
}

void FWK::ModelComponentBase::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

nlohmann::json FWK::ModelComponentBase::SerializePrefab()
{
	return m_jsonConverter.SerializePrefab(*this);
}