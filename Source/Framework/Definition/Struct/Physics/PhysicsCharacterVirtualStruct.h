#pragma once

namespace FWK::Struct
{
	struct PhysicsCharacterVirtualHandle final
	{
		std::uint64_t m_characterVirtualID = Constant::k_invalidPhysicsCharacterVirtualID;

		bool m_isValid = false;
	};

	struct PhysicsCharacterVirtualCreateSetting final
	{
		TypeAlias::Math::Vector3 m_worldPosition = {};

		float m_capsuleHalfHeightOfCylinder = Constant::k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder;

		float m_capsuleRadius = Constant::k_defaultCharacterVirtualCapsuleRadius;

		float m_maxSlopeAngleRadians = Constant::k_defaultCharacterVirtualMaxSlopeAngleRadians;

		bool m_isEnhancedInternalEdgeRemovalDisabled = false;
	};

	struct PhysicsCharacterVirtualUpdateData final
	{
		TypeAlias::Math::Vector3 m_desiredVelocity = {};

		float m_jumpSpeed = Constant::k_defaultCharacterVirtualJumpSpeed;

		bool m_isJumpRequested = false;
	};
}