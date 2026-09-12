#pragma once

namespace FWK
{
    class AudioManager final : public Utility::SingletonBase<AudioManager>
    {
    private:

        using SoundEffectMap = std::unordered_map<std::filesystem::path, std::shared_ptr<SoundEffect>>;

        friend class SingletonBase<AudioManager>;

         AudioManager();
        ~AudioManager() override;

    public:

        void INIT();

        void LoadCONFIG();

        void Update();

        void StopAllSound  () const;
        void PauseAllSound () const;
        void ResumeAllSound() const;

        void SaveCONFIG() const;

        void ApplyMasterVolume(const float a_volume);

        std::weak_ptr<SoundEffect> AddSoundEffect   (const std::filesystem::path&                  a_filePath);
        void                       AddEffectInstance(const std::weak_ptr<SoundEffectInstanceBase>& a_soundEffectInstanceBase);

        void ClearMappedSound();

        const auto& GetREFAudioEngine() const { return m_audioEngine; }

        float GetVALMasterVolume() const { return m_masterVolume; }

        bool GetVALWasAudioDeviceReset() const { return m_wasAudioDeviceReset; }

    private:

        bool CreateAudioEngine();

        void RemoveSoundEffectInstanceIfStopped();

        void Release();

        inline static const std::filesystem::path k_configFileIOPath = "CONFIG/Audio/AudioCONFIG.json";

        static constexpr std::size_t k_initialRemoveSoundEffectInstanceIndex   = 0ULL;
        static constexpr std::size_t k_soundEffectInstanceBeforeEndIndexOffset = 1ULL;

        static constexpr float k_minMasterVolume = 0.0F;
        static constexpr float k_maxMasterVolume = 1.0F;

        static constexpr bool k_isRightHandedCoordinates = false;

        SoundEffectMap m_soundEffectMap;

        std::vector<std::weak_ptr<SoundEffectInstanceBase>> m_soundEffectInstanceList;

        std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

        DirectX::AudioListener m_audioListener;

        Converter::AudioManagerJsonConverter m_jsonConverter;

        float m_masterVolume;

        bool m_isAudioDeviceResetRequested;
        bool m_wasAudioDeviceReset;
    };
}