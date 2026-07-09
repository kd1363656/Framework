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

		// ワイド文字列キー用の等価ハッシュ構造体
		struct WStringHash final
		{
			// 透過ハッシュであることをSTLに通知するための宣言
			using is_transparent = void;
		
			// 受け取った文字列をstd::wstring_viewとして扱い、ハッシュ値を計算する
			std::size_t operator()(const std::wstring_view& a_key) const { return std::hash<std::wstring_view>{}(a_key); }
		};

	public:

		 DefaultTextureJsonConverter() = default;
		~DefaultTextureJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::DefaultTexture& a_defaultTexture) const;

		nlohmann::json Serialize(const Graphics::DefaultTexture& a_defaultTexture) const;

		static constexpr std::size_t k_defaultTextureColorChannelCount = static_cast<std::size_t>(Enum::DefaultTextureColorChannel::Count);

		static constexpr std::uint8_t k_maxDefaultTextureColorChannelValue = 255U;

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