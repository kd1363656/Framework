#pragma once

namespace FWK::Constant
{
	static constexpr TypeAlias::Math::Vector3 k_defaultDirectionalLightDirection = {  0.0F, 0.0F, 1.0F };
	static constexpr TypeAlias::Math::Vector3 k_defaultDirectionalLightColor     = {  1.0F, 1.0F, 1.0F };
	static constexpr TypeAlias::Math::Vector3 k_defaultAmbientLightColor	     = {  1.0F, 1.0F, 1.0F };

	static constexpr float k_defaultDirectionalLightIntensity = 1.0F;
	static constexpr float k_defaultAmbientLightIntensity	  = 0.25F;
}