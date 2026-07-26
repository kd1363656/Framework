#pragma once

namespace FWK::Struct
{
	struct CBModelCascadeShadowPass final
	{
		TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;
	};
}