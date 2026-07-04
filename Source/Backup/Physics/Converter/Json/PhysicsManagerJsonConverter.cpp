#include "PhysicsManagerJsonConverter.h"

void FWK::Converter::PhysicsManagerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsManager& a_physicsManager) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_physicsCharacterVirtualRegistryJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_characterVirtualRegistry = a_physicsManager.GetMutableREFCharacterVirtualRegistry();

		l_characterVirtualRegistry.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::PhysicsManagerJsonConverter::Serialize(const Physics::PhysicsManager & a_physicsManager) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_characterVirtualRegistry = a_physicsManager.GetREFCharacterVirtualRegistry();

	l_rootJson[k_physicsCharacterVirtualRegistryJsonKey] = l_characterVirtualRegistry.Serialize();

	return l_rootJson;
}