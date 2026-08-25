#include "RotationComponentJsonConverter.h"

void FWK::Converter::RotationComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, RotationComponent& a_rotationComponent) const
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::RotationComponentJsonConverter::SerializePrefab(const RotationComponent& a_rotationComponent) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}