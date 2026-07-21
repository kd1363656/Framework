#pragma once

namespace FWK::Physics
{
	class PhysicsStaticCapsuleBody;
}

namespace FWK::Converter
{
	class PhysicsStaticCapsuleBodyJsonConverter
	{
	public:

		 PhysicsStaticCapsuleBodyJsonConverter() = default;
		~PhysicsStaticCapsuleBodyJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsStaticCapsuleBody& a_physicsStaticCapsuleBody) const;

		nlohmann::json Serialize(const Physics::PhysicsStaticCapsuleBody& a_physicsStaticCapsuleBody) const;

	private:

		static constexpr std::string_view k_halfHeightOfCylinderJsonKey = "HalfHeightOfCylinder";
		static constexpr std::string_view k_radiusJsonKey               = "Radius";
	};
}