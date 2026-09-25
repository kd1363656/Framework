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

    // シーン遷移マップのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_nextSceneLoadFilePathMapJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializeNextSceneLoadFilePathMap(l_json, a_scene);
    }

    // プレハブシステムのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_prefabSystemJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem();

        l_prefabSystem.Deserialize(l_json, l_assetFilePathRegistry);
    }

    // ゲームオブジェクトリストのデシリアライズ
    // TODO

    const auto& l_sceneName = a_rootJson.value(k_sceneNameJsonKey, std::string{ Constant::k_stringUnknown });
    
    a_scene.SetSceneName(l_sceneName);
}

nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(Scene& a_scene) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_sceneName             = a_scene.GetREFSceneName              ();
          auto& l_prefabSystem          = a_scene.GetMutableREFPrefabSystem    ();
    const auto& l_assetFilePathRegistry = a_scene.GetREFAssetFilePathRegistry  ();
    
    // アセットレジストリのデシリアライズ
    l_rootJson[k_assetFilePathRegistryJsonKey] = l_assetFilePathRegistry.Serialize();

    // シーン遷移マップのシリアライズ
    l_rootJson[k_nextSceneLoadFilePathMapJsonKey] = SerializeNextSceneLoadFilePathMap(a_scene);

    // プレハブシステムのシリアライズ
    l_rootJson[k_prefabSystemJsonKey] = l_prefabSystem.Serialize(l_assetFilePathRegistry);

    l_rootJson[k_sceneNameJsonKey] = l_sceneName;
    
    return l_rootJson;
}