#pragma once

namespace FWK::Converter
{
	class TextureBinaryConverter final : public BinaryFileConverterBase
	{
	private:

		struct TextureBinaryHeader final
		{
			std::uint64_t m_fileSize    = Constant::k_emptyAssetFileSize;
			std::uint16_t m_version     = Constant::k_emptyAssetVersion;
			std::uint16_t m_assetTypeID = Constant::k_emptyAssetTypeID;

			std::uint64_t m_width            = Constant::k_emptyTextureWidth;
			std::uint64_t m_height           = Constant::k_emptyTextureHeight;
			std::uint64_t m_depth            = k_emptyTextureDepth;
			std::uint64_t m_arraySize        = Constant::k_defaultTexture2DArraySize;
			std::uint64_t m_mipLevels        = Constant::k_defaultTexture2DMipLevels;
			std::uint64_t m_subresourceCount = k_emptyTextureSubresourceCount;

			// フォーマットは本来ならEnumだがAPI側では絶対に変更できないEnumなので
			// 保守性は高いと判断しEnumにロード時に復元する前提で整数値として保存
			std::uint32_t m_format = static_cast<std::uint32_t>(DXGI_FORMAT_UNKNOWN);

			std::uint32_t m_dimension = k_emptyTextureDimension;

			// CubeMapなどの追加情報
			std::uint32_t m_miscFlags = k_emptyTextureMiscFlags;

			// AlphaModeなどの追加情報
			std::uint32_t m_miscFlags2 = k_emptyTextureMiscFlags2;
		};

		struct TextureBinarySubresourceHeader final
		{
			std::uint64_t m_width         = Constant::k_emptyTextureWidth;
			std::uint64_t m_height        = Constant::k_emptyTextureHeight;
			std::uint64_t m_rowPitch      = k_emptyTextureRowPitch;
			std::uint64_t m_slicePitch    = k_emptyTextureSlicePitch;
			std::uint64_t m_pixelDataSize = k_emptyTexturePixelDataSize;
		};

	public:

		 TextureBinaryConverter()		   = default;
		~TextureBinaryConverter() override = default;

		TextureBinaryConverter(const TextureBinaryConverter&)	     = delete;
		TextureBinaryConverter(	  TextureBinaryConverter&&) noexcept = delete;

		TextureBinaryConverter& operator=(const TextureBinaryConverter&)			= delete;
		TextureBinaryConverter& operator=(	     TextureBinaryConverter&&) noexcept = delete;

		bool LoadTextureAsset(const std::filesystem::path& a_filePath, DirectX::ScratchImage& a_scratchImage, DirectX::TexMetadata& a_texMetadata);

		bool SaveTextureAsset(const std::filesystem::path& a_filePath, const DirectX::ScratchImage& a_scratchImage);

	private:

		bool CanLoadTextureAsset(const std::filesystem::path& a_filePath) const;

		std::filesystem::path CreateTextureAssetFilePath(const std::filesystem::path& a_filePath) const;

		TextureBinaryHeader CreateTextureBinaryHeader(const DirectX::ScratchImage& a_scratchImage, const std::uint64_t& a_fileSize) const;

		TextureBinarySubresourceHeader CreateTextureBinarySubresourceHeader(const DirectX::Image& a_image) const;

		DirectX::TexMetadata CreateTexMetadata(const TextureBinaryHeader& a_textureBinaryHeader) const;

		std::uint64_t CalculateTextureAssetFileSize(const DirectX::ScratchImage& a_scratchImage) const;

		static constexpr std::uint64_t k_emptyTextureDepth            = 0ULL;
		static constexpr std::uint64_t k_emptyTextureSubresourceCount = 0ULL;

		static constexpr std::uint64_t k_emptyTextureRowPitch      = 0ULL;
		static constexpr std::uint64_t k_emptyTextureSlicePitch    = 0ULL;
		static constexpr std::uint64_t k_emptyTexturePixelDataSize = 0ULL;
		
		static constexpr std::uint32_t k_emptyTextureDimension  = 0U;
		static constexpr std::uint32_t k_emptyTextureMiscFlags  = 0U;
		static constexpr std::uint32_t k_emptyTextureMiscFlags2 = 0U;

		// 'T' = 0x54,'X' = 0x58のため、0x5458で"TX"を表す
		static constexpr std::uint16_t k_textureAssetTypeID = 0x5458U;

		// ※ 注意 : TextureAssetとして保存する構造体が変化したらバージョンを上げる
		static constexpr std::uint16_t k_textureAssetVersion = 1U;
	};
}