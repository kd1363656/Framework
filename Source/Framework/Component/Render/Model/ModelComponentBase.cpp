#include "ModelComponentBase.h"

void FWK::ModelComponentBase::INIT()
{
	if (!m_assetFilePathHelper)
	{
		m_assetFilePathHelper = std::make_shared<Utility::AssetFilePathHelper>();
	}

	m_assetFilePathHelper->SetAllowedFileExtension(Constant::k_lowerFBXExtension);
}

void FWK::ModelComponentBase::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}
void FWK::ModelComponentBase::PostDeserialize()
{
	const auto& l_owner = GetREFOwner().lock();

	if (!l_owner) { return; }

	m_fetchTransformComponentFromSelfGameObjectHelper.PostDeserialize(*l_owner);
}

void FWK::ModelComponentBase::EditInspector()
{
	m_inspector.EditInspector(*this);
}

nlohmann::json FWK::ModelComponentBase::SerializePrefab()
{
	return m_jsonConverter.SerializePrefab(*this);
}