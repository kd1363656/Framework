#pragma once

namespace FWK::Graphics
{
	class TextureBatchUploadRecordBuilder final
	{
	public:

		struct TextureUploadRecord final
		{
			 TextureUploadRecord() = default;
			~TextureUploadRecord() = default;

			TextureUploadRecord(const TextureUploadRecord&)           = delete;
			TextureUploadRecord(	  TextureUploadRecord&&) noexcept = default;

			TextureUploadRecord& operator=(const TextureUploadRecord&)			 = delete;
			TextureUploadRecord& operator=(	     TextureUploadRecord&&) noexcept = default;

			// 各サブリソースの配置情報をまとめたリスト
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> m_layoutList = {};

			// DEFAULTヒープ上のテクスチャリソースへコピーするための中間バッファ
			Graphics::UploadBuffer m_uploadBuffer = {};
		};

		struct TextureBatchUploadRecord final
		{
			 TextureBatchUploadRecord() = default;
			~TextureBatchUploadRecord() = default;

			TextureBatchUploadRecord(const TextureBatchUploadRecord&)           = delete;
			TextureBatchUploadRecord(	   TextureBatchUploadRecord&&) noexcept = default;

			TextureBatchUploadRecord& operator=(const TextureBatchUploadRecord&)		   = delete;
			TextureBatchUploadRecord& operator=(	  TextureBatchUploadRecord&&) noexcept = default;

			std::shared_ptr<Graphics::TextureRecord> m_textureRecord = nullptr;

			TextureUploadRecord m_textureUploadRecord = {};
		};

	public:
		
		 TextureBatchUploadRecordBuilder() = default;
		~TextureBatchUploadRecordBuilder() = default;

		bool CreateTextureBatchUploadRecord(const Device&                             a_device,
											const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
										    const std::wstring&						  a_filePath,
											const DirectX::ScratchImage&              a_scratchImage,
											const DirectX::TexMetadata&               a_texMetadata,
											const TypeAlias::StorageID				  a_storageID,
												  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
												  TextureBatchUploadRecord&           a_textureBatchUploadRecord) const;

	private:

		bool CreateTextureResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const DirectX::TexMetadata& a_texMetadata, Graphics::TextureRecord& a_textureRecord) const;

		bool CreateTextureUploadRecord(const Device& a_device, const DirectX::ScratchImage& a_scratchImage, TextureBatchUploadRecord& a_textureBatchUploadRecord) const;

		bool CreateTextureSRV(const Device&                             a_device,
							  const DirectX::TexMetadata&               a_texMetadata,
									TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
									Graphics::TextureRecord&	        a_textureRecord) const;

		static constexpr FLOAT k_resourceMINLODClamp = 0.0F;

		static constexpr UINT64 k_uploadBufferBeginOffset         = 0ULL;
		static constexpr UINT64 k_initialRequiredUploadBufferSize = 0ULL;

		static constexpr UINT k_mostDetailedMIP = 0U;
		static constexpr UINT k_planeSlice		= 0U;
		static constexpr UINT k_firstArraySlice = 0U;

		static constexpr std::size_t k_texture2DArrayMINArraySize = 2ULL;
	};
}