#include "ModelComponentBase.h"

void FWK::ModelComponentBase::Setup()
{
	m_assetFilePathHelper.SetAllowedFileExtension(Constant::k_lowerFBXExtension);
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