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

	if (const auto& l_json = a_rootJson.value(k_defaultTextureListJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeDefaultTextureList(l_json, a_textureSystem);
	}
}

nlohmann::json FWK::Converter::TextureSystemJsonConverter::Serialize(const Graphics::TextureSystem& a_textureSystem) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_textureStorage = a_textureSystem.GetREFTextureStorage();

	l_rootJson[k_textureStorageJsonKey] = l_textureStorage.Serialize();
	
	l_rootJson[k_defaultTextureListJsonKey] = SerializeDefaultTextureList(a_textureSystem);

	return l_rootJson;
}

void FWK::Converter::TextureSystemJsonConverter::DeserializeDefaultTextureList(const nlohmann::json& a_rootJson, Graphics::TextureSystem& a_textureSystem) const
{
	if (a_rootJson.is_null())          { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (std::size_t l_i = 0ULL; l_i < Graphics::TextureSystem::GetREFDefaultTextureTypeCount(); ++l_i)
	{
		const auto& l_json = a_rootJson[l_i];

		const auto& l_defaultTextureJson = l_json.value(k_defaultTextureJsonKey,     nlohmann::json{});
		const auto& l_defaultTextureType = l_json.value(k_defaultTextureTypeJsonKey, Enum::DefaultTextureType::Count);

		// デフォルトテクスチャタイプの値がDefaultTextureTypeのCountを超えていればreturn;
		if (static_cast<std::size_t>(l_defaultTextureType) >= static_cast<std::size_t>(Enum::DefaultTextureType::Count)) { continue; }

		Graphics::DefaultTexture l_defaultTexture = {};

		l_defaultTexture.Deserialize(l_defaultTextureJson);

		// デシリアライズ後のデフォルトテクスチャを格納
		a_textureSystem.ApplyDefaultTexture(l_defaultTextureType, std::move(l_defaultTexture));
	}
}

nlohmann::json FWK::Converter::TextureSystemJsonConverter::SerializeDefaultTextureList(const Graphics::TextureSystem& a_textureSystem) const
{
	nlohmann::json l_rootJsonArray = nlohmann::json::array();

	const auto& l_defaultTextureList = a_textureSystem.GetREFDefaultTextureList();

	for (std::size_t l_i = 0ULL; l_i < l_defaultTextureList.size(); ++l_i)
	{
		const auto& l_defaultTexture = l_defaultTextureList[l_i];

		// 名前が空かどうかを確認
		if (l_defaultTexture.GetREFTextureName().empty()) { continue; }

		const auto l_defaultTextureType = static_cast<Enum::DefaultTextureType>(l_i);

		// Countは実態を持つDefautltTextureではないので保存しない
		if (static_cast<std::size_t>(l_defaultTextureType) >= static_cast<std::size_t>(Enum::DefaultTextureType::Count)) { continue; }

		nlohmann::json l_json = {};

		// DefaultTextureTypeを保存
		l_json[k_defaultTextureTypeJsonKey] = l_defaultTextureType;

		l_json[k_defaultTextureJsonKey] = l_defaultTexture.Serialize();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}