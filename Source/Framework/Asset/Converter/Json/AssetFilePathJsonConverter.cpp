#include "AssetFilePathJsonConverter.h"

void FWK::Converter::AssetFilePathJsonConverter::Deserialize(const nlohmann::json& a_rootJson, AssetFilePath& a_assetFilePath) const
{
    if (a_rootJson.is_null()) { return; }

    const boost::uuids::uuid& l_assetFilePathUUID = Utility::DeserializeUUID(a_rootJson, k_assetFilePathUUIDJsonKey);

    a_assetFilePath.SetAssetFilePathUUID(l_assetFilePathUUID);
}

nlohmann::json FWK::Converter::AssetFilePathJsonConverter::Serialize(const AssetFilePath& a_assetFilePath) const
{
              nlohmann::json l_rootJson          = {};
    const auto&              l_assetFilePathUUID = a_assetFilePath.GetREFAssetFilePathUUID();

    Utility::UpdateJson(Utility::SerializeUUID(l_assetFilePathUUID, k_assetFilePathUUIDJsonKey), l_rootJson);

    return l_rootJson;
}