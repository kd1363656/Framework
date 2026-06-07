#include "TextureSystemJsonConverter.h"

void FWK::Converter::TextureSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::TextureSystem& a_textureSystem) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_textureStorageJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_textureStorage = a_textureSystem.GetMutableREFTextureStorage();

		l_textureStorage.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::TextureSystemJsonConverter::Serialize(const Graphics::TextureSystem& a_textureSystem) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_textureStorage = a_textureSystem.GetREFTextureStorage();

	l_rootJson[k_textureStorageJsonKey] = l_textureStorage.Serialize();
	
	return l_rootJson;
}