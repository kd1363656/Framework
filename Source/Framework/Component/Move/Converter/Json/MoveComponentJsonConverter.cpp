#include "MoveComponentJsonConverter.h"

void FWK::Converter::MoveComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, MoveComponent& a_moveComponent) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_moveMode = a_moveComponent.GetMutableREFMoveMode();

	Utility::DeserializeInstanceType<TypeAlias::MoveComponentModeUniqueFactory>(a_rootJson, k_moveModeJsonKey, l_moveMode);

	if (const auto& l_json = a_rootJson.value(k_moveModeDataJsonKey, nlohmann::json{});
		l_moveMode &&
		!l_json.is_null())
	{
		l_moveMode->Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::MoveComponentJsonConverter::SerializePrefab(const MoveComponent& a_moveComponent) const
{
	nlohmann::json l_rootJson = {};

	if (const auto& l_moveMode = a_moveComponent.GetREFMoveMode())
	{
		Utility::UpdateJson(l_rootJson, Utility::SerializeInstanceType(l_moveMode, k_moveModeJsonKey));

		l_rootJson[k_moveModeDataJsonKey] = l_moveMode->Serialize();
	}

	return l_rootJson;
}