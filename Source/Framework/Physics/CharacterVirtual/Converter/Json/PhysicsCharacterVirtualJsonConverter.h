#pragma once

namespace FWK::Physics
{
	class PhysicsCharacterVirtualBase;
}

namespace FWK::Converter
{
	class PhysicsCharacterVirtualBaseJsonConverter
	{
	public:

		PhysicsCharacterVirtualBaseJsonConverter() = default;
		~PhysicsCharacterVirtualBaseJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsCharacterVirtualBase& a_physicsCharacterVirtualBase) const;

		nlohmann::json Serialize(const Physics::PhysicsCharacterVirtualBase& a_physicsCharacterVirtualBase) const;

	private:

		static constexpr std::string_view k_capsuleHalfHeightOfCylinderJsonKey             = "CapsuleHalfHeightOfCylinder";
		static constexpr std::string_view k_capsuleRadiusJsonKey                           = "CapsuleRadius";
		static constexpr std::string_view k_maxSlopeRadiansJsonKey                         = "MaxSlopeAngleRadians";
		static constexpr std::string_view k_isEnhancedInternalEdgeRemovealDisabledeJsonKey = "IsEnhancedInternalEdgeRemovalDisabled";
	};
}