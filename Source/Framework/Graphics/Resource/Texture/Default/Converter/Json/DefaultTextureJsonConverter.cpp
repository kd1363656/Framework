#include "DefaultTextureJsonConverter.h"

void FWK::Converter::DefaultTextureJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DefaultTexture& a_defaultTexture) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_textureName = a_rootJson.value(k_textureNameJsonKey, std::string{});

	a_defaultTexture.SetTextureName(Utility::StringToWString(l_textureName));

	const auto l_format = a_rootJson.value(k_textureFormatJsonKey, DXGI_FORMAT_R8G8B8A8_UNORM);

	a_defaultTexture.SetFormat(l_format);

	a_defaultTexture.SetColor(Utility::DeserializeColor(a_rootJson, k_textureColorJsonKey));
}

nlohmann::json FWK::Converter::DefaultTextureJsonConverter::Serialize(const Graphics::DefaultTexture& a_defaultTexture) const
{
	nlohmann::json l_rootJson = {};



	l_rootJson[k_textureFormatJsonKey] = a_defaultTexture.GetVALFormat();

	Utility::UpdateJson(l_rootJson, Utility::SerializeColor(a_defaultTexture.GetREFColor(), k_textureColorJsonKey));

	return nlohmann::json();
}
