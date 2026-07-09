#pragma once

namespace FWK::Graphics
{
	class Camera final
	{
	public:

		 Camera() = default;
		~Camera() = default;

		void Setup(const TypeAlias::Math::Matrix& a_cameraMatrix,
				   const float					  a_aspectRatio,
				   const float					  a_fovYDegree  = k_defaultFOVYDegree,
				   const float					  a_farClip     = Struct::CBCameraPass::k_defaultFarClip,
				   const float					  a_nearClip    = Struct::CBCameraPass::k_defaultNearClip);

		void ApplyCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix);
		
		void ApplyProjectionMatrix(const float a_aspectRatio,
								   const float a_fovYDegree  = k_defaultFOVYDegree,
								   const float a_farClip     = Struct::CBCameraPass::k_defaultFarClip,
								   const float a_nearClip    = Struct::CBCameraPass::k_defaultNearClip);

		void ApplyProjectionMatrix(const TypeAlias::Math::Matrix& a_projectionMatrix);

		const auto& GetREFCameraMatrix() const { return m_cameraMatrix; }
		
	private:

		void SetupPerspective(const TypeAlias::Math::Matrix& a_cameraMatrix,
							  const float					 a_aspectRatio,
							  const float					 a_fovYDegree  = k_defaultFOVYDegree,
							  const float					 a_farClip     = Struct::CBCameraPass::k_defaultFarClip,
							  const float					 a_nearClip    = Struct::CBCameraPass::k_defaultNearClip);

		void UpdateViewProjectionMatrix();

		void RegisterCameraPassConstantBufferSource();

		static constexpr float k_defaultFOVYDegree  = 60.0F;
		static constexpr float k_invalidAspectRatio = 0.0F;

		static constexpr float k_halfFOVScale = 0.5F;

		std::shared_ptr<Struct::CBCameraPass> m_cbCameraPass = nullptr;
		
		TypeAlias::Math::Matrix m_cameraMatrix      = TypeAlias::Math::Matrix::Identity;
		TypeAlias::Math::Matrix m_debugCameraMatrix = TypeAlias::Math::Matrix::Identity;
	};
}