#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelBatchUploadRecordBuilder final
	{
	private:

		struct BoneBufferElement final
		{
			static constexpr std::uint32_t k_initialHierarchyDepth = 0U;

			TypeAlias::Math::Vector3 m_bindPoseLocalScale = TypeAlias::Math::Vector3::One;

			TypeAlias::Math::Quaternion m_bindPoseLocalRotation = TypeAlias::Math::Quaternion::Identity;

			TypeAlias::Math::Vector3 m_bindPoseLocalTranslation = TypeAlias::Math::Vector3::Zero;

			std::uint32_t m_parentBoneIndex = std::numeric_limits<std::uint32_t>::max();

			std::uint32_t m_hierarchyDepth = k_initialHierarchyDepth;
		};

		struct MotionSequenceBufferElement final
		{
			static constexpr float k_initialDurationSecond = 0.0F;
			static constexpr float k_initialFrameRate      = 0.0F;

			static constexpr std::uint32_t k_initialFirstBoneMotionTrackIndex = 0U;

			float m_durationSecond = k_initialDurationSecond;

			float m_frameRate = k_initialFrameRate;

			std::uint32_t m_firstBoneMotionTrackIndex = k_initialFirstBoneMotionTrackIndex;
		};

		struct BoneMotionTrackBufferElement final
		{
			static constexpr std::uint32_t k_initialKeyFrameCount = 0U;

			std::uint32_t m_firstKeyFrameIndex = std::numeric_limits<std::uint32_t>::max();

			std::uint32_t m_keyFrameCount = k_initialKeyFrameCount;
		};

		struct KeyFrameBufferElement final
		{
			static constexpr float k_initialTimeSecond = 0.0F;

			TypeAlias::Math::Vector3 m_scale = TypeAlias::Math::Vector3::One;

			TypeAlias::Math::Quaternion m_rotation = TypeAlias::Math::Quaternion::Identity;

			TypeAlias::Math::Vector3 m_position = TypeAlias::Math::Vector3::Zero;

			float m_timeSecond = k_initialTimeSecond;
		};

		struct StructuredBufferSourceData final
		{
			std::vector<BoneBufferElement> m_boneBufferElementList = {};

			std::vector<MotionSequenceBufferElement> m_motionSequenceBufferElementList = {};

			std::vector<BoneMotionTrackBufferElement> m_boneMotionTrackBufferElementList = {};

			std::vector<KeyFrameBufferElement> m_keyFrameBufferElementList = {};

			std::uint32_t m_maxBoneHierarchyDepth = SkeletalAnimationModelRecord::k_initialMAXBoneHierarchyDepth;
		};

	public:

		 SkeletalAnimationModelBatchUploadRecordBuilder() = default;
		~SkeletalAnimationModelBatchUploadRecordBuilder() = default;

		bool CreateSkeletalAnimationModelBatchUploadRecord(const Device&									               a_device,
												           const GPUMemoryAllocator&					                   a_gpuMemoryAllocator,
												           	     std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
												           	     TypeAlias::CBVSRVUAVDescriptorPool&		               a_cbvSRVUAVDescriptorPool,
												           	     SkeletalAnimationModelRecord&			                   a_skeletalAnimationModelRecord) const;

	private:

		// SkeletonとMotionをGPU向け一次元配列へ変換し、
		// 共有StructuredBufferを作成する
		bool CreateSkeletalAnimationModelSharedStructuredBuffer(const Device&                                                   a_device,
			                                                    const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
			                                                          std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
			                                                          TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool,
			                                                          SkeletalAnimationModelRecord&                             a_skeletalAnimationModelRecord) const;

		// CPU側の可変長Skeleton／Motion構造を、
		// GPUが直接Index参照できる密な一次元配列へ変換する。
		bool BuildSkeletalAnimationModelStructuredBufferSourceData(SkeletalAnimationModelRecord::ModelData& a_modelData, StructuredBufferSourceData& a_structuredBufferSourceData) const;

		void ReleaseCreatedSkeletalAnimationModelStructuredBuffer(std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const;

		static constexpr std::size_t k_initialKeyFrameIndex        = 0ULL;
		static constexpr std::size_t k_previousKeyFrameIndexOffset = 1ULL;
		static constexpr std::size_t k_initialTotalKeyFrameCount   = 0ULL;

		static constexpr std::uint32_t k_childHierarchyDepthOffset = 1U;

		ModelBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};
	};
}