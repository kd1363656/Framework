#pragma once

namespace FWK
{
	class FPSController final
	{
	public:

		 FPSController() = default;
		~FPSController() = default;

		void LoadCONFIG    ();
		void PostLoadCONFIG();

		void BeginFrame();
		void EndFrame  ();

		void SaveCONFIG() const;

		void SetTargetFPS(const float a_set) { m_targetFPS = a_set; }

		float GetVALScaledDeltaTime() const { return m_scaledDeltaTime; }

		float GetVALCurrentFPS() const { return m_currentFPS; }
		float GetVALTargetFPS () const { return m_targetFPS; }

		static constexpr float k_defaultFPS = 60.0F;

		static constexpr float k_initialDeltaTime = 0.0F;
		static constexpr float k_minDeltaTime     = 0.0F;

	private:

		void LimitFramerate() const;
		
		static constexpr float k_initialFPS       = 0.0F;
		static constexpr float k_defaultTimeScale = 1.0F;

		static constexpr float k_second                 = 1.0F;
		static constexpr float k_milliSecond            = 1000.0F;
		static constexpr float k_initialScaledDeltaTime = 0.0F;
		
		const std::filesystem::path k_configFileIOPath = "Asset/Data/CONFIG/FPS/FPSCONFIG.json";

		Converter::FPSControllerJsonConverter m_jsonConverter = {};
		
		std::chrono::steady_clock::time_point m_previousTime   = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_frameBeginTime = std::chrono::steady_clock::now();

		float m_timeScale       = k_defaultTimeScale;
		float m_deltaTime       = k_initialDeltaTime;
		float m_scaledDeltaTime = k_initialScaledDeltaTime;

		float m_currentFPS = k_initialFPS;
		float m_targetFPS  = k_defaultFPS;
	};
}