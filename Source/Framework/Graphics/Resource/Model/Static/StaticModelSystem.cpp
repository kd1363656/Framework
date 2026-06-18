#include "StaticModelSystem.h"

void FWK::Graphics::StaticModelSystem::LoadStaticModelForBatchUpload(const Device&			             a_device, 
															         const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
																	 const std::filesystem::path&        a_filePath, 
																		   TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	// 読み込めるファイルかどうかを確認
	if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension)) { return; }

	auto l_staticModelRecord = std::make_shared<Graphics::StaticModelRecord>();

}