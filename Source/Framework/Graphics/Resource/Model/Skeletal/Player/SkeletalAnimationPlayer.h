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

	public:

		struct Animation final
		{
			static constexpr float k_defaultPlaybackSpeed       = 1.0F;
			static constexpr float k_initialBlendDurationSecond = 0.0F;
			
			float m_startTimeSecond = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;

			float m_playbackSpeed = k_defaultPlaybackSpeed;

			float m_blendDurationSecond = k_initialBlendDurationSecond;

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

		const FrameData* FindPTRCurrentFrameData() const;

		FrameData* FindMutablePTRCurrentFrameData();

		float FetchVALBlendWeight() const;

		const auto& GetREFSkeletalAnimationModelRecord() const { return m_skeletalAnimationModelRecord; }

		const auto& GetREFAnimation() const { return m_animation; }

		const auto& GetREFBlendTargetAnimation() const { return m_blendTargetAnimation; }

		float GetVALAnimationTimeSecond() const { return m_animationTimeSecond; }

		float GetVALBlendTargetAnimationTimeSecond() const { return m_blendTargetAnimationTimeSecond; }

		bool GetVALIsBlending() const { return m_isBlending; }

		static constexpr float k_initialBlendWeight = 0.0F;

	private:

		bool EvaluateCurrentPose();

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

		Animation m_animation            = {};
		Animation m_blendTargetAnimation = {};

		float m_animationTimeSecond            = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;
		float m_blendTargetAnimationTimeSecond = SkeletalAnimationModelRecord::k_initialAnimationTimeSecond;

		float m_blendElapsedSecond = k_initialBlendElapsedSecond;

		bool m_isBlending = false;
	};
}