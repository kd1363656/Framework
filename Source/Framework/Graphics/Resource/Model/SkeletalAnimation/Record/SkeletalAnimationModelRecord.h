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

			std::uint32_t m_bonePaletteIndex0 = k_invalidPaletteIndex;
			std::uint32_t m_bonePaletteIndex1 = k_invalidPaletteIndex;
			std::uint32_t m_bonePaletteIndex2 = k_invalidPaletteIndex;
			std::uint32_t m_bonePaletteIndex3 = k_invalidPaletteIndex;

			TypeAlias::Math::Vector4 m_boneWeight = {};
		};

		struct ModelBonePaletteElement final
		{
			TypeAlias::Math::Matrix m_inverseBindPoseMatrix = TypeAlias::Math::Matrix::Identity;

			std::uint32_t m_boneIndex = k_invalidBoneIndex;
		};

		struct ModelMesh final
		{
			 ModelMesh() = default;
			~ModelMesh() = default;

			ModelMesh(const ModelMesh&)           = delete;
			ModelMesh(      ModelMesh&&) noexcept = default;

			ModelMesh& operator=(const ModelMesh&)           = delete;
			ModelMesh& operator=(      ModelMesh&&) noexcept = default;

			std::vector<ModelVertex> m_modelVertexList = {};

			std::vector<ModelBonePaletteElement> m_bonePaletteList = {};

			std::vector<std::uint32_t> m_indexList = {};

			Struct::ModelMaterial m_modelMaterial = {};

			Struct::ModelMeshletData m_modelMeshletData = {};

			Struct::ModelMeshRuntimeData m_modelMeshRuntimeData = {};
		};

		struct ModelBone final
		{
			std::wstring m_boneName = {};

			TypeAlias::Math::Matrix m_bindPoseLocalMatrix = TypeAlias::Math::Matrix::Identity;
			
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
			 ModelData() = default;
			~ModelData() = default;

			ModelData(const ModelData&)           = delete;
			ModelData(      ModelData&&) noexcept = default;

			ModelData& operator=(const ModelData&)           = delete;
			ModelData& operator=(      ModelData&&) noexcept = default;

			std::vector<ModelMesh> m_modelMeshList = {};

			std::vector<ModelBone> m_boneList = {};

			std::vector<ModelMotionSequence> m_motionSequenceList = {};
		};

	public:

		 SkeletalAnimationModelRecord()          = default;
		~SkeletalAnimationModelRecord() override = default;

		bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;

		void SetModelData(ModelData&& a_set) { m_modelData = std::move(a_set); }

		const auto& GetREFModelData() const { return m_modelData; }

		auto& GetMutableREFModelData() { return m_modelData; }

		static constexpr float k_initialAnimationTimeSecond     = 0.0F;
		static constexpr float k_initialAnimationDurationSecond = 0.0F;
		static constexpr float k_defaultAnimationFrameRate      = 30.0F;

		static constexpr std::uint32_t k_invalidBoneIndex    = std::numeric_limits<std::uint32_t>::max();
		static constexpr std::uint32_t k_invalidPaletteIndex = std::numeric_limits<std::uint32_t>::max();

	private:

		ModelData m_modelData = {};
	};
}