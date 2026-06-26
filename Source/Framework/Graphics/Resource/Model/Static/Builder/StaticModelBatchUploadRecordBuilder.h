#pragma once

namespace FWK::Graphics
{
	class StaticModelBatchUploadRecordBuilder final
	{
	public:

		 StaticModelBatchUploadRecordBuilder() = default;
		~StaticModelBatchUploadRecordBuilder() = default;

		bool CreateStaticModelBatchUploadRecord(const Device&									a_device,
												const GPUMemoryAllocator&					    a_gpuMemoryAllocator,
													  std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
													  TypeAlias::SRVDescriptorPool&		        a_srvDescriptorPool,
													  Graphics::StaticModelRecord&			    a_staticModelRecord) const;

	private:

		bool CreateModelBatchUploadRecord(const Device&									  a_device,
										  const GPUMemoryAllocator&						  a_gpuMemoryAllocator,
											    std::vector<Struct::BufferUploadCommand>& a_bufferUploadCommandList,
												TypeAlias::SRVDescriptorPool&		      a_srvDescriptorPool,
												Struct::StaticModelMesh&				  a_staticModelMesh) const;

		void ReleaseCreatedStaticModelStructuredBufferSRV(std::vector<Struct::StaticModelMesh>& a_staticModelMeshList, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const;
	};
}