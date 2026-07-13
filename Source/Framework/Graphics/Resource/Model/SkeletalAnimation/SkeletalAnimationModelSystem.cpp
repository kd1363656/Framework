#include "SkeletalAnimationModelSystem.h"

void FWK::Graphics::SkeletalAnimationModelSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (!a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::SkeletalAnimationModelSystem::Create()
{
	return false;
}

FWK::Struct::SkeletalAnimationModelLoadResult FWK::Graphics::SkeletalAnimationModelSystem::LoadSkeletalAnimationModelForBatchUpload(const Device&                             a_device, 
	                                                                                                                                const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
	                                                                                                                                const std::filesystem::path&              a_filePath, 
	                                                                                                                                      TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	return Struct::SkeletalAnimationModelLoadResult();
}

nlohmann::json FWK::Graphics::SkeletalAnimationModelSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::SkeletalAnimationModelSystem::RegisterPendingSkeletalAnimationModels()
{

}

bool FWK::Graphics::SkeletalAnimationModelSystem::AddSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>&a_skeletalAnimationModelRecord)
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::SubtractSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimationModelRecord, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::BuildSkeletalAnimationModelRuntimeData(const std::shared_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimmationModelRecord, 
	                                                                                     const Device&                                        a_device, 
	                                                                                     const GPUMemoryAllocator&                            a_gpuMemoryAllocator, 
	                                                                                     const std::filesystem::path&                         a_filePath, 
	                                                                                     const TypeAlias::StorageID                           a_storageID,
	                                                                                           TypeAlias::CBVSRVUAVDescriptorPool&            a_cbvSRVUAVDescriptorPool)
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::CreateSkeletalAnimationModelBatchUploadRecord(const std::shared_ptr<SkeletalAnimationModelRecord>&   a_skeletalAnimmationModelRecord, 
	                                                                                            const Device&                                          a_device,
	                                                                                            const GPUMemoryAllocator&                              a_gpuMemoryAllocator, 
	                                                                                                  TypeAlias::CBVSRVUAVDescriptorPool&              a_cbvSRVUAVDescriptorPool, 
	                                                                                                  Struct::SkeletalAnimationModelBatchUploadRecord& a_skeletalAnimationModelBatchUploadRecord) const
{
	return false;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::TryResolveCachedSkeletalAnimationModelResult(const std::filesystem::path& a_filePath, Struct::SkeletalAnimationModelLoadResult& a_skeletalAnimationModelLoadResult)
{
	return false;
}
