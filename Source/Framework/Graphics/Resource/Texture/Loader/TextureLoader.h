#pragma once

namespace FWK::Graphics
{
	class TextureLoader final
	{
	public:

		 TextureLoader() = default;
		~TextureLoader() = default;

		bool LoadTextureFile(const std::filesystem::path& a_filePath, DirectX::ScratchImage& a_scratchImage, DirectX::TexMetadata& a_texMetadata) const;

	private:

		Converter::TextureBinaryConverter m_binaryConverter = {};
	};
}