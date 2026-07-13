#include "SkeletalAnimationModelSystem.h"

void FWK::Graphics::SkeletalAnimationModelSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::SkeletalAnimationModelSystem::Create()
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_modelStorage.Create(), "AssetStorageの作成に失敗したため、SkeletalAnimationModelSystemの作成処理に失敗しました。", false);

	return true;
}

FWK::Struct::SkeletalAnimationModelLoadResult FWK::Graphics::SkeletalAnimationModelSystem::LoadSkeletalAnimationModelForBatchUpload(const Device&                             a_device, 
	                                                                                                                                const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
	                                                                                                                                const std::filesystem::path&              a_filePath, 
	                                                                                                                                      TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	Struct::SkeletalAnimationModelLoadResult l_skeletalAnimationModelLoadResult = {};

	// .fbxでない、もしくは存在しないなら読み込まない
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "SkeletalAnimationMOdelのFBXファイルが存在しません。", l_skeletalAnimationModelLoadResult);
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

bool FWK::Graphics::SkeletalAnimationModelSystem::BuildSkeletalAnimationModelAssetData(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
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
