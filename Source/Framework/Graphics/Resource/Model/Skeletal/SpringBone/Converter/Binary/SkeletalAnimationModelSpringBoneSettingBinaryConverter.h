#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelSpringBoneSetting;
}

namespace FWK::Converter
{
    // SkeletalAnimationModelSpringBoneSettingを.springファイルへ保存、読み込みするクラス
    // .springファイルはFBXと同じ場所・同じ名前で、拡張子だけを.springにしたファイル
    //
    // ファイルの並び
    //   SpringBoneSettingBinaryHeader
    //   チェーンのグループの数だけ : ChainSettingBinaryData → 根元のボーンの名前(m_rootBoneNameCount個、1つごとに文字数 → 文字)
    //   当たり判定の数だけ         : ColliderSettingBinaryData → ボーンの名前
    class SkeletalAnimationModelSpringBoneSettingBinaryConverter final : public BinaryConverterBase
    {
    private:

        // ※ ファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
        struct SpringBoneSettingBinaryHeader final
        {
            std::uint64_t m_fileSize              = k_emptyAssetFileSize;
            std::uint64_t m_chainSettingCount     = k_emptySettingCount;
            std::uint64_t m_colliderSettingCount  = k_emptySettingCount;
            std::uint32_t m_prewarmIterationCount = Constant::k_defaultModelSpringBonePrewarmIterationCount;
            std::uint32_t m_assetTypeID           = k_springBoneSettingAssetTypeID;
            std::uint16_t m_version               = k_springBoneSettingAssetVersion;
            std::uint16_t m_padding               = k_initialPadding;
            std::uint32_t m_paddingB              = k_initialPadding;
        };

        // チェーンのグループ1つ分の、名前以外の設定
        // ※ ファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
        struct ChainSettingBinaryData final
        {
            std::uint64_t m_rootBoneNameCount = k_emptySettingCount;

            TypeAlias::Math::Vector3 m_gravityDirection = Constant::k_defaultModelSpringBoneGravityDirection;

            float m_stiffness                = Constant::k_defaultModelSpringBoneStiffness;
            float m_drag                     = Constant::k_defaultModelSpringBoneDrag;
            float m_gravityPower             = Constant::k_defaultModelSpringBoneGravityPower;
            float m_hitRadius                = Constant::k_defaultModelSpringBoneHitRadius;
            float m_maxAngularVelocityDegree = Constant::k_defaultModelSpringBoneMAXAngularVelocityDegree;
            float m_endLength                = Constant::k_defaultModelSpringBoneEndLength;
            float m_adjacentLinkStiffness    = Constant::k_defaultModelSpringBoneAdjacentLinkStiffness;

            std::uint32_t m_isEnabled             = k_trueValue;
            std::uint32_t m_isAdjacentChainLinked = k_falseValue;
            std::uint32_t m_isAdjacentChainLooped = k_falseValue;
        };

        // 当たり判定1つ分の、名前以外の設定
        // ※ ファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
        struct ColliderSettingBinaryData final
        {
            std::uint64_t m_boneNameSize = k_emptyNameSize;

            TypeAlias::Math::Vector3 m_offset     = Constant::k_defaultModelSpringBoneColliderOffset;
            TypeAlias::Math::Vector3 m_tailOffset = Constant::k_defaultModelSpringBoneColliderTailOffset;

            Enum::ModelSpringBoneColliderShape m_shape = Enum::ModelSpringBoneColliderShape::Sphere;

            float m_radius = Constant::k_defaultModelSpringBoneColliderRadius;
        };

    public:

         SkeletalAnimationModelSpringBoneSettingBinaryConverter()          = default;
        ~SkeletalAnimationModelSpringBoneSettingBinaryConverter() override = default;

        // a_modelFilePath : モデル(FBX)のパス(.springファイルのパスはここから作る)
        // .springファイルが無い、又は壊れている場合はfalseを返す(設定は変えない)
        bool Load(Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::filesystem::path& a_modelFilePath);

        bool Save(const Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::filesystem::path& a_modelFilePath);

    private:

        bool TryReadChainSetting   (Struct::ModelSpringBoneChainSetting&    a_chainSetting,    std::uint64_t& a_memoryReadOffset) const;
        bool TryReadColliderSetting(Struct::ModelSpringBoneColliderSetting& a_colliderSetting, std::uint64_t& a_memoryReadOffset) const;

        void WriteChainSetting   (const Struct::ModelSpringBoneChainSetting&    a_chainSetting,    std::uint64_t& a_memoryWriteOffset) const;
        void WriteColliderSetting(const Struct::ModelSpringBoneColliderSetting& a_colliderSetting, std::uint64_t& a_memoryWriteOffset) const;

        std::uint64_t CalculateFileSize(const Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const;

        static constexpr std::uint32_t k_springBoneSettingAssetTypeID = 0x47525053U;

        static constexpr std::uint16_t k_springBoneSettingAssetVersion = 1U;

        static constexpr std::uint16_t k_initialPadding = 0U;

        static constexpr std::uint64_t k_emptySettingCount = 0ULL;
        static constexpr std::uint64_t k_emptyNameSize     = 0ULL;

        static constexpr std::uint32_t k_trueValue  = 1U;
        static constexpr std::uint32_t k_falseValue = 0U;
    };
}