#pragma once

namespace FWK::Graphics
{
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

		struct LocalTransform final
		{
			TypeAlias::Math::Vector3    m_scale       = TypeAlias::Math::Vector3::One;
			TypeAlias::Math::Quaternion m_rotation    = TypeAlias::Math::Quaternion::Identity;
			TypeAlias::Math::Vector3    m_translation = TypeAlias::Math::Vector3::Zero;
		};

	public:

		struct Animation final
		{
			static constexpr float k_defaultPlaybackSpeed       = 1.0F;
			static constexpr float k_initialBlendDurationSecond = 0.0F;
			
			static constexpr std::uint32_t k_invalidMotionIndex = std::numeric_limits<std::uint32_t>::max();

			float m_startTimeSecond = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;

			float m_playbackSpeed = k_defaultPlaybackSpeed;

			float m_blendDurationSecond = k_initialBlendDurationSecond;

			std::uint32_t m_motionIndex = k_invalidMotionIndex;

			bool m_isLoop = false;
		};

		struct FrameData final
		{
			DynamicRWStructuredBuffer m_boneMatrixBuffer = {};

			std::vector<DynamicRWStructuredBuffer> m_skinnedVertexBufferList = {};

			std::vector<TypeAlias::Math::Matrix> m_globalBoneMatrixList = {};
		};

	public:

		 SkeletalAnimationPlayer() = default;
		~SkeletalAnimationPlayer() = default;

		SkeletalAnimationPlayer(const SkeletalAnimationPlayer&)           = delete;
		SkeletalAnimationPlayer(      SkeletalAnimationPlayer&&) noexcept = default;

		SkeletalAnimationPlayer& operator=(const SkeletalAnimationPlayer&)           = delete;
		SkeletalAnimationPlayer& operator=(      SkeletalAnimationPlayer&&) noexcept = default;

		bool Create(const SkeletalAnimationModel& a_skeletalAnimationModel);
		
		bool PlayMotion(const std::uint32_t a_motionIndex, const bool a_isLoop, const float a_playbackSpeed);

		void AdvanceTime(const float a_deltaTime);

		bool IsAnimationEnd() const;

		void Stop();

		bool ApplyAnimation(const Animation& a_animation);

		const FrameData* FetchPTRCurrentFrameData() const;

		FrameData* FetchMutablePTRCurrentFrameData();

		float FetchVALBlendWeight() const;

		const auto& GetREFSkeletalAnimationModelRecord() const { return m_skeletalAnimationModelRecord; }

		const auto& GetREFAnimation() const { return m_animation; }

		const auto& GetREFBlendTargetAnimation() const { return m_blendTargetAnimation; }

		float GetVALAnimationTimeSecond() const { return m_animationTimeSecond; }

		float GetVALBlendTargetAnimationTimeSecond() const { return m_blendTargetAnimationTimeSecond; }

		bool GetVALIsBlending() const { return m_isBlending; }

		static constexpr float k_initialBlendWeight = 0.0F;

	private:

		bool                    CreateAnimationLookupData(      SkeletalAnimationModelRecord::ModelData& a_modelData, std::vector<LocalTransform>& a_bindPoseLocalTransformList, std::vector<std::vector<std::uint32_t>>& a_boneMotionTrackIndexList) const;
		TypeAlias::Math::Matrix CreateLocalMatrix        (const LocalTransform&                          a_localTransform)                                                                                                                            const;

		
		bool EvaluateCurrentPose();

		LocalTransform SampleLocalTransform(const SkeletalAnimationModelRecord::ModelMotionSequence& a_motionSequence,
			                                const float                                              a_timeSecond,
	                                        const std::uint32_t                                      a_motionIndex,
	                                        const std::uint32_t                                      a_boneIndex);

		LocalTransform InterpolateLocalTransform(const LocalTransform& a_startLocalTransform, const LocalTransform& a_endLcoalTransform, const float a_interpolationWeight) const;

		float CalculateAdvancedTimeSecond(const Animation& a_animation, const float a_timeSecond, const float a_deltaTime) const;

		void CompleteAnimationBlend();

		void ResetPlaybackState();

		float FetchMotionDurationSecond(const Animation& a_animation) const;

		static constexpr float k_initialBlendElapsedSecond = 0.0F;

		static constexpr float k_completeBlendWeight = 1.0F;

		static constexpr float k_stoppedPlaybackSpeed = 0.0F;

		static constexpr float k_minKeyFrameTimeRange = 0.000001F;

		static constexpr std::size_t k_firstBoneIndex            = 0ULL;
		static constexpr std::size_t k_firstMotionIndex          = 0ULL;
		static constexpr std::size_t k_firstBoneMotionTrackIndex = 0ULL;
		static constexpr std::size_t k_firstKeyFrameIndex        = 0ULL;
		static constexpr std::size_t k_singleKeyFrameCount       = 1ULL;
		static constexpr std::size_t k_nextKeyFrameOffset        = 1ULL;

		static constexpr std::size_t k_binarySearchPartitionCount = 2ULL;

		static constexpr std::uint32_t k_invalidBoneMotionTrackIndex = std::numeric_limits<std::uint32_t>::max();

		std::vector<std::vector<std::uint32_t>> m_boneMotionTrackIndexList = {};

		std::vector<FrameData> m_frameDataList = {};

		std::vector<LocalTransform> m_bindPoseLocalTransformList = {};

		std::weak_ptr<SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = {};
		
		Animation m_animation            = {};
		Animation m_blendTargetAnimation = {};

		float m_animationTimeSecond            = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;
		float m_blendTargetAnimationTimeSecond = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;

		float m_blendElapsedSecond = k_initialBlendElapsedSecond;

		bool m_isBlending = false;
	};
}