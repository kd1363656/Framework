#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelMorphSetting;
}

namespace FWK::Converter
{
    // SkeletalAnimationModelMorphSettingをjsonへ変換するクラス
    class SkeletalAnimationModelMorphSettingJsonConverter final
    {
    public:

         SkeletalAnimationModelMorphSettingJsonConverter() = default;
        ~SkeletalAnimationModelMorphSettingJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting) const;

        nlohmann::json Serialize(const Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting) const;

    private:

        static constexpr std::string_view k_targetSettingListJsonKey = "TargetSettingList";
        static constexpr std::string_view k_targetNameJsonKey        = "TargetName";
        static constexpr std::string_view k_lodProtectionJsonKey     = "LODProtection";
        static constexpr std::string_view k_defaultWeightJsonKey     = "DefaultWeight";
        static constexpr std::string_view k_minWeightJsonKey         = "MINWeight";
        static constexpr std::string_view k_maxWeightJsonKey         = "MAXWeight";
        static constexpr std::string_view k_isShadowEnabledJsonKey   = "IsShadowEnabled";
    };
}