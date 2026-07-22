#pragma once

namespace FWK::Physics
{
	class PhysicsStaticSphereBody;
}

namespace FWK::Converter
{
	class PhysicsStaticSphereBodyJsonConverter
	{
	public:

		 PhysicsStaticSphereBodyJsonConverter() = default;
		~PhysicsStaticSphereBodyJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsStaticSphereBody& a_physicsStaticSphereBody) const;

		nlohmann::json Serialize(const Physics::PhysicsStaticSphereBody& a_physicsStaticSphereBody) const;

	private:

		static constexpr std::string_view k_radiusJsonKey = "Radius";		
	};
}