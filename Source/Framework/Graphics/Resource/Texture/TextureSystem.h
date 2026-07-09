#pragma once

namespace FWK::Graphics
{
	class TextureSystem
	{
	private:

		using PendingTextureBatchUploadRecordMap = std::unordered_map<std::wstring, Struct::TextureBatchUploadRecord, Struct::WStringHash, std::equal_to<>>;
		
	public:

		 TextureSystem() = default;
		~TextureSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create	    (const Device&		   a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		Struct::TextureLoadResult LoadTextureForBatchUpload(const Device&			                  a_device, 
													        const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
													        const std::filesystem::path&		      a_filePath,
															const Enum::TextureLoadColorSpace         a_textureLoadColorSpace,
															const Enum::DefaultTextureType            a_defaultTextureType,
																  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		nlohmann::json Serialize() const;

		// ※ 注意
		// UploadSystem側でテクスチャのコピーが終わっていること前提
		void RegisterPendingTextures();

		bool AddTextureReferenceCount     (const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord);
		bool SubtractTextureReferenceCount(const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

		void ApplyDefaultTexture(const Enum::DefaultTextureType a_defaultTextureType, const std::shared_ptr<DefaultTexture>& a_defaultTexture);

		std::weak_ptr<Graphics::TextureRecord> FetchVALDefaultTextureRecord(const Enum::DefaultTextureType a_defaultTextureType) const;

		static constexpr std::size_t k_defaultTextureTypeCount = static_cast<std::size_t>(Enum::DefaultTextureType::Count);

		const auto& GetREFPendingTextureBatchUploadRecordMap() const { return m_pendingTextureBatchUploadRecordMap; }

		const auto& GetREFDefaultTextureList() const { return m_defaultTextureList; }

		const auto& GetREFTextureStorage() const { return m_textureStorage; }

		auto& GetMutableREFTextureStorage() { return m_textureStorage; }

	private:

		bool CreateDefaultTexturesForBatchUpload(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		void CreateAndRegisterPendingTextureForBachUpload(const Device&				                a_device,
														  const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
														  const std::filesystem::path&              a_filePath,
														  const DirectX::ScratchImage&              a_scratchImage,
														  const DirectX::TexMetadata&               a_texMetadata,
														  	    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
														  	    Struct::TextureLoadResult&          a_textureLoadResult);

		bool TryResolveCachedTextureResult(const std::filesystem::path& a_filePath, Struct::TextureLoadResult& a_textureLoadResult);

		void ApplyDefaultTextureToLoadResult(const Enum::DefaultTextureType a_defaultTextureType, Struct::TextureLoadResult& a_textureLoadResult) const;

		PendingTextureBatchUploadRecordMap m_pendingTextureBatchUploadRecordMap = {};

		std::array<std::shared_ptr<DefaultTexture>, k_defaultTextureTypeCount> m_defaultTextureList = {};

		AssetStorage<Graphics::TextureRecord> m_textureStorage = {};

		TextureLoader					m_loader				   = {};
		TextureBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};

		Converter::TextureSystemJsonConverter m_jsonConverter		   = {};
		Converter::TextureBinaryConverter     m_textureBinaryConverter = {};
	};
}