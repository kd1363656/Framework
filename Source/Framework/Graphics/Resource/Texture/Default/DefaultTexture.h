#pragma once

namespace FWK::Graphics
{
	class DefaultTexture
	{
	public:

				 DefaultTexture() = default;
		virtual ~DefaultTexture() = default;

		DefaultTexture(const DefaultTexture&)			= delete;
		DefaultTexture(	     DefaultTexture&&) noexcept = default;

		DefaultTexture& operator=(const DefaultTexture&)		   = delete;
		DefaultTexture& operator=(	    DefaultTexture&&) noexcept = default;

		bool CreateTextureBatchUploadRecord(const Device&					        a_device,
											const GPUMemoryAllocator&		        a_gpuMemoryAllocator,
											const TextureBatchUploadRecordBuilder&  a_textureBatchUploadRecordBuilder,
												  TypeAlias::SRVDescriptorPool&     a_srvDescriptorPool,
												  Struct::TextureBatchUploadRecord& a_textureBatchUploadRecord) const;
	
	private:

		bool CreateScratchImage(DirectX::ScratchImage& a_scratchImage) const;

		static constexpr std::size_t k_defaultTextureWidth     = 1ULL;
		static constexpr std::size_t k_defaultTextureHeight    = 1ULL;
		static constexpr std::size_t k_defaultTextureArraySize = 1ULL;
		static constexpr std::size_t k_defaultTextureMipLevels = 1ULL;

		static constexpr std::size_t k_defaultTextureMipIndex   = 0ULL;
		static constexpr std::size_t k_defaultTextureItemIndex  = 0ULL;
		static constexpr std::size_t k_defaultTextureSliceIndex = 0ULL;

		static constexpr std::size_t k_defaultTexturePixelChannelCount = 4ULL;
	};
}