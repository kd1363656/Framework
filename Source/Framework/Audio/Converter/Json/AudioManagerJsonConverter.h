#pragma once

namespace FWK
{
    class AudioManager;
}

namespace FWK::Converter
{
    class AudioManagerJsonConverter final
    {
    public:

         AudioManagerJsonConverter() = default;
        ~AudioManagerJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, AudioManager& a_audioManager) const;

        nlohmann::json Serialize(const AudioManager& a_audioManager) const;

    private:

        static constexpr std::string_view k_masterVolumeJsonKey = "MasterVolume";
    };
}