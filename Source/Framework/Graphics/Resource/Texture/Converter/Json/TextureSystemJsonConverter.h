#pragma once

namespace FWK::Graphics
{
	class TextureSystem;
}

namespace FWK::Converter
{
	class TextureSystemJsonConverter final
	{
	public:

		 TextureSystemJsonConverter() = default;
		~TextureSystemJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::TextureSystem& a_textureSystem) const;

		nlohmann::json Serialize(const Graphics::TextureSystem& a_textureSystem) const;

	private:

		static constexpr std::string_view k_textureStorageJsonKey = "TextureStorage";
	};
}