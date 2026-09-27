#include "SceneJsonConverter.h"

void FWK::Converter::SceneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
    if (a_rootJson.is_null()) { return; }

    auto& l_assetFilePathRegistry = a_scene.GetMutableREFAssetFilePathRegistry();

    // アセットファイルパスレジストリーのデシリアライズ
    // 一番初めに行う必要がある(PrefabSystemのデシリアライズなどに影響するため)
    if (const auto& l_json = a_rootJson.value(k_assetFilePathRegistryJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        l_assetFilePathRegistry.Deserialize(l_json);
    }

    // シーンチェンジャーのデシリアライズ、
    if (const auto& l_json = a_rootJson.value(k_sceneChanger, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_sceneChanger = a_scene.GetMutableREFSceneChanger();

        l_sceneChanger.Deserialize(l_json, l_assetFilePathRegistry);
    }

    // プレハブシステムのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_gameObjectPrefabSystemJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_gameObjectPrefabSystem = a_scene.GetMutableREFGameObjectPrefabSystem();

        l_gameObjectPrefabSystem.Deserialize(l_json, l_assetFilePathRegistry);
    }

    // ゲームオブジェクトリストのデシリアライズ
    // TODO
}

nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(Scene& a_scene) const
{
          nlohmann::json l_rootJson                = {};
    const auto&          l_name                    = a_scene.GetREFName                         ();
    const auto&          l_sceneChanger            = a_scene.GetREFSceneChanger                 ();
          auto&          l_gameObjectPrefabSystem  = a_scene.GetMutableREFGameObjectPrefabSystem();
    const auto&          l_assetFilePathRegistry   = a_scene.GetREFAssetFilePathRegistry        ();
    
    // アセットレジストリのデシリアライズ
    l_rootJson[k_assetFilePathRegistryJsonKey] = l_assetFilePathRegistry.Serialize();

    // シーンチェンジャーのシリアライズ
    l_rootJson[k_sceneChanger] = l_sceneChanger.Serialize(l_assetFilePathRegistry);

    // プレハブシステムのシリアライズ
    l_rootJson[k_gameObjectPrefabSystemJsonKey] = l_gameObjectPrefabSystem.Serialize(l_assetFilePathRegistry);

    return l_rootJson;
}