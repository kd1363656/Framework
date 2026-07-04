#pragma once

namespace FWK::Constant
{
	inline constexpr float k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder = 0.75F;
	inline constexpr float k_defaultCharacterVirtualCapsuleRadius			    = 0.35F;
	inline constexpr float k_defaultCharacterVirtualMaxSlopeAngleRadans         = DirectX::XMConvertToRadians(50.0F);

	inline constexpr float k_defaultCharacterVirtualJumpSpeed			  = 5.0F;
	inline constexpr float k_characterVirtualMovingTowardsGroundTolerance = 0.1F;

	inline constexpr std::uint64_t k_invalidPhysicsCharacterVirtualID = std::numeric_limits<std::uint64_t>::max();
	inline constexpr std::uint64_t k_initialPhysicsCharacterVirtualID = 0ULL;
	inline constexpr std::uint64_t k_maxPhysicsCharacterVirtualID     = std::numeric_limits<std::uint64_t>::max();
}