#include "RotationComponentConstantModeBase.h"

void FWK::RotationComponentConstantModeBase::INIT()
{
	RotationComponentModeBase::INIT();

	m_rotationSpeed = Constant::k_rotationComponentConstantDefaultRotationSpeed;
}

void FWK::RotationComponentConstantModeBase::Deserialize(const nlohmann::json & a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::RotationComponentConstantModeBase::EditInspector()
{
	m_inspector.EditInspector(*this);
}

nlohmann::json FWK::RotationComponentConstantModeBase::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}