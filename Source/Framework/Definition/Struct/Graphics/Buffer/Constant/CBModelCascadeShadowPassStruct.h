#pragma once

namespace FWK::Struct
{
	struct CBModelCascadeShadowPass final
	{
		TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_viewMatrix           = TypeAlias::Math::Matrix::Identity;

		TypeAlias::Math::Vector3 m_orthographicsMIN = TypeAlias::Math::Vector3::Zero;
		float                    m_orthographicMINPadding = 0.0F;

		TypeAlias::Math::Vector3 m_orthographicMAX        = TypeAlias::Math::Vector3::Zero;
		float                    m_orthographicMAXPadding = 0.0F;

		TypeAlias::Math::Vector3 m_lightDirection        = TypeAlias::Math::Vector3::Zero;
		float                    m_lightDirectionPadding = 0.0F;
	};
}