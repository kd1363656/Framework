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

		Struct::StaticModelLoadResult LoadStaticModelForBatchUpload(const Device&			            a_device,
																    const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
																    const std::filesystem::path&        a_filePath, 
																    	  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool);

		nlohmann::json Serialize() const;

		bool AddStaticModelReferenceCount     (const std::weak_ptr<Graphics::StaticModelRecord>& a_staticModelRecord);
		bool SubtractStaticModelReferenceCount(const std::weak_ptr<Graphics::StaticModelRecord>& a_staticModelRecord, const DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

		const auto& GetREFStaticModelStorage() const { return m_staticModelStorage; }

		auto& GetMutableREFStaticModelStorage() { return m_staticModelStorage; }

	private:

		void CreateMaterialTexture(const std::filesystem::path& a_filePath, StaticModelRecord& a_staticModelRecord) const;

		std::shared_ptr<Texture> CreateMaterialTexture(const std::filesystem::path&      a_modelFilePath, 
													   const std::wstring&			     a_textureFilePath,
													   const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
													   const Enum::DefaultTextureType    a_defaultTextureType) const;

		bool TryResolveCachedStaticModelResult(const std::filesystem::path& a_filePath, Struct::StaticModelLoadResult& a_staticModelLoadResult);

		PendingStaticModelBatchUploadRecordMap m_pendingStaticModelBatchUploadRecordMap = {};
		
		AssetStorage<Graphics::StaticModelRecord> m_staticModelStorage = {};

		StaticModelFBXLoader m_loader = {};

		Converter::StaticModelSystemJsonConverter m_jsonConverter			   = {};
		Converter::StaticModelBinaryConverter     m_staticModelBinaryConverter = {};
	};
}