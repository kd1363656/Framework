#include "RotationComponentJsonConverter.h"

void FWK::Converter::RotationComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, RotationComponent& a_rotationComponent) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_rotationMode = a_rotationComponent.GetMutableREFRotationMode();

	Utility::DeserializeInstanceType<TypeAlias::RotatationComponentModeUniqueFactory>(a_rootJson, k_rotationModeJsonKey, l_rotationMode);

	if (const auto& l_json = a_rootJson.value(k_rotationModeDataJsonKey, nlohmann::json{});
		l_rotationMode &&
		!l_json.is_null())
	{
		l_rotationMode->Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::RotationComponentJsonConverter::SerializePrefab(const RotationComponent& a_rotationComponent) const
{
	nlohmann::json l_rootJson     = {};
	
	if (const auto& l_rotationMode = a_rotationComponent.GetREFRotationMode())
	{
		Utility::UpdateJson(l_rootJson, Utility::SerializeInstanceType(l_rotationMode, k_rotationModeJsonKey));
	
		l_rootJson[k_rotationModeDataJsonKey] = l_rotationMode->Serialize();
	}

	return l_rootJson;
}