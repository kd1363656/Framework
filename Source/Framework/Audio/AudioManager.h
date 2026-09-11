#pragma once

namespace FWK
{
	class AudioManager final : public Utility::SingletonBase<AudioManager>
	{
	private:

		using SoundEffectMap = std::unordered_map<std::filesystem::path, std::unique_ptr<DirectX::SoundEffect>>;

		friend class SingletonBase<AudioManager>;

		 AudioManager();
		~AudioManager() override;

	public:

		void INIT();

		void LoadCONFIG     ();
		bool LoadSoundEffect(const std::filesystem::path& a_filePath);

		void Update();

		void Suspend();
		void Resume ();

		void SaveCONFIG() const;

		bool UnloadSoundEffect        (const std::filesystem::path& a_filePath);
		void ReleaseUnusedSoundEffects();


		bool PlayOneShot(const std::filesystem::path& a_filePath,
			             const float                  a_volume = Constant::k_defaultVolume,
			             const float                  a_pitch  = k_defaultPitch,
			             const float                  a_pan    = k_defaultPan);

		std::unique_ptr<DirectX::SoundEffectInstance> Create2DSoundEffectInstance(const std::filesystem::path& a_filePath);
		std::unique_ptr<DirectX::SoundEffectInstance> Create3DSoundEffectInstance(const std::filesystem::path& a_filePath);

		void Apply3D          (const DirectX::AudioListener& a_listener, const DirectX::AudioEmitter& a_emitter, DirectX::SoundEffectInstance& a_soundEffectInstance) const;
		void ApplyMasterVolume(const float a_volume);

		void RequestAudioDeviceReset();

		DirectX::AudioStatistics FetchVALStatistics() const;

		bool FetchVALIsAudioDevicePresent() const;

		float GetVALMasterVolume() const { return m_masterVolume; }

		bool GetVALWasAudioDeviceReset () const { return m_wasAudioDeviceReset; }

	private:

		bool CreateAudioEngine();

		inline static const std::filesystem::path k_configFileIOPath = "CONFIG/Audio/AudioCONFIG.json";

		static constexpr float k_minVolume     = 0.0F;
		static constexpr float k_maxVolume     = 1.0F;

		static constexpr float k_defaultPitch =  0.0F;
		static constexpr float k_minPitch     = -1.0F;
		static constexpr float k_maxPitch     =  1.0F;

		static constexpr float k_defaultPan =  0.0F;
		static constexpr float k_minPan     = -1.0F;
		static constexpr float k_maxPan     =  1.0F;

		static constexpr bool k_isRightHandedCoordinates = false;

		std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

		SoundEffectMap m_soundEffectMap;

		Converter::AudioManagerJsonConverter m_jsonConverter = {};

		float m_masterVolume;

		bool m_isAudioDeviceResetRequested;
		bool m_wasAudioDeviceReset;
	};
}