#pragma once

namespace FWK::Constant
{
	inline constexpr float k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder = 0.75F;
	inline constexpr float k_defaultCharacterVirtualCapsuleRadius			    = 0.35F;
	inline constexpr float k_defaultCharacterVirtualMaxSlopeAngleRadians        = DirectX::XMConvertToRadians(50.0F);

	inline constexpr float k_defaultCharacterVirtualJumpSpeed			  = 5.0F;
	inline constexpr float k_characterVirtualMovingTowardsGroundTolerance = 0.1F;
}