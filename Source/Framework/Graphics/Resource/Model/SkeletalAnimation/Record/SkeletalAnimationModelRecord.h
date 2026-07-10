#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelRecord final : public AssetRecordBase
	{
	public:

		struct ModelBone final
		{
			std::wstring m_boneName = {};

			TypeAlias::Math::Matrix m_bindPoseLocalMatrix   = TypeAlias::Math::Matrix::Identity;
			TypeAlias::Math::Matrix m_inverseBindPoseMatrix = TypeAlias::Math::Matrix::Identity;

			std::uint32_t m_parentBoneIndex = k_invalidBoneIndex;
		};

		struct ModelKeyFrame final
		{
			TypeAlias::Math::Vector3    m_scale       = TypeAlias::Math::Vector3::One;
			TypeAlias::Math::Quaternion m_rotation    = TypeAlias::Math::Quaternion::Identity;
			TypeAlias::Math::Vector3    m_translation = TypeAlias::Math::Vector3::Zero;

			float m_timeSecond = k_initialAnimationTimeSecond;
		};

		struct ModelBoneMotionTrack final
		{
			std::vector<ModelKeyFrame> m_keyFrameList = {};

			std::uint32_t m_boneIndex = k_invalidBoneIndex;
		};

		struct ModelMotionSequence final
		{
			std::vector<ModelBoneMotionTrack> m_boneMotionTrackList = {};

			std::wstring m_motionName = {};

			float m_durationSecond = k_initialAnimationDurationSecond;

			float m_frameRate = k_defaultAnimationFrameRate;
		};

		struct ModelData final
		{
			std::vector<ModelBone> m_boneList = {};

			std::vector<ModelMotionSequence> m_motionSequenceList = {};
		};

	public:

		 SkeletalAnimationModelRecord()          = default;
		~SkeletalAnimationModelRecord() override = default;

		bool ReserveRelease(const UINT64&, ResourceReleaseContext&) override;

		void SetAnimationModelData(ModelData&& a_set) { m_animationModelData = std::move(a_set); }

		const auto& GetREFAnimationData() const { return m_animationModelData; }

		auto& GetMutableREFAnimationData() { return m_animationModelData; }

		static constexpr float k_initialAnimationTimeSecond     = 0.0F;
		static constexpr float k_initialAnimationDurationSecond = 0.0F;
		static constexpr float k_defaultAnimationFrameRate      = 30.0F;

		static constexpr std::uint32_t k_invalidBoneIndex = std::numeric_limits<std::uint32_t>::max();

	private:

		ModelData m_animationModelData = {};
	};
}