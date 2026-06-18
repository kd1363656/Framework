#pragma once

namespace FWK::Graphics
{
	class TextureBatchUploadRecordBuilder final
	{
	public:
		
		 TextureBatchUploadRecordBuilder() = default;
		~TextureBatchUploadRecordBuilder() = default;

		bool CreateTextureBatchUploadRecord(const Device&                            a_device,
											const GPUMemoryAllocator&                a_gpuMemoryAllocator,
											const DirectX::ScratchImage&             a_scratchImage,
											const DirectX::TexMetadata&              a_texMetadata,
											const std::wstring&						 a_filePath,
											const TypeAlias::StorageID				 a_storageID,
												  TypeAlias::SRVDescriptorPool&		 a_srvDescriptorPool,
												  Struct::TextureBatchUploadRecord&  a_textureBatchUploadRecord) const;

	private:

		bool CreateTextureResource(const GPUMemoryAllocator& a_gpuMemoryAllocator, const DirectX::TexMetadata& a_texMetadata, Graphics::TextureRecord& a_textureRecord) const;

		bool CreateTextureUploadRecord(const Device& a_device, const DirectX::ScratchImage& a_scratchImage, Struct::TextureBatchUploadRecord& a_textureBatchUploadRecord) const;

		bool CreateTextureSRV(const Device&                       a_device,
							  const DirectX::TexMetadata&         a_texMetadata,
									TypeAlias::SRVDescriptorPool& a_srvDescriptorPool,
									Graphics::TextureRecord&	  a_textureRecord) const;

		static constexpr UINT64 k_uploadBufferBeginOffset         = 0ULL;
		static constexpr UINT64 k_initialRequiredUploadBufferSize = 0ULL;

		static constexpr UINT k_mostDetailedMIP = 0U;
		static constexpr UINT k_planeSlice		= 0U;
		static constexpr UINT k_firstArraySlice = 0U;

		static constexpr FLOAT k_resourceMINLODClamp = 0.0F;

		static constexpr std::size_t k_texture2DArrayMINArraySize = 2ULL;
	};
}