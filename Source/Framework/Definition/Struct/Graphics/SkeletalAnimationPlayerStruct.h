#pragma once

namespace FWK::Struct
{
    struct SkeletalAnimationPlayerAnimation final
    {
        static constexpr float k_defaultPlaybackSpeed       = 1.0F;
        static constexpr float k_initialBlendDurationSecond = 0.0F;

        float m_startTimeSecond = Constant::k_initialAnimationTimeSecond;

        float m_playbackSpeed = k_defaultPlaybackSpeed;

        float m_blendDurationSecond = k_initialBlendDurationSecond;

        std::uint32_t m_motionIndex = Graphics::SkeletalAnimationPoseEvaluator::k_invalidMotionIndex;

        bool m_isLoop = false;
    };

    struct SkeletalAnimationPlayerFrameData final
    {
         SkeletalAnimationPlayerFrameData() = default;
        ~SkeletalAnimationPlayerFrameData() = default;

        SkeletalAnimationPlayerFrameData(const SkeletalAnimationPlayerFrameData&)          = delete;
        SkeletalAnimationPlayerFrameData(      SkeletalAnimationPlayerFrameData&& a_other) = default;

        SkeletalAnimationPlayerFrameData& operator=(const SkeletalAnimationPlayerFrameData&)           = delete;
        SkeletalAnimationPlayerFrameData& operator=(      SkeletalAnimationPlayerFrameData&&) noexcept = delete;

        std::vector<Graphics::DynamicRWStructuredBuffer> m_skinnedVertexBufferList = {};

        std::vector<Graphics::DynamicRWStructuredBuffer> m_meshletBoundsBufferList = {};
        std::vector<TypeAlias::Math::Matrix>             m_globalBoneMatrixList    = {};

        Graphics::DynamicRWStructuredBuffer m_boneMatrixBuffer = {};

        Graphics::SkeletalAnimationBoneMatrixBufferUploader m_boneMatrixBufferUploader = {};
    };
}