#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationPlayer final
	{
	public:

		struct Animation final
		{
			static constexpr float k_initialTimeSecond          = 0.0F;
			static constexpr float k_defaultPlaybackSpeed       = 1.0F;
			static constexpr float k_initialBlendDurationSecond = 0.0F;

			static constexpr std::uint32_t k_invalidMotionIndex = std::numeric_limits<std::uint32_t>::max();

			float m_startTimeSecond = k_initialTimeSecond;

			float m_playbackSpeed = k_defaultPlaybackSpeed;

			float m_blendDurationSecond = k_initialBlendDurationSecond;

			std::uint32_t m_motionIndex = k_invalidMotionIndex;

			bool m_isLoop = false;
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

		bool IsValid() const;

		bool ApplyAnimation(const Animation& a_animation);

		const DynamicRWStructuredBuffer* FetchPTRBoneMatrixBuffer() const;

		DynamicRWStructuredBuffer* FetchMutablePTRBoneMatrixBuffer();

		const auto& GetREFSkeletalAnimationModelRecord() const { return m_skeletalAnimationModelRecord; }

		const auto& GetREFAnimation() const { return m_animation; }

		const auto& GetREFBlendTargetAnimation() const { return m_blendTargetAnimation; }

	private:

		bool IsValidAnimation(const Animation& a_animation) const;

		float FetchMotionDurationSecond(const Animation& a_animation) const;

		float CalculateAdvancedTimeSecond(const Animation& a_animation, const float a_timeSecond, const float a_deltaTime) const;

		float CompleteAnimationBlend();

		void ResetPlaybackState();

		std::vector<DynamicRWStructuredBuffer> m_boneMatrixBufferList = {};

		std::weak_ptr<SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord = {};
		
		Animation m_animation            = {};
		Animation m_blendTargetAnimation = {};
	};
}