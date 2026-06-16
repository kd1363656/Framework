#pragma once

namespace FWK::Graphics
{
	class Camera final
	{
	public:

		 Camera() = default;
		~Camera() = default;

		void SetupPerspective(const TypeAlias::Math::Matrix& a_cameraMatrix,
							  const float					 a_aspectRatio,
							  const float					 a_fovYDegree  = k_defaultFOVYDegree,
							  const float					 a_farClip     = k_defaultFarClip,
							  const float					 a_nearClip    = k_defaultNearClip);

		void SetCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix);

		void SetProjectionMatrix(const float a_aspectRatio,
								 const float a_fovYDegree  = k_defaultFOVYDegree,
								 const float a_farClip     = k_defaultFarClip,
								 const float a_nearClip    = k_defaultNearClip);

		void SetProjectionMatrix(const TypeAlias::Math::Matrix& a_projectionMatrix);

		void SyncCameraPassDrawRequest();
		
		const auto& GetREFCameraMatrix() const { return m_cameraMatrix; }
		
	private:

		void UpdateViewProjectionMatrix();

		static constexpr float k_defaultFOVYDegree  = 60.0F;
		static constexpr float k_defaultNearClip    = 0.01F;
		static constexpr float k_defaultFarClip     = 2000.0F;
		static constexpr float k_invalidAspectRatio = 0.0F;

		std::shared_ptr<Struct::CBCameraPass> m_cbCameraPass = nullptr;
		
		TypeAlias::Math::Matrix m_cameraMatrix = TypeAlias::Math::Matrix::Identity;
	};
}