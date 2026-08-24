#pragma once

namespace FWK
{
	class CameraComponent;
}

namespace FWK
{
	class CameraComponentInspector final
	{
	public:

		 CameraComponentInspector() = default;
		~CameraComponentInspector() = default;

		void EditInspector(CameraComponent& a_cameraComponent) const;

	private:

		static constexpr std::string_view k_aspectoRatioLabel = "アスペクト比 : %.2f";
		static constexpr std::string_view k_fovYDegreeLabel   = "視野角";
		static constexpr std::string_view k_farClipLabel      = "ファークリップ";
		static constexpr std::string_view k_nearClipLabel     = "ニアークリップ";
	};
}