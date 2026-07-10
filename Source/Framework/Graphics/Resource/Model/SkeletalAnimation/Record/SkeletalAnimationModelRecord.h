#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelRecord final : public AssetRecordBase
	{
	public:

		struct ModelVertex final
		{
			TypeAlias::Math::Vector3 m_position = {};
			TypeAlias::Math::Vector3 m_normal   = {};
			TypeAlias::Math::Vector4 m_tangent  = {};
			TypeAlias::Math::Vector2 m_uv       = {};

			std::uint32_t m_boneIndex0 = k_invalidBoneIndex;
			std::uint32_t m_boneIndex1 = k_invalidBoneIndex;
			std::uint32_t m_boneIndex2 = k_invalidBoneIndex;
			std::uint32_t m_boneIndex3 = k_invalidBoneIndex;

			TypeAlias::Math::Vector4 m_boneWeight = {};
		};

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

		void SetModelData(ModelData&& a_set) { m_modelData = std::move(a_set); }

		const auto& GetREFModelData() const { return m_modelData; }

		auto& GetMutableREFModelData() { return m_modelData; }

		static constexpr float k_initialAnimationTimeSecond     = 0.0F;
		static constexpr float k_initialAnimationDurationSecond = 0.0F;
		static constexpr float k_defaultAnimationFrameRate      = 30.0F;

		static constexpr std::uint32_t k_invalidBoneIndex = std::numeric_limits<std::uint32_t>::max();

	private:

		ModelData m_modelData = {};
	};
}