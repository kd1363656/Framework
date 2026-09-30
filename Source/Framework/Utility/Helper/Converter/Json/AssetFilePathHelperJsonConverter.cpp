#include "AssetFilePathHelperJsonConverter.h"

void FWK::Converter::AssetFilePathHelperJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Utility::AssetFilePathHelper& a_assetFilePathHelper) const
{
    if (a_rootJson.is_null()) { return; }

    const boost::uuids::uuid& l_assetFilePathUUID = Utility::DeserializeUUID(a_rootJson, k_assetFilePathUUIDJsonKey);

    a_assetFilePathHelper.SetAssetFilePathUUID(l_assetFilePathUUID);
}

nlohmann::json FWK::Converter::AssetFilePathHelperJsonConverter::Serialize(const Utility::AssetFilePathHelper& a_assetFilePathHelper) const
{
          nlohmann::json l_rootJson          = {};
    const auto&          l_assetFilePathUUID = a_assetFilePathHelper.GetREFAssetFilePathUUID();

    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_assetFilePathUUID, k_assetFilePathUUIDJsonKey));

    return l_rootJson;
}