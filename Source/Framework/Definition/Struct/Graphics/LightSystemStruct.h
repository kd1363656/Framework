#pragma once

namespace FWK::Struct
{
	// 平行光源
	struct DirectionalLight final
	{
		TypeAlias::Math::Vector3 m_direction = Constant::k_defaultDirectionalLightDirection;
		float					 m_intensity = Constant::k_defaultDirectionalLightIntensity;

		TypeAlias::Math::Vector3 m_color = Constant::k_defaultDirectionalLightColor;
	};

	// 環境光
	struct AmbientLight final
	{
		TypeAlias::Math::Vector3 m_color	 = Constant::k_defaultAmbientLightColor;
		float					 m_intensity = Constant::k_defaultAmbientLightIntensity;
	};
}