#include "ApplicationJsonConverter.h"
#include "../../Application.h"

void Converter::ApplicationJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Application& a_application) const
{
    if (a_rootJson.is_null()) { return; }

    // アセットファイルパスレジストリーのデシリアライズ
    // 一番初めに行う必要がある(PrefabSystemのデシリアライズなどに影響するため)
    if (const auto& l_json = a_rootJson.value(k_assetFilePathRegistryJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_assetFilePathRegistry = a_application.GetMutableREFAssetFilePathRegistry();

        l_assetFilePathRegistry.Deserialize(l_json);
    }
}

nlohmann::json Converter::ApplicationJsonConverter::Serialize(const Application& a_application) const
{
          nlohmann::json l_rootJson              = {};
    const auto&          l_assetFilePathRegistry = a_application.GetREFAssetFilePathRegistry();

    // アセットレジストリのシリアライズ
    l_rootJson[k_assetFilePathRegistryJsonKey] = l_assetFilePathRegistry.Serialize();

    return l_rootJson;
}