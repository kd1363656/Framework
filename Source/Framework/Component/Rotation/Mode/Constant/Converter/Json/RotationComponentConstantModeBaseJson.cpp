#include "RotationComponentConstantModeBaseJson.h"

void FWK::Converter::RotationComponentConstantModeBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, RotationComponentConstantModeBase& a_rotationComponentConstantModeBase) const
{
	if (a_rootJson.is_null()) { return; }

	const float l_rotationSpeed = a_rootJson.value(k_rotationSpeedJsonKey, Constant::k_rotationComponentConstantDefaultRotationSpeed);

	a_rotationComponentConstantModeBase.SetRotationSpeed(l_rotationSpeed);
}

nlohmann::json FWK::Converter::RotationComponentConstantModeBaseJsonConverter::Serialize(const RotationComponentConstantModeBase& a_rotationComponentConstantModeBase) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_rotationSpeedJsonKey] = a_rotationComponentConstantModeBase.GetVALRotationSpeed();

	return l_rootJson;
}