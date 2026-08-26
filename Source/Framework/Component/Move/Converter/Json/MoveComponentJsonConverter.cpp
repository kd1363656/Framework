#include "MoveComponentJsonConverter.h"

void FWK::Converter::MoveComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, MoveComponent& a_moveComponent) const
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::MoveComponentJsonConverter::SerializePrefab(const MoveComponent& a_moveComponent) const
{
	return nlohmann::json();
}