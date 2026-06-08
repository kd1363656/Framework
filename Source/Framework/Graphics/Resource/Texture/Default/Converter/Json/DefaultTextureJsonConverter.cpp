#include "DefaultTextureJsonConverter.h"

void FWK::Converter::DefaultTextureJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DefaultTexture& a_defaultTexture) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_textureName = a_rootJson.value(k_textureNameJsonKey, std::string{});

	a_defaultTexture.SetTextureName(Utility::StringToWString(l_textureName));

	const auto l_format = a_rootJson.value(k_textureFormatJsonKey, DXGI_FORMAT_R8G8B8A8_UNORM);

	a_defaultTexture.SetFormat(l_format);

	// JsonにTextureColorがない場合は、1.0を初期値と下津
	// これにより、Json側に色を書き忘れても真っ黒にはならない
	if (const auto& l_json = a_rootJson.value(k_textureColorJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		const auto l_rColor = DeserializeColorChannel(l_json, k_textureColorRJsonKey, Constant::k_maxDefaultTextureColorChannelValue);
		const auto l_gColor = DeserializeColorChannel(l_json, k_textureColorGJsonKey, Constant::k_maxDefaultTextureColorChannelValue);
		const auto l_bColor = DeserializeColorChannel(l_json, k_textureColorBJsonKey, Constant::k_maxDefaultTextureColorChannelValue);
		const auto l_aColor = DeserializeColorChannel(l_json, k_textureColorAJsonKey, Constant::k_maxDefaultTextureColorChannelValue);

		a_defaultTexture.ApplyColorChannel(Enum::DefaultTextureColorChannel::R, l_rColor);
		a_defaultTexture.ApplyColorChannel(Enum::DefaultTextureColorChannel::G, l_gColor);
		a_defaultTexture.ApplyColorChannel(Enum::DefaultTextureColorChannel::B, l_bColor);
		a_defaultTexture.ApplyColorChannel(Enum::DefaultTextureColorChannel::A, l_aColor);
	}
}

nlohmann::json FWK::Converter::DefaultTextureJsonConverter::Serialize(const Graphics::DefaultTexture& a_defaultTexture) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_textureNameJsonKey] = Utility::WStringToString(a_defaultTexture.GetREFTextureName());

	l_rootJson[k_textureFormatJsonKey] = a_defaultTexture.GetVALFormat();

	nlohmann::json l_colorJson = {};
	
	l_colorJson[k_textureColorRJsonKey] = a_defaultTexture.FetchVALColorChannel(Enum::DefaultTextureColorChannel::R);
	l_colorJson[k_textureColorGJsonKey] = a_defaultTexture.FetchVALColorChannel(Enum::DefaultTextureColorChannel::G);
	l_colorJson[k_textureColorBJsonKey] = a_defaultTexture.FetchVALColorChannel(Enum::DefaultTextureColorChannel::B);
	l_colorJson[k_textureColorAJsonKey] = a_defaultTexture.FetchVALColorChannel(Enum::DefaultTextureColorChannel::A);

	Utility::UpdateJson(l_rootJson, l_colorJson);

	return l_rootJson;
}

std::uint8_t FWK::Converter::DefaultTextureJsonConverter::DeserializeColorChannel(const nlohmann::json& a_json, const std::string_view a_key, const std::uint8_t a_defaultValue) const
{
	if (a_json.is_null()) { return Constant::k_maxDefaultTextureColorChannelValue; }

	const auto l_value = a_json.value(a_key, a_defaultValue);

	if (l_value > Constant::k_maxDefaultTextureColorChannelValue)
	{
		return Constant::k_maxDefaultTextureColorChannelValue;
	}

	return l_value;
}