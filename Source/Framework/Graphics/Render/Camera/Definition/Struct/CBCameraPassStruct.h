#pragma once

namespace FWK::Struct
{	
	struct CBCameraPass final
	{
		TypeAlias::Math::Matrix m_viewMatrix	       = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_projectionMatrix     = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

		TypeAlias::Math::Matrix m_debugViewMatrix	        = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_debugProjectionMatrix     = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_debugViewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

		float m_nearClip    = Constant::k_defaultNearClip;
		float m_farClip     = Constant::k_defaultFarClip;
		float m_tanHalfFOVX = Constant::k_defaultTangentHalfFOVX;
		float m_tanHalfFOVY = Constant::k_defaultTangentHalfFOVY;
	};
}