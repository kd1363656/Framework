#pragma once

namespace FWK::Graphics
{
	class TextureLoader final
	{
	public:

		 TextureLoader() = default;
		~TextureLoader() = default;

		bool LoadTextureFile(const std::filesystem::path&	   a_filePath,
							 const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
								   DirectX::ScratchImage&      a_scratchImage, 
								   DirectX::TexMetadata&       a_texMetadata) const;

	private:

		DirectX::WIC_FLAGS CreateWICFlags(const Enum::TextureLoadColorSpace a_textureLoadColorSpace) const;
	};
}