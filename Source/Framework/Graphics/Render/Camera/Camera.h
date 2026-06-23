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
							  const float					 a_farClip     = Constant::k_defaultFarClip,
							  const float					 a_nearClip    = Constant::k_defaultNearClip);

		void SetCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix);
		
		void SetProjectionMatrix(const float a_aspectRatio,
								 const float a_fovYDegree  = k_defaultFOVYDegree,
								 const float a_farClip     = Constant::k_defaultFarClip,
								 const float a_nearClip    = Constant::k_defaultNearClip);

		void SetProjectionMatrix(const TypeAlias::Math::Matrix& a_projectionMatrix);

		const auto& GetREFCameraMatrix() const { return m_cameraMatrix; }
		
	private:

		void UpdateViewProjectionMatrix();

		void SyncCameraPassDrawRequest();

		static constexpr float k_defaultFOVYDegree  = 60.0F;
		static constexpr float k_invalidAspectRatio = 0.0F;

		static constexpr float k_halfFOVScale = 0.5F;

		std::shared_ptr<Struct::CBCameraPass> m_cbCameraPass = nullptr;
		
		TypeAlias::Math::Matrix m_cameraMatrix      = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_debugCameraMatrix = TypeAlias::Math::Matrix::Identity;
	};
}