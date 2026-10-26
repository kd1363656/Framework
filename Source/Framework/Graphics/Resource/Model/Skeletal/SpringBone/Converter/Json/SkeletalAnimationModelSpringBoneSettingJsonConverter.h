#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelSpringBoneSetting;
}

namespace FWK::Converter
{
    // SkeletalAnimationModelSpringBoneSettingをjsonへ変換するクラス
    // (エディターのUndo/Redoでは、編集前と編集後の設定をこのjsonで丸ごと覚えておく)
    class SkeletalAnimationModelSpringBoneSettingJsonConverter final
    {
    public:

         SkeletalAnimationModelSpringBoneSettingJsonConverter() = default;
        ~SkeletalAnimationModelSpringBoneSettingJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const;

        nlohmann::json Serialize(const Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const;

    private:

        Struct::ModelSpringBoneChainSetting    DeserializeChainSetting   (const nlohmann::json& a_chainSettingJson)    const;
        Struct::ModelSpringBoneColliderSetting DeserializeColliderSetting(const nlohmann::json& a_colliderSettingJson) const;

        TypeAlias::Math::Vector3 DeserializeVector3OrDefault(const nlohmann::json& a_json, const std::string_view a_key, const TypeAlias::Math::Vector3& a_defaultValue) const;

        nlohmann::json SerializeChainSetting   (const Struct::ModelSpringBoneChainSetting&    a_chainSetting)    const;
        nlohmann::json SerializeColliderSetting(const Struct::ModelSpringBoneColliderSetting& a_colliderSetting) const;

        static constexpr std::string_view k_chainSettingListJsonKey      = "ChainSettingList";
        static constexpr std::string_view k_colliderSettingListJsonKey   = "ColliderSettingList";
        static constexpr std::string_view k_prewarmIterationCountJsonKey = "PrewarmIterationCount";

        static constexpr std::string_view k_rootBoneNameListJsonKey         = "RootBoneNameList";
        static constexpr std::string_view k_gravityDirectionJsonKey         = "GravityDirection";
        static constexpr std::string_view k_stiffnessJsonKey                = "Stiffness";
        static constexpr std::string_view k_dragJsonKey                     = "Drag";
        static constexpr std::string_view k_gravityPowerJsonKey             = "GravityPower";
        static constexpr std::string_view k_hitRadiusJsonKey                = "HitRadius";
        static constexpr std::string_view k_maxAngularVelocityDegreeJsonKey = "MAXAngularVelocityDegree";
        static constexpr std::string_view k_endLengthJsonKey                = "EndLength";
        static constexpr std::string_view k_adjacentLinkStiffnessJsonKey    = "AdjacentLinkStiffness";
        static constexpr std::string_view k_isEnabledJsonKey                = "IsEnabled";
        static constexpr std::string_view k_isAdjacentChainLinkedJsonKey    = "IsAdjacentChainLinked";
        static constexpr std::string_view k_isAdjacentChainLoopedJsonKey    = "IsAdjacentChainLooped";

        static constexpr std::string_view k_boneNameJsonKey   = "BoneName";
        static constexpr std::string_view k_offsetJsonKey     = "Offset";
        static constexpr std::string_view k_tailOffsetJsonKey = "TailOffset";
        static constexpr std::string_view k_shapeJsonKey      = "Shape";
        static constexpr std::string_view k_radiusJsonKey     = "Radius";
    };
}