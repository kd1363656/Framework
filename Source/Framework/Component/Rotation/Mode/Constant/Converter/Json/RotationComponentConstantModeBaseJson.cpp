#include "RotationComponentConstantModeBaseJson.h"

void FWK::Converter::RotationComponentConstantModeBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, RotationComponentConstantModeBase& a_moveComponentConstantModeBase) const
{
	if (a_rootJson.is_null()) { return; }

	a_moveComponentConstantModeBase.RotationComponentModeBase::Deserialize(a_rootJson);

	const float l_rotationSpeed = a_rootJson.value(k_rotationSpeedJsonKey, Constant::k_rotationComponentConstantDefaultRotationSpeed);

	a_moveComponentConstantModeBase.SetRotationSpeed(l_rotationSpeed);
}

nlohmann::json FWK::Converter::RotationComponentConstantModeBaseJsonConverter::Serialize(const RotationComponentConstantModeBase& a_moveComponentConstantModeBase) const
{
	nlohmann::json l_rootJson = a_moveComponentConstantModeBase.RotationComponentModeBase::Serialize();

	l_rootJson[k_rotationSpeedJsonKey] = a_moveComponentConstantModeBase.GetVALRotationSpeed();

	return l_rootJson;
}