#pragma once

namespace FWK::Graphics
{
    // 一つ一つのスケルタルアニメーションモデルのanimation進捗度などを管理するクラス
    class SkeletalAnimationPlayer final
    {
    private:

        struct SkinnedVertexBufferElement final
        {
            TypeAlias::Math::Vector3 m_position = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector3 m_normal   = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector4 m_tangent  = TypeAlias::Math::Vector4::Zero;
            TypeAlias::Math::Vector2 m_uv       = TypeAlias::Math::Vector2::Zero;
        };

    public:

        struct Animation final
        {
            float m_startTimeSecond = Constant::k_initialAnimationTimeSecond;

            float m_playbackSpeed = Constant::k_defaultAnimationPlaybackSpeed;

            float m_blendDurationSecond = Constant::k_initialAnimationBlendDurationSecond;

            std::uint32_t m_motionIndex = SkeletalAnimationPoseEvaluator::k_invalidMotionIndex;

            bool m_isLoop = false;
        };

        struct FrameData final
        {
             FrameData() = default;
            ~FrameData() = default;

            FrameData(const FrameData&)          = delete;
            FrameData(      FrameData&& a_other) = default;

            FrameData& operator=(const FrameData&)           = delete;
            FrameData& operator=(      FrameData&&) noexcept = delete;

            DynamicRWStructuredBuffer m_boneMatrixBuffer = {};

            SkeletalAnimationBoneMatrixBufferUploader m_boneMatrixBufferUploader = {};

            std::vector<DynamicRWStructuredBuffer> m_skinnedVertexBufferList = {};

            std::vector<DynamicRWStructuredBuffer> m_meshletBoundsBufferList = {};
            std::vector<TypeAlias::Math::Matrix>   m_globalBoneMatrixList    = {};
        };

    public:

         SkeletalAnimationPlayer() = default;
        ~SkeletalAnimationPlayer() = default;

        SkeletalAnimationPlayer(const SkeletalAnimationPlayer&)           = delete;
        SkeletalAnimationPlayer(      SkeletalAnimationPlayer&&) noexcept = default;

        SkeletalAnimationPlayer& operator=(const SkeletalAnimationPlayer&)           = delete;
        SkeletalAnimationPlayer& operator=(      SkeletalAnimationPlayer&&) noexcept = default;

        // 姿勢の計算に必要なCPU側のデータだけを作る(GPUのBufferはCreateGPUFrameDataで作る)
        bool Create(const SkeletalAnimationModel& a_skeletalAnimationModel);

        // 従来の方式で描画するためのGPUのBuffer(ボーン行列、1体ごとの変形後の頂点など)を作る
        // 既に作ってあれば何もしない(AdvanceTimeは、まだ作っていなければこの関数を呼ぶ)
        bool CreateGPUFrameData();

        bool PlayMotion(const std::uint32_t a_motionIndex, const bool a_isLoop, const float a_playbackSpeed);

        // 再生時刻を進め、その時刻の姿勢をCPUで計算する(揺れものがあれば、その姿勢を揺らす)
        // a_worldMatrix : 揺れものはワールド座標で計算するため、モデルのワールド行列を渡す
        void AdvanceTime        (const float a_deltaTime, const TypeAlias::Math::Matrix& a_worldMatrix);
        // 再生時刻だけを進め、姿勢は計算しない(ベイクの方式で描画している間に使う)
        void AdvancePlaybackTime(const float a_deltaTime);

        bool IsAnimationEnd() const;

        // 姿勢をその場で書き換える必要があるか(揺れものの設定が有効なモデル)
        // trueの場合は、ベイクの方式(事前計算した姿勢の表)では描画できない
        bool IsRuntimePoseRequired() const;

        void Stop();

        bool ApplyAnimation(const Animation& a_animation);

        const FrameData* FindPTRCurrentFrameData() const;

        FrameData* FindMutablePTRCurrentFrameData();

        float FetchVALBlendWeight() const;

        const auto& GetREFSkeletalAnimationModelRecord() const { return m_skeletalAnimationModelRecord; }

        const auto& GetREFAnimation() const { return m_animation; }

        const auto& GetREFBlendTargetAnimation() const { return m_blendTargetAnimation; }

        float GetVALAnimationTimeSecond() const { return m_animationTimeSecond; }

        float GetVALBlendTargetAnimationTimeSecond() const { return m_blendTargetAnimationTimeSecond; }

        bool GetVALIsBlending() const { return m_isBlending; }

    private:

        bool EvaluateCurrentPose();

        bool ApplySpringBone(const float a_deltaTime, const TypeAlias::Math::Matrix& a_worldMatrix);

        float CalculateAdvancedTimeSecond(const Animation& a_animation, const float a_timeSecond, const float a_deltaTime) const;

        void CompleteAnimationBlend();

        void ResetPlaybackState();

        float FetchMotionDurationSecond(const Animation& a_animation) const;

        static constexpr float k_initialBlendElapsedSecond = 0.0F;
        static constexpr float k_completeBlendWeight       = 1.0F;
        static constexpr float k_stoppedPlaybackSpeed      = 0.0F;

        std::vector<FrameData> m_frameDataList = {};

        std::weak_ptr<SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = {};

        SkeletalAnimationPoseEvaluator m_poseEvaluator = {};

        SkeletalAnimationSpringBoneSolver m_springBoneSolver = {};

        Animation m_animation            = {};
        Animation m_blendTargetAnimation = {};

        float m_animationTimeSecond            = Constant::k_initialAnimationTimeSecond;
        float m_blendTargetAnimationTimeSecond = Constant::k_initialAnimationTimeSecond;

        float m_blendElapsedSecond = k_initialBlendElapsedSecond;

        bool m_isBlending = false;
    };
}