#pragma once

namespace FWK::Struct
{	
	struct CBCameraPass final
	{
		TypeAlias::Math::Matrix m_viewMatrix	       = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_projectionMatrix     = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;
	};
}