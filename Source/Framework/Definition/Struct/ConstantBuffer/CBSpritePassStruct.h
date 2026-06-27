#pragma once

namespace FWK::Struct
{
	struct CBSpritePass final
	{
		TypeAlias::Math::Matrix m_projectionMatrix = TypeAlias::Math::Matrix::Identity;
	};
}