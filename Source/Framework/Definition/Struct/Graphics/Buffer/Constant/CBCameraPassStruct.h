#pragma once

namespace FWK::Struct
{	
	struct CBCameraPass final
	{
		TypeAlias::Math::Matrix m_viewMatrix	       = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_projectionMatrix     = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

		float m_nearClip    = Constant::k_cameraDefaultNearClip;
		float m_farClip     = Constant::k_cameraDefaultFarClip;
		float m_tanHalfFOVX = Constant::k_cameraDefaultTangentHalfFOVX;
		float m_tanHalfFOVY = Constant::k_cameraDefaultTangentHalfFOVY;

		TypeAlias::Math::Vector3 m_cameraWorldPosition = TypeAlias::Math::Vector3::Zero;

		float m_padding = 0.0F;
	};
}