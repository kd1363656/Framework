#pragma once

namespace FWK
{
	class AudioManager final : public Utility::SingletonBase<AudioManager>
	{
	private:

		friend class SingletonBase<AudioManager>;

		 AudioManager();
		~AudioManager() override;

	public:

		void INIT();

		void LoadCONFIG();
		
		void Update();

		void SaveCONFIG() const;

		void ApplyMasterVolume(const float a_volume);

		const auto& GetREFAudioEngine() const { return m_audioEngine; }

		float GetVALMasterVolume() const { return m_masterVolume; }

		bool GetVALWasAudioDeviceReset () const { return m_wasAudioDeviceReset; }

	private:

		bool CreateAudioEngine();

		inline static const std::filesystem::path k_configFileIOPath = "CONFIG/Audio/AudioCONFIG.json";

		static constexpr float k_minMasterVolume = 0.0F;
		static constexpr float k_maxMasterVolume = 1.0F;

		static constexpr bool k_isRightHandedCoordinates = false;

		std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

		Converter::AudioManagerJsonConverter m_jsonConverter = {};

		float m_masterVolume;

		bool m_isAudioDeviceResetRequested;
		bool m_wasAudioDeviceReset;
	};
}