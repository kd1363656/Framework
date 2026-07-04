#include "PhysicsCharacterVirtualRegistryJsonConverter.h"

void FWK::Converter::PhysicsCharacterVirtualRegistryJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsCharacterVirtualRegistry& a_physicsCharacterVirtualRegistry) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_storageIDAllocatorJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_storageIDAllcator = a_physicsCharacterVirtualRegistry.GetMutableREFStorageIDAllocator();

		l_storageIDAllcator.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::PhysicsCharacterVirtualRegistryJsonConverter::Serialize(const Physics::PhysicsCharacterVirtualRegistry & a_physicsCharacterVirtualRegistry) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_storageIDAllcator = a_physicsCharacterVirtualRegistry.GetREFStorageIDAllocator();

	l_rootJson[k_storageIDAllocatorJsonKey] = l_storageIDAllcator.Serialize();

	return l_rootJson;
}