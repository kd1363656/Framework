#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelSystem final
	{
	public:

		 SkeletalAnimationModelSystem() = default;
		~SkeletalAnimationModelSystem() = default;

		Struct::SkeletalAnimationModelLoadResult LoadSkeletalAnimationModelForBatchUpload(const Device&                             a_device,
																						  const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
			                                                                              const std::filesystem::path&              a_filePath,
																							    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		void RegisterPendingSkeletalAnimationModels();

		bool AddSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimationModelRecord);

		bool SubtractSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimationModelRecord, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

	private:

		bool BuildSkeletalAnimationModelAssetData(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

		SkeletalAnimationModelFBXLoader                                  m_loader                        = {};
		ModelMaterialRuntimeTextureBuilder<SkeletalAnimationModelRecord> m_materialRuntimeTextureBuilder = {};

		ModelMeshOptimizer<SkeletalAnimationModelRecord>  m_meshOptimizer  = {};
		ModelMeshletBuilder<SkeletalAnimationModelRecord> m_meshletBuilder = {};

		Converter::SkeletalAnimationModelBinaryConverter m_binaryConverter = {};
	};
}