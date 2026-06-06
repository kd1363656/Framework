#pragma once

namespace FWK::Converter
{
	class TextureBinaryConverter final : public BinaryFileConverterBase
	{
	private:

		struct TextureAssetHeader final : public Struct::AssetHeaderBase
		{
			std::uint64_t m_width  = Constant::k_emptyTextureWidth;
			std::uint64_t m_height = Constant::k_emptyTextureHeight;
			
		};

	public:

		 TextureBinaryConverter()		   = default;
		~TextureBinaryConverter() override = default;

	private:

		static constexpr std::uint64_t k_emptyTextureAssetFileSize = 0ULL;
		
		static constexpr std::uint64_t k_emptyPixelDataSize = 0ULL;
		static constexpr std::uint64_t k_emptyImageCount    = 0ULL;
		static constexpr std::uint64_t k_emptyArraySize     = 0ULL;
		static constexpr std::uint64_t k_emptyMipLevels     = 0ULL;

		// 'T' = 0x54, 'X' = 0x58のため、0x5458でTexture用Assetを表す
		static constexpr std::uint16_t k_textureAssetTypeID = 0x5458U;

		// 保存形式を変えた場合は、古い.textureを読み込まない用に必ず上げる
		static constexpr std::uint16_t k_textureAssetVersion = 0U;
	};
}