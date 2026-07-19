#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationPoseEvaluator final
	{
	private:

		struct LocalTransform final
		{
			TypeAlias::Math::Vector3    m_scale       = TypeAlias::Math::Vector3::One;
			TypeAlias::Math::Quaternion m_rotation    = TypeAlias::Math::Quaternion::Identity;
			TypeAlias::Math::Vector3    m_translation = TypeAlias::Math::Vector3::Zero;
		};

	public:

		 SkeletalAnimationPoseEvaluator() = default;
		~SkeletalAnimationPoseEvaluator() = default;

		SkeletalAnimationPoseEvaluator(const SkeletalAnimationPoseEvaluator&)           = delete;
		SkeletalAnimationPoseEvaluator(      SkeletalAnimationPoseEvaluator&&) noexcept = default;

		SkeletalAnimationPoseEvaluator& operator=(const SkeletalAnimationPoseEvaluator&)           = delete;
		SkeletalAnimationPoseEvaluator& operator=(      SkeletalAnimationPoseEvaluator&&) noexcept = default;

		bool Create(SkeletalAnimationModelRecord::ModelData& a_modelData);

		bool EvaluatePose(const SkeletalAnimationModelRecord::ModelData& a_modelData,
			              const float                                    a_animationTimeSecond,
			              const float                                    a_blendTargetAnimationTimeSecond,
			              const float                                    a_blendWeight,
			              const std::uint32_t                            a_motionIndex,
			              const std::uint32_t                            a_blendTargetMotionIndex,
			              const bool                                     a_isBlending,
			                    std::vector<TypeAlias::Math::Matrix>&    a_globalBoneMatrixList) const;

		const auto& GetREFBindPoseGlobalBoneMatrixList() const { return m_bindPoseGlobalBoneMatrixList; }

		static constexpr std::uint32_t k_invalidMotionIndex = std::numeric_limits<std::uint32_t>::max();

	private:

		bool                    CreateAnimationLookupData(      SkeletalAnimationModelRecord::ModelData& a_modelData, std::vector<LocalTransform>& a_bindPoseLocalTransformList, std::vector<std::vector<std::uint32_t>>& a_boneMotionTrackIndexList) const;
		TypeAlias::Math::Matrix CreateLocalMatrix        (const LocalTransform&                          a_localTransform)                                                                                                                            const;

		LocalTransform SampleLocalTransform(const SkeletalAnimationModelRecord::ModelMotionSequence& a_motionSequence,
			                                const float                                              a_timeSecond,
	                                        const std::uint32_t                                      a_motionIndex,
	                                        const std::uint32_t                                      a_boneIndex) const;

		LocalTransform InterpolateLocalTransform(const LocalTransform& a_startLocalTransform, const LocalTransform& a_endLocalTransform, const float a_interpolationWeight) const;

		static constexpr float k_minKeyFrameTimeRange = 0.000001F;

		static constexpr float k_minInterpolationWeight = 0.0F;
		static constexpr float k_maxInterpolationWeight = 1.0F;

		static constexpr std::size_t k_firstBoneIndex            = 0ULL;
		static constexpr std::size_t k_firstMotionIndex          = 0ULL;
		static constexpr std::size_t k_firstBoneMotionTrackIndex = 0ULL;
		static constexpr std::size_t k_firstKeyFrameIndex        = 0ULL;
		static constexpr std::size_t k_singleKeyFrameCount       = 1ULL;
		static constexpr std::size_t k_nextKeyFrameOffset        = 1ULL;

		static constexpr std::uint32_t k_invalidBoneMotionTrackIndex = std::numeric_limits<std::uint32_t>::max();

		std::vector<std::vector<std::uint32_t>> m_boneMotionTrackIndexList = {};

		std::vector<TypeAlias::Math::Matrix> m_bindPoseGlobalBoneMatrixList = {};
		
		std::vector<LocalTransform> m_bindPoseLocalTransformList = {};
	};
}