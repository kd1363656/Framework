#include "AssetFilePathUtilityJsonConverter.h"

void FWK::Converter::AssetFilePathJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Utility::AssetFilePath& a_assetFilePath) const
{
	if (a_rootJson.is_null()) { return; }

	const std::filesystem::path& l_assetFilePath = a_rootJson.value(k_assetFilePathJsonKey.data(), std::filesystem::path{});

	a_assetFilePath.SetAssetFilePath(l_assetFilePath);
}

nlohmann::json FWK::Converter::AssetFilePathJsonConverter::Serialize(const Utility::AssetFilePath & a_assetFilePath) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_assetFilePathJsonKey] = a_assetFilePath.GetREFAssetFilePath();

	return l_rootJson;
}