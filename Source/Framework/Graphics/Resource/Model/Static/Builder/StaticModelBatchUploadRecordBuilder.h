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

		bool CreateModelBatchUploadRecord(const Device&									                  a_device,
										  const GPUMemoryAllocator&						                  a_gpuMemoryAllocator,
											    std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
												TypeAlias::CBVSRVUAVDescriptorPool&		                  a_cbvSRVUAVDescriptorPool,
												StaticModelRecord::StaticModelMesh&				          a_staticModelMesh) const;

		void ReleaseCreatedStaticModelStructuredBufferSRV(std::vector<StaticModelRecord::StaticModelMesh>& a_staticModelMeshList, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool) const;
	};
}