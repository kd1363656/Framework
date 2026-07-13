#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelSystem final
	{
	private:

		using PendingSkeletalAnimationModelBatchUploadRecordMap = std::unordered_map<std::wstring, Struct::SkeletalAnimationModelBatchUploadRecord, Struct::WStringHash, std::not_equal_to<>>;

	public:

		 SkeletalAnimationModelSystem() = default;
		~SkeletalAnimationModelSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create     ();

		Struct::SkeletalAnimationModelLoadResult LoadSkeletalAnimationModelForBatchUpload(const Device&                             a_device,
																						  const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
			                                                                              const std::filesystem::path&              a_filePath,
																							    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		nlohmann::json Serialize() const;

		void RegisterPendingSkeletalAnimationModels();

		bool AddSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimationModelRecord);

		bool SubtractSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimationModelRecord, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

		const auto& GetREFPendingModelBatchUploadRecordMap() const { return m_pendingModelBatchUploadRecordMap; }

		const auto& GetREFModelStorage() const { return m_skeletalAnimationModelStorage; }

		auto& GetMutableREFModelStorage() { return m_skeletalAnimationModelStorage; }

	private:

		bool BuildSkeletalAnimationModelRuntimeData(const std::shared_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimmationModelRecord,
			                                        const Device&                                        a_device,
			                                        const GPUMemoryAllocator&                            a_gpuMemoryAllocator,
			                                        const std::filesystem::path&                         a_filePath,
												    const TypeAlias::StorageID                           a_storageID,
			                                              TypeAlias::CBVSRVUAVDescriptorPool&            a_cbvSRVUAVDescriptorPool);
		
		bool CreateSkeletalAnimationModelBatchUploadRecord(const std::shared_ptr<SkeletalAnimationModelRecord>&   a_skeletalAnimmationModelRecord,
			                                               const Device&                                          a_device,
			                                               const GPUMemoryAllocator&                              a_gpuMemoryAllocator,
			                                                     TypeAlias::CBVSRVUAVDescriptorPool&              a_cbvSRVUAVDescriptorPool,
															     Struct::SkeletalAnimationModelBatchUploadRecord& a_skeletalAnimationModelBatchUploadRecord) const;
 
		bool TryResolveCachedSkeletalAnimationModelResult(const std::filesystem::path& a_filePath, Struct::SkeletalAnimationModelLoadResult& a_skeletalAnimationModelLoadResult);

		PendingSkeletalAnimationModelBatchUploadRecordMap m_pendingModelBatchUploadRecordMap = {};

		AssetStorage<SkeletalAnimationModelRecord> m_skeletalAnimationModelStorage = {};

		SkeletalAnimationModelBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};

		SkeletalAnimationModelFBXLoader                                  m_loader                        = {};
		ModelMaterialRuntimeTextureBuilder<SkeletalAnimationModelRecord> m_materialRuntimeTextureBuilder = {};

		ModelMeshOptimizer<SkeletalAnimationModelRecord>  m_meshOptimizer  = {};
		ModelMeshletBuilder<SkeletalAnimationModelRecord> m_meshletBuilder = {};

		Converter::SkeletalAnimationModelSystemJsonConverter m_jsonConverter   = {};
		Converter::SkeletalAnimationModelBinaryConverter     m_binaryConverter = {};
	};
}