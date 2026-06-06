#pragma once

namespace FWK::Graphics
{
	class TextureSystem
	{
	public:

		 TextureSystem() = default;
		~TextureSystem() = default;

		bool LoadTextureFile(const std::filesystem::path& a_filePath, const Enum::TextureLoadType a_loadType = Enum::TextureLoadType::Auto);

	private:

		TextureLoader m_loader = {};
	};
}