#pragma once

namespace FWK::Graphics
{
	class TextureSystem
	{
	private:

		using PendingTextureBatchUploadRecordMap = std::unordered_map<std::wstring, Struct::TextureBatchUploadRecord, Struct::WStringHash, std::equal_to<>>;
		using DefaultTextureMap					 = std::unordered_map<Enum::DefaultTextureType, DefaultTexture>;

	public:

		 TextureSystem() = default;
		~TextureSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create	    ();

		Struct::TextureLoadResult LoadTextureForBatchUpload(const Device&			            a_device, 
													        const GPUMemoryAllocator&           a_gpuMemoryAllocator,
													        const std::filesystem::path&		a_filePath,
																  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool,
															const Enum::TextureLoadType			a_loadType = Enum::TextureLoadType::Auto);

		nlohmann::json Serialize() const;

		// ※ 注意
		// UploadSystem側でテクスチャのコピーが終わっていること前提
		void RegisterPendingTextures();

		bool AddTextureReferenceCount     (const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord);
		bool SubtractTextureReferenceCount(const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord, const DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

		void SetIsUploadToDefaultHeapCopyCompleted(const bool a_set) { m_isUploadToDefaultHeapCopyCompleted = a_set; }

		const auto& GetREFPendingTextureBatchUploadRecordMap() const { return m_pendingTextureBatchUploadRecordMap; }

		const auto& GetREFTextureStorage() const { return m_textureStorage; }

		auto& GetMutableREFTextureStorage() { return m_textureStorage; }

	private:

		PendingTextureBatchUploadRecordMap m_pendingTextureBatchUploadRecordMap = {};

		AssetStorage<Graphics::TextureRecord> m_textureStorage = {};

		TextureLoader					m_loader				   = {};
		TextureBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};

		Converter::TextureSystemJsonConverter m_jsonConverter = {};

		bool m_isUploadToDefaultHeapCopyCompleted = false;
	};
}