#include "RotationComponentConstantModeBase.h"

void FWK::RotationComponentConstantModeBase::INIT()
{
	RotationComponentModeBase::INIT();
}

void FWK::RotationComponentConstantModeBase::Deserialize(const nlohmann::json & a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

}

void FWK::RotationComponentConstantModeBase::EditInspector()
{
	RotationComponentModeBase::EditInspector();
}

nlohmann::json FWK::RotationComponentConstantModeBase::Serialize()
{
	return nlohmann::json();
}