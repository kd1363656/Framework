#pragma once

namespace FWK::Graphics
{
    // 一つ一つのスケルタルアニメーションモデルのanimation進捗度などを管理するクラス
    class SkeletalAnimationPlayer final
    {
    private:

        struct SkinnedVertexBufferElement final
        {
            TypeAlias::Math::Vector3 m_position       = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector3 m_normal         = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector4 m_tangent        = TypeAlias::Math::Vector4::Zero;
            TypeAlias::Math::Vector2 m_uv             = TypeAlias::Math::Vector2::Zero;
            TypeAlias::Math::Vector3 m_smoothedNormal = TypeAlias::Math::Vector3::Zero;
        };

    public:

         SkeletalAnimationPlayer() = default;
        ~SkeletalAnimationPlayer() = default;

        SkeletalAnimationPlayer(const SkeletalAnimationPlayer&)           = delete;
        SkeletalAnimationPlayer(      SkeletalAnimationPlayer&&) noexcept = default;

        SkeletalAnimationPlayer& operator=(const SkeletalAnimationPlayer&)           = delete;
        SkeletalAnimationPlayer& operator=(      SkeletalAnimationPlayer&&) noexcept = default;

        bool Create(const SkeletalAnimationModel& a_skeletalAnimationModel);

        bool PlayMotion(const std::uint32_t a_motionIndex, const float a_playbackSpeed, const bool a_isLoop);

        void AdvanceTime(const float a_deltaTime);

        void Stop();

        bool IsAnimationEnd() const;

        bool ApplyAnimation(const Struct::SkeletalAnimationPlayerAnimation& a_animation);

        const Struct::SkeletalAnimationPlayerFrameData* FindPTRCurrentFrameData() const;

        Struct::SkeletalAnimationPlayerFrameData* FindMutablePTRCurrentFrameData();

        float FetchVALBlendWeight() const;

        const auto& GetREFFrameDataList() const { return m_frameDataList; }

        const auto& GetREFSkeletalAnimationModelRecord() const { return m_skeletalAnimationModelRecord; }

        const auto& GetREFAnimation() const { return m_animation; }

        const auto& GetREFBlendTargetAnimation() const { return m_blendTargetAnimation; }

        float GetVALAnimationTimeSecond() const { return m_animationTimeSecond; }

        float GetVALBlendTargetAnimationTimeSecond() const { return m_blendTargetAnimationTimeSecond; }

        bool GetVALIsBlending() const { return m_isBlending; }

        static constexpr float k_initialBlendWeight = 0.0F;

    private:

        bool EvaluateCurrentPose();

        float CalculateAdvancedTimeSecond(const Struct::SkeletalAnimationPlayerAnimation& a_animation, const float a_timeSecond, const float a_deltaTime) const;

        void CompleteAnimationBlend();

        void ResetPlaybackState();

        float FetchVALMotionDurationSecond(const Struct::SkeletalAnimationPlayerAnimation& a_animation) const;

        static constexpr float k_initialBlendElapsedSecond = 0.0F;
        static constexpr float k_completeBlendWeight       = 1.0F;
        static constexpr float k_stoppedPlaybackSpeed      = 0.0F;

        std::vector<Struct::SkeletalAnimationPlayerFrameData> m_frameDataList = {};

        std::weak_ptr<SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = {};

        SkeletalAnimationPoseEvaluator m_poseEvaluator = {};

        Struct::SkeletalAnimationPlayerAnimation m_animation            = {};
        Struct::SkeletalAnimationPlayerAnimation m_blendTargetAnimation = {};

        float m_animationTimeSecond            = Constant::k_initialAnimationTimeSecond;
        float m_blendTargetAnimationTimeSecond = Constant::k_initialAnimationTimeSecond;

        float m_blendElapsedSecond = k_initialBlendElapsedSecond;

        bool m_isBlending = false;
    };
}