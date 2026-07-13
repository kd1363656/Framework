#pragma once

namespace FWK::Graphics
{
	class StaticModelBatchUploadRecordBuilder final
	{
	public:

		 StaticModelBatchUploadRecordBuilder() = default;
		~StaticModelBatchUploadRecordBuilder() = default;

		bool CreateStaticModelBatchUploadRecord(const Device&									                a_device,
												const GPUMemoryAllocator&					                    a_gpuMemoryAllocator,
													  std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
													  TypeAlias::CBVSRVUAVDescriptorPool&		                a_cbvSRVUAVDescriptorPool,
													  Graphics::StaticModelRecord&			                    a_staticModelRecord) const;

	private:

		void ReleaseCreatedStaticModelStructuredBuffer(std::vector<StaticModelRecord::ModelMesh>& a_modelMeshList) const;

		ModelBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};
	};
}