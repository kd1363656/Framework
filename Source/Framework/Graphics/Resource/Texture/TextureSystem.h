#pragma once

namespace FWK::Graphics
{
	class TextureSystem
	{
	public:

		 TextureSystem() = default;
		~TextureSystem() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create	    ();

		// 仮読み込み
		bool LoadTextureFile(const std::filesystem::path& a_filePath, const Enum::TextureLoadType a_loadType = Enum::TextureLoadType::Auto);

		Struct::TextureLoadResult LoadTextureForBatchUpload(const Device&			            a_device, 
													        const GPUMemoryAllocator&           a_gpuMemoryAllocator,
													        const std::filesystem::path&		a_filePath,
																  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool,
															const Enum::TextureLoadType			a_loadType = Enum::TextureLoadType::Auto);

		nlohmann::json Serialize() const;

		bool AddTextureReferenceCount     (const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord);
		bool SubtractTextureReferenceCount(const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord, const DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

		const auto& GetREFTextureStorage() const { return m_textureStorage; }

		auto& GetMutableREFTextureStorage() { return m_textureStorage; }

	private:

		TypeAlias::PendingTextureBatchUploadRecordMap m_pendingTextureBatchUploadRecordMap = {};

		AssetStorage<Graphics::TextureRecord> m_textureStorage = {};

		TextureLoader					m_loader				   = {};
		TextureBatchUploadRecordBuilder m_batchUploadRecordBuilder = {};

		Converter::TextureSystemJsonConverter m_jsonConverter = {};
	};
}