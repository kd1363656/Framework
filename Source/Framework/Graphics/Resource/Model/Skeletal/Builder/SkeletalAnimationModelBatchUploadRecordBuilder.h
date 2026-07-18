#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelBatchUploadRecordBuilder final
	{
	public:

		 SkeletalAnimationModelBatchUploadRecordBuilder() = default;
		~SkeletalAnimationModelBatchUploadRecordBuilder() = default;

		bool CreateSkeletalAnimationModelBatchUploadRecord(const Device&									               a_device,
												           const GPUMemoryAllocator&					                   a_gpuMemoryAllocator,
												           	     std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
												           	     TypeAlias::CBVSRVUAVDescriptorPool&		               a_cbvSRVUAVDescriptorPool,
												           	     SkeletalAnimationModelRecord&			                   a_skeletalAnimationModelRecord) const;

	private:

		void ReleaseCreatedSkeletalAnimationModelStructuredBuffer(std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const;

		static constexpr std::size_t k_initialKeyFrameIndex        = 0ULL;
		static constexpr std::size_t k_previousKeyFrameIndexOffset = 1ULL;
		static constexpr std::size_t k_initialTotalKeyFrameCount   = 0ULL;

		static constexpr std::uint32_t k_childHierarchyDepthOffset = 1U;

		ModelBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};
	};
}