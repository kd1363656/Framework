#include "StaticModelSystem.h"

void FWK::Graphics::StaticModelSystem::LoadStaticModelForBatchUpload(const Device&			             a_device, 
															         const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
																	 const std::filesystem::path&        a_filePath, 
																		   TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	auto l_staticModelRecord = std::make_shared<Graphics::StaticModelRecord>();

	m_loader.LoadStaticModelFile(a_filePath, *l_staticModelRecord);
}