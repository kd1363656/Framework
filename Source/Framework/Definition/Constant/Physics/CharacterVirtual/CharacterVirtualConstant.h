#pragma once

namespace FWK::Constant
{
	inline constexpr float k_defaultPhysicsCharacterCapsuleHalfHeightOfCylinder = 0.75F;

	inline constexpr float k_defaultPhysicsCharacterCapsuleRadius              = 0.35F;
	inline constexpr float k_defaultPhysicsCharacterMass                       = 70.0F;
	inline constexpr float k_defaultPhysicsCharacterMaxPushingForceRigidBody   = 100.0F;
	inline constexpr float k_defaultPhysicsCharacterMaxSlopeAngleRadians       = DirectX::XMConvertToRadians(50.0F);
	inline constexpr float k_defaultPhysicsCharacterPadding	                   = 0.02F;
	inline constexpr float k_defaultPhysicsCharacterPredictiveContactDistance  = 0.1F;
	inline constexpr float k_defaultPhysicsCharacterPenetrationRecoverySpeed   = 1.0F;
	inline constexpr float k_defaultPhysicsCharacterWalkStairsStepUpHeight     = 0.4F;
	inline constexpr float k_defaultPhysicsCharacterStickToFloorStepDownLength = 0.5F;
	inline constexpr float k_defaultPhysicsCharacterJumpSpeed                  = 5.0F;

	inline constexpr float k_minPhysicsCharacterCapsuleHalfHeightOfCylinder = 0.0F;
	inline constexpr float k_minPhysicsCharacterCapsuleRadius               = 0.0F;
	inline constexpr float k_minPhysicsCharacterMass                        = 0.0F;
	inline constexpr float k_minPhysicsCharacterMaxPushingForceRigidBody    = 0.0F;
	inline constexpr float k_minPhysicsCharacterMaxSlopeAngleRadians        = 0.0F;
	inline constexpr float k_maxPhysicsCharacterMaxSlopeAngleRadians        = DirectX::XM_PIDIV2;
	inline constexpr float k_minPhysicsCharacterPadding				        = 0.0F;
	inline constexpr float k_minPhysicsCharacterPredictiveContactDistance   = 0.0F;
	inline constexpr float k_minPhysicsCharacterPenetrationRecoverySpeed    = 0.0F;
	inline constexpr float k_maxPhysicsCharacterPenetrationRecoverySpeed    = 1.0F;
	inline constexpr float k_minPhysicsCharacterWalkStairsStepUpHeight      = 0.0F;
	inline constexpr float k_minPhysicsCharacterStickToFloorStepDownLength  = 0.0F;
	inline constexpr float k_minPhysicsCharacterJumpSpeed				    = 0.0F;

	inline constexpr float k_physicsCharacterMovingTowardsGroundTolerance = 0.1F;

	inline constexpr std::uint64_t k_invalidPhysicsCharacterVirtualID = 0ULL;
	inline constexpr std::uint64_t k_initialPhysicsCharacterVirtualID = 1ULL;
}