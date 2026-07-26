#pragma once

namespace FWK::Struct
{
	struct CBCascadeShadowPass final
	{
		TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;
	};
}