#pragma once

namespace FWK::Graphics
{
	class TextureLoader final
	{
	public:

		 TextureLoader() = default;
		~TextureLoader() = default;

		TextureLoader(const TextureLoader&)	          = delete;
		TextureLoader(	    TextureLoader&&) noexcept = delete;

		TextureLoader& operator=(const TextureLoader&)			 = delete;
		TextureLoader& operator=(	   TextureLoader&&) noexcept = delete;

		bool LoadTextureFile(const std::filesystem::path&	   a_filePath,
							 const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
								   DirectX::ScratchImage&      a_scratchImage, 
								   DirectX::TexMetadata&       a_texMetadata) const;

	private:

		DirectX::WIC_FLAGS CreateWICFlags(const Enum::TextureLoadColorSpace a_textureLoadColorSpace) const;
	};
}