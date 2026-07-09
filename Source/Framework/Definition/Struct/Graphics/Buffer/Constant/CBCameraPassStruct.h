#pragma once

namespace FWK::Struct
{	
	struct CBCameraPass final
	{
		static constexpr float k_defaultNearClip        = 0.01F;
		static constexpr float k_defaultFarClip         = 2000.0F;
		static constexpr float k_defaultTangentHalfFOVX = 0.0F;
		static constexpr float k_defaultTangentHalfFOVY = 0.0F;

		TypeAlias::Math::Matrix m_viewMatrix	       = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_projectionMatrix     = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_viewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

		float m_nearClip    = k_defaultNearClip;
		float m_farClip     = k_defaultFarClip;
		float m_tanHalfFOVX = k_defaultTangentHalfFOVX;
		float m_tanHalfFOVY = k_defaultTangentHalfFOVY;

		TypeAlias::Math::Vector3 m_cameraWorldPosition = TypeAlias::Math::Vector3::Zero;

		float m_padding = 0.0F;
	};
}