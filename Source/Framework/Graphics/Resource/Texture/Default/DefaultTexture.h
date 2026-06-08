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

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize() const;

		bool CreateTextureBatchUploadRecord(const Device&					        a_device,
											const GPUMemoryAllocator&		        a_gpuMemoryAllocator,
											const TextureBatchUploadRecordBuilder&  a_textureBatchUploadRecordBuilder,
												  TypeAlias::SRVDescriptorPool&     a_srvDescriptorPool,
												  Struct::TextureBatchUploadRecord& a_textureBatchUploadRecord) const;
	
		void SetColor(const TypeAlias::Math::Color& a_set) { m_color = a_set; }

		void SetFormat(const DXGI_FORMAT a_set) { m_format = a_set; }

		void SetTextureName(const std::wstring& a_set) { m_textureName = a_set; }

		const auto& GetREFColor() const { return m_color; }

		auto GetVALFormat() const { return m_format; }

		const auto& GetREFTextureName() const { return m_textureName; }

	private:

		bool CreateScratchImage(DirectX::ScratchImage& a_scratchImage) const;

		static constexpr std::size_t k_defaultTextureWidth     = 1ULL;
		static constexpr std::size_t k_defaultTextureHeight    = 1ULL;
		static constexpr std::size_t k_defaultTextureArraySize = 1ULL;
		static constexpr std::size_t k_defaultTextureMipLevels = 1ULL;

		static constexpr std::size_t k_defaultTextureMipIndex   = 0ULL;
		static constexpr std::size_t k_defaultTextureItemIndex  = 0ULL;
		static constexpr std::size_t k_defaultTextureSliceIndex = 0ULL;

		static constexpr float k_maxColorChannel = 1.0F;

		TypeAlias::Math::Color m_color =
		{
			k_maxColorChannel,
			k_maxColorChannel,
			k_maxColorChannel,
			k_maxColorChannel
		};

		DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;

		std::wstring m_textureName = {};
	};
}