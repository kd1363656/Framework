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
				   const float					  a_fovYDegree  = Constant::k_cameraDefaultFOVYDegree,
				   const float					  a_farClip     = Constant::k_cameraDefaultFarClip,
				   const float					  a_nearClip    = Constant::k_cameraDefaultNearClip);

		void Setup();

		void ApplyCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix);
		
		void ApplyProjectionMatrix(const float a_aspectRatio,
								   const float a_fovYDegree  = Constant::k_cameraDefaultFOVYDegree,
								   const float a_farClip     = Constant::k_cameraDefaultFarClip,
								   const float a_nearClip    = Constant::k_cameraDefaultNearClip);

		void ApplyProjectionMatrix(const TypeAlias::Math::Matrix& a_projectionMatrix);

		void ApplyPerspectiveAspectRatio(const float a_aspectRatio);

		const auto& GetREFCameraMatrix() const { return m_cameraMatrix; }
		
		void SetCameraMatrix(const TypeAlias::Math::Matrix& a_set) { m_cameraMatrix = a_set; }

		void SetAspectRatio(const float a_set) { m_aspectRatio = a_set; }
		void SetFovYDegree (const float a_set) { m_fovYDegree  = a_set; }
		void SetFarClip    (const float a_set) { m_farClip     = a_set; }
		void SetNearClip   (const float a_set) { m_nearClip    = a_set; }

		float GetVALAspectRatio() const { return m_aspectRatio; }
		float GetVALFovYDegree () const { return m_fovYDegree; }
		float GetVALFarClip    () const { return m_farClip; }
		float GetVALNearClip   () const { return m_nearClip; }

	private:

		void SetupPerspective(const TypeAlias::Math::Matrix& a_cameraMatrix,
							  const float					 a_aspectRatio,
							  const float					 a_fovYDegree  = Constant::k_cameraDefaultFOVYDegree,
							  const float					 a_farClip     = Constant::k_cameraDefaultFarClip,
							  const float					 a_nearClip    = Constant::k_cameraDefaultNearClip);

		void UpdateViewProjectionMatrix       ();
		void UpdatePerspectiveProjectionMatrix();

		void RegisterCBCameraPass          ();
		void RegisterToEditorViewportWindow();

		static constexpr float k_halfFOVScale = 0.5F;

		std::shared_ptr<Struct::CBCameraPass> m_cbCameraPass = nullptr;
		
		TypeAlias::Math::Matrix m_cameraMatrix = TypeAlias::Math::Matrix::Identity;
		
		float m_aspectRatio = Constant::k_cameraInvalidAspectRatio;
		float m_fovYDegree  = Constant::k_cameraDefaultFOVYDegree;
		float m_farClip     = Constant::k_cameraDefaultFarClip;
		float m_nearClip    = Constant::k_cameraDefaultNearClip;
	};
}