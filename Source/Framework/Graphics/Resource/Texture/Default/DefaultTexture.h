#pragma once

namespace FWK::Graphics
{
	class DefaultTexture
	{
		using DefaultTextureColor = std::array<std::uint8_t, Constant::k_defaultTextureColorChannelCount>;

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
											const TypeAlias::StorageID				a_storageID,
												  TypeAlias::SRVDescriptorPool&     a_srvDescriptorPool,
												  Struct::TextureBatchUploadRecord& a_textureBatchUploadRecord);

		void ApplyColorChannel(const Enum::DefaultTextureColorChannel a_colorChannel, const std::uint8_t a_colorValue);

		void SetFormat(const DXGI_FORMAT a_set) { m_format = a_set; }

		void SetTextureName(const std::wstring& a_set) { m_textureName = a_set; }

		std::uint8_t FetchVALColorChannel(const Enum::DefaultTextureColorChannel a_colorChannel) const;

		const auto& GetREFTextureRecord() const { return m_textureRecord; }

		const auto& GetREFTextureName() const { return m_textureName; }

		auto GetVALFormat() const { return m_format; }

	private:

		bool CreateScratchImage(DirectX::ScratchImage& a_scratchImage) const;

		static constexpr std::size_t k_defaultTextureWidth     = 1ULL;
		static constexpr std::size_t k_defaultTextureHeight    = 1ULL;
		static constexpr std::size_t k_defaultTextureArraySize = 1ULL;
		static constexpr std::size_t k_defaultTextureMipLevels = 1ULL;

		static constexpr std::size_t k_defaultTextureMipIndex   = 0ULL;
		static constexpr std::size_t k_defaultTextureItemIndex  = 0ULL;
		static constexpr std::size_t k_defaultTextureSliceIndex = 0ULL;


		std::array<std::uint8_t, Constant::k_defaultTextureColorChannelCount> m_color = 
		{
			Constant::k_maxDefaultTextureColorChannelValue,
			Constant::k_maxDefaultTextureColorChannelValue,
			Constant::k_maxDefaultTextureColorChannelValue,
			Constant::k_maxDefaultTextureColorChannelValue
		};

		std::weak_ptr<Graphics::TextureRecord> m_textureRecord = {};

		Converter::DefaultTextureJsonConverter m_jsonConverter = {};

		DXGI_FORMAT m_format = DXGI_FORMAT_R8G8B8A8_UNORM;

		std::wstring m_textureName = {};
	};
}