#pragma once

namespace FWK::Constant
{
	inline constexpr float k_minCharacterVirtualDeltaTime				    = 0.0F;
	inline constexpr float k_minCharacterVirtualCapsuleHalfHeightOfCylinder = 0.0F;
	inline constexpr float k_minCharacterVirtualCapsuleRadius				= 0.0F;
	inline constexpr float k_minCharacterVirtualMaxSlopeAngleRadians        = 0.0F;
	inline constexpr float k_maxCharacterVirtualMaxSlopeAngleRadians        = DirectX::XM_PIDIV2;
	inline constexpr float k_minCharacterVirtualJumpSpeed                   = 0.0F;
	
	inline constexpr float k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder = 0.75F;
	inline constexpr float k_defaultCharacterVirtualCapsuleRadius			    = 0.35F;
	inline constexpr float k_defaultCharacterVirtualMaxSlopeAngleRadians        = DirectX::XMConvertToRadians(50.0F);

	inline constexpr float k_defaultCharacterVirtualJumpSpeed			     = 5.0F;
	inline constexpr float k_characterVirtualMovingTowardsGroundTolerance    = 0.1F;
	inline constexpr float k_characterVirtualShapeChangePenetrationSlopScale = 1.5F;
}