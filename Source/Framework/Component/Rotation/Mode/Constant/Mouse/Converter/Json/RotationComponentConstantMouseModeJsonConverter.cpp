#include "RotationComponentConstantMouseModeJsonConverter.h"

void FWK::Converter::RotationComponentConstantMouseModeJsonConverter::Deserialize(const nlohmann::json& a_rootJson, RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode) const
{
	if (a_rootJson.is_null()) { return; }

	a_rotationComponentConstantMouseMode.RotationComponentConstantModeBase::Deserialize(a_rootJson);
}

nlohmann::json FWK::Converter::RotationComponentConstantMouseModeJsonConverter::Serialize(const RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode) const
{
	auto l_rootJson = a_rotationComponentConstantMouseMode.RotationComponentConstantModeBase::Serialize();

	return l_rootJson;
}