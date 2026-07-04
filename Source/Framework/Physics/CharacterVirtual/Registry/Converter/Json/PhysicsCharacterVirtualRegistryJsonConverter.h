#pragma once

namespace FWK::Physics
{
	class PhysicsCharacterVirtualRegistry;
}

namespace FWK::Converter
{
	class PhysicsCharacterVirtualRegistryJsonConverter final
	{
	public:

		 PhysicsCharacterVirtualRegistryJsonConverter() = default;
		~PhysicsCharacterVirtualRegistryJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsCharacterVirtualRegistry& a_physicsCharacterVirtualRegistry) const;

		nlohmann::json Serialize(const Physics::PhysicsCharacterVirtualRegistry& a_physicsCharacterVirtualRegistry)	const;

	private:

		static constexpr std::string_view k_storageIDAllocatorJsonKey = "StorageIDAllocator";
	};
}