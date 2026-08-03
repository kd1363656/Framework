#include "AssetFilePathHelperUtilityJsonConverter.h"

void FWK::Converter::AssetFilePathHelperJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Utility::AssetFilePathHelper& a_assetFilePathHelper) const
{
	if (a_rootJson.is_null()) { return; }

	const std::filesystem::path& l_assetFilePath = a_rootJson.value(k_assetFilePathJsonKey.data(), std::filesystem::path{});

	a_assetFilePathHelper.SetAssetFilePath(l_assetFilePath);
}

nlohmann::json FWK::Converter::AssetFilePathHelperJsonConverter::Serialize(const Utility::AssetFilePathHelper& a_assetFilePathHelper) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_assetFilePathJsonKey] = a_assetFilePathHelper.GetREFAssetFilePath();

	return l_rootJson;
}