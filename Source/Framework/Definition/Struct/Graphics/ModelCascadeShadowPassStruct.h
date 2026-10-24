#pragma once

namespace FWK::Struct
{
    struct StaticModelPerObjectDrawRequestData final
    {
        std::weak_ptr<Graphics::StaticModelRecord> m_staticModelRecord = {};

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        float m_worldMaxScale = Constant::k_defaultStaticModelDrawRequestWorldMaxScale;
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
        std::weak_ptr<Graphics::SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = {};

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        float m_animationTimeSecond = Constant::k_initialGPUSkinnedDrawRequestAnimationTimeSecond;

        std::uint32_t m_motionIndex = Constant::k_defaultGPUSkinnedDrawRequestMotionIndex;
    };
}