#pragma once

namespace FWK::Struct
{
    struct StaticModelPerObjectDrawRequestData final
    {
        static constexpr float k_defaultWorldMaxScale = 0.0F;

        std::weak_ptr<Graphics::StaticModelRecord> m_staticModelRecord = {};

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        float m_worldMaxScale = k_defaultWorldMaxScale;
    };

    struct SkeletalAnimationModelPerObjectDrawRequestData final
    {
        std::weak_ptr<Graphics::SkeletalAnimationPlayer> m_skeletalAnimationPlayer = {};

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;
    };

    // 大量のキャラクター向けの、GPUスキニングで描画するSkeletalAnimationModelの描画申請
    // (SkeletalAnimationPlayerを使わず、モーション番号と再生時刻だけを持つ)
    struct SkeletalAnimationModelGPUSkinnedDrawRequestData final
    {
        static constexpr float k_initialAnimationTimeSecond = 0.0F;

        static constexpr std::uint32_t k_defaultMotionIndex = 0U;

        std::weak_ptr<Graphics::SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = {};

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        // 再生しているモーションの再生時刻(秒)
        float m_animationTimeSecond = k_initialAnimationTimeSecond;

        // 再生するモーションの番号
        std::uint32_t m_motionIndex = k_defaultMotionIndex;
    };
}