#pragma once

namespace FWK::Graphics
{
	class DefaultTexture;
}

namespace FWK::Converter
{
	class DefaultTextureJsonConverter final
	{
	public:

		 DefaultTextureJsonConverter() = default;
		~DefaultTextureJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::DefaultTexture& a_defaultTexture) const;

		nlohmann::json Serialize(const Graphics::DefaultTexture& a_defaultTexture) const;

	private:

		static constexpr std::string_view k_textureNameJsonKey   = "TextureName";
		static constexpr std::string_view k_textureFormatJsonKey = "TextureFormat";
		static constexpr std::string_view k_textureColorJsonKey  = "TextureColor";
	};
}