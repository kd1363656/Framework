#pragma once

namespace FWK::Struct
{
	// 平行光源
	struct DirectionalLight final
	{
		static constexpr TypeAlias::Math::Vector3 k_defaultDirectionalLightDirection = {  0.0F, 0.0F, 1.0F };
		static constexpr TypeAlias::Math::Vector3 k_defaultDirectionalLightColor     = {  1.0F, 1.0F, 1.0F };
		
		static constexpr float k_defaultDirectionalLightIntensity = 1.0F;

		TypeAlias::Math::Vector3 m_direction = k_defaultDirectionalLightDirection;
		float					 m_intensity = k_defaultDirectionalLightIntensity;

		TypeAlias::Math::Vector3 m_color = k_defaultDirectionalLightColor;
	};

	// 環境光
	struct AmbientLight final
	{
		static constexpr TypeAlias::Math::Vector3 k_defaultAmbientLightColor = { 1.0F, 1.0F, 1.0F };

		static constexpr float k_defaultAmbientLightIntensity	  = 0.25F;

		TypeAlias::Math::Vector3 m_color	 = k_defaultAmbientLightColor;
		float					 m_intensity = k_defaultAmbientLightIntensity;
	};
}