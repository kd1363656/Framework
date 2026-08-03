#pragma once

namespace FWK::Graphics
{
	class Camera final : public std::enable_shared_from_this<Camera>
	{
	public:

		 Camera() = default;
		~Camera() = default;

		void Setup(const TypeAlias::Math::Matrix& a_cameraMatrix,
				   const float					  a_aspectRatio,
				   const float					  a_fovYDegree  = k_defaultFOVYDegree,
				   const float					  a_farClip     = Constant::k_defaultFarClip,
				   const float					  a_nearClip    = Constant::k_defaultNearClip);

		void ApplyCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix);
		
		void ApplyProjectionMatrix(const float a_aspectRatio,
								   const float a_fovYDegree  = k_defaultFOVYDegree,
								   const float a_farClip     = Constant::k_defaultFarClip,
								   const float a_nearClip    = Constant::k_defaultNearClip);

		void ApplyProjectionMatrix(const TypeAlias::Math::Matrix& a_projectionMatrix);

		void ApplyPerspectiveAspectRatio(const float a_aspectRatio);

		const auto& GetREFCameraMatrix() const { return m_cameraMatrix; }
		
	private:

		void SetupPerspective(const TypeAlias::Math::Matrix& a_cameraMatrix,
							  const float					 a_aspectRatio,
							  const float					 a_fovYDegree  = k_defaultFOVYDegree,
							  const float					 a_farClip     = Constant::k_defaultFarClip,
							  const float					 a_nearClip    = Constant::k_defaultNearClip);

		void UpdateViewProjectionMatrix       ();
		void UpdatePerspectiveProjectionMatrix();

		void RegisterCBCameraPass          ();
		void RegisterToEditorViewportWindow();

		static constexpr float k_defaultFOVYDegree  = 60.0F;
		static constexpr float k_invalidAspectRatio = 0.0F;

		static constexpr float k_halfFOVScale = 0.5F;

		std::shared_ptr<Struct::CBCameraPass> m_cbCameraPass = nullptr;
		
		TypeAlias::Math::Matrix m_cameraMatrix = TypeAlias::Math::Matrix::Identity;
		
		float m_aspectRatio = k_invalidAspectRatio;
		float m_fovYDegree  = k_defaultFOVYDegree;
		float m_farClip     = Constant::k_defaultFarClip;
		float m_nearClip    = Constant::k_defaultNearClip;
	};
}