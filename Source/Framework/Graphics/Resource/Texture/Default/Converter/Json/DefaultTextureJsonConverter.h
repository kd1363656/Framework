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

		std::uint8_t DeserializeColorChannel(const nlohmann::json& a_json, const std::string_view a_key, const std::uint8_t a_defaultValue) const;

		static constexpr std::string_view k_textureNameJsonKey   = "TextureName";
		static constexpr std::string_view k_textureFormatJsonKey = "TextureFormat";
		static constexpr std::string_view k_textureColorJsonKey  = "TextureColor";

		static constexpr std::string_view k_textureColorRJsonKey = "R";
		static constexpr std::string_view k_textureColorGJsonKey = "G";
		static constexpr std::string_view k_textureColorBJsonKey = "B";
		static constexpr std::string_view k_textureColorAJsonKey = "A";
	};
}