#pragma once

namespace FWK::Graphics
{
	class StaticModelSystem final
	{
	private:

		using PendingStaticModelBatchUploadRecordMap = std::unordered_map<std::wstring, Struct::StaticModelBatchUploadRecord, Struct::WStringHash, std::equal_to<>>;

	public:

		 StaticModelSystem() = default;
		~StaticModelSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create     ();

		Struct::StaticModelLoadResult LoadStaticModelForBatchUpload(const Device&			                  a_device,
																    const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
																    const std::filesystem::path&              a_filePath, 
																    	  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		nlohmann::json Serialize() const;

		// ※ 注意
		// UploadSystem側でスタティックモデルのバッファーのコピーが終わっていること前提
		void RegisterPendingStaticModels();

		bool AddStaticModelReferenceCount     (const std::weak_ptr<Graphics::StaticModelRecord>& a_staticModelRecord);
		bool SubtractStaticModelReferenceCount(const std::weak_ptr<Graphics::StaticModelRecord>& a_staticModelRecord, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

		const auto& GetREFPendingStaticModelBatchUploadRecordMap() const { return m_pendingStaticModelBatchUploadRecordMap; }

		const auto& GetREFStaticModelStorage() const { return m_staticModelStorage; }

		auto& GetMutableREFStaticModelStorage() { return m_staticModelStorage; }

	private:

		bool BuildStaticModelAssetData(const std::filesystem::path& a_filePath, StaticModelRecord& a_staticModelRecord);

		void BuildMaterialRuntimeTextures(const std::filesystem::path& a_filePath, StaticModelRecord& a_staticModelRecord) const;

		void BuildStaticModelRuntimeData(const std::shared_ptr<StaticModelRecord>& a_staticModelRecord,
										 const Device&			                   a_device,
										 const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
										 const std::filesystem::path&			   a_filePath,
										 const TypeAlias::StorageID				   a_storageID,
											   TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
											   Struct::StaticModelLoadResult&      a_staticModelLoadResult);

		std::shared_ptr<Texture> CreateSingleMaterialTexture(const std::filesystem::path&      a_modelFilePath, 
													         const std::wstring&			   a_textureFilePath,
													         const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
													         const Enum::DefaultTextureType    a_defaultTextureType) const;

		bool CreateStaticBatchUploadRecord(const std::shared_ptr<StaticModelRecord>    a_staticModelRecord,
										   const Device&							   a_device, 
										   const GPUMemoryAllocator&				   a_gpuMemoryAllocator, 
										   	     TypeAlias::CBVSRVUAVDescriptorPool&   a_cbvSRVUAVDescriptorPool,
												 Struct::StaticModelBatchUploadRecord& a_staticModelBatchUploadRecord) const;


		bool TryResolveCachedStaticModelResult(const std::filesystem::path& a_filePath, Struct::StaticModelLoadResult& a_staticModelLoadResult);

		PendingStaticModelBatchUploadRecordMap m_pendingStaticModelBatchUploadRecordMap = {};
		
		AssetStorage<Graphics::StaticModelRecord> m_staticModelStorage = {};

		StaticModelFBXLoader				   m_loader				     = {};
		ModelMeshOptimizer<StaticModelRecord>  m_meshOptimizer			 = {};
		ModelMeshletBuilder<StaticModelRecord> m_meshletBuilder		     = {};
		StaticModelBatchUploadRecordBuilder    m_batchUploadRecordBuilder = {};

		Converter::StaticModelSystemJsonConverter m_jsonConverter			   = {};
		Converter::StaticModelBinaryConverter     m_staticModelBinaryConverter = {};
	};
}