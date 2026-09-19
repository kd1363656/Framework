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

    // シーン遷移オブザーバーのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_sceneShiftEventObserverJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_sceneShiftEventObserver = a_scene.GetMutableREFSceneShiftEventObserver();

        l_sceneShiftEventObserver.Deserialize(l_json);
    }

    // プレハブシステムのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_prefabSystemJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem();

        l_prefabSystem.Deserialize(l_json, l_assetFilePathRegistry);
    }

    // ゲームオブジェクトリストのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_gameObjectListJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializeGameObjectList(l_json, a_scene);
    }

    const auto& l_sceneName = a_rootJson.value(k_sceneNameJsonKey, std::string{ Constant::k_stringUnknown });

    a_scene.SetSceneName(l_sceneName);
}

nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(Scene& a_scene) const
{
          nlohmann::json l_rootJson     = {};
    const auto&          l_sceneName    = a_scene.GetREFSceneName                ();
          auto&          l_prefabSystem = a_scene.GetMutableREFPrefabSystem      ();
    const auto& l_assetFilePathRegistry   = a_scene.GetREFAssetFilePathRegistry  ();
    const auto& l_sceneShiftEventObserver = a_scene.GetREFSceneShiftEventObserver();

    // アセットレジストリのデシリアライズ
    l_rootJson[k_assetFilePathRegistryJsonKey] = l_assetFilePathRegistry.Serialize();

    // シーン遷移マップのシリアライズ
    l_rootJson[k_nextSceneLoadFilePathMapJsonKey] = SerializeNextSceneLoadFilePathMap(a_scene);

    // シーンオブザーバーのシリアライズ
    l_rootJson[k_sceneShiftEventObserverJsonKey] = l_sceneShiftEventObserver.Serialize();

    // プレハブシステムのシリアライズ
    l_rootJson[k_prefabSystemJsonKey] = l_prefabSystem.Serialize(l_assetFilePathRegistry);

    // ゲームオブジェクトリストのデシリアライズ
    l_rootJson[k_gameObjectListJsonKey] = SerializeGameObjectList(a_scene);

    l_rootJson[k_sceneNameJsonKey] = l_sceneName;

    return l_rootJson;
}

void FWK::Converter::SceneJsonConverter::DeserializeNextSceneLoadFilePathMap(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson))
    {
        return; 
    }
    
    for (const auto& l_json : a_rootJson)
    {
        const auto& l_sceneUUID = Utility::DeserializeUUID(l_json, k_sceneUUIDJsonKey);

        if (l_sceneUUID.is_nil()) { continue; }

        a_scene.AddNextSceneLoadFilePath(l_sceneUUID);
    }
}
void FWK::Converter::SceneJsonConverter::DeserializeGameObjectList(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
    if (a_rootJson.is_null())              { return; }
    if (!Utility::IsJsonArray(a_rootJson)) { return; }

    for (const auto& l_json : a_rootJson)
    {
        const auto& l_gameObjectJson = l_json.value(k_gameObjectJsonKey, nlohmann::json{});

        if (l_gameObjectJson.is_null()) { continue; }

        auto l_gameObject = std::make_shared<GameObject>();

        // 親PrefabUUIDはRootGameObjectごとに独立して管理する
        // Scene内の別のRootGameObjectが同じPrefabUUIDを持っていても、
        // 親子関係ではないため循環扱いにしない
        std::unordered_set<boost::uuids::uuid> l_parentPrefabUUIDSet = {};

        // Prefab情報を生成した後
        // Scene固有情報としてPrefabInstanceNUMなどを復元する
        // 子ゲームオブジェクトなどをシリアライズしてシーンに登録
        l_gameObject->INIT       ();
        l_gameObject->Deserialize(l_gameObjectJson, l_parentPrefabUUIDSet, a_scene);

        // PrefabInstanceNUMがDeserializeで有効値にならず無効値のままなら
        // Sceneへ登録しない
        if (l_gameObject->GetREFSceneInstanceName().empty() ||
            l_gameObject->GetVALPrefabSceneInstanceNUM() == Constant::k_invalidPrefabSceneInstanceNUM)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabNameまたはPrefabInstanceNUMが無効のため、GameObjectをSceneへ追加できませんでした。");

            continue;
        }

        // シーンに親ゲームオブジェクトを追加
        a_scene.AddGameObject(l_gameObject);
    }
}

nlohmann::json FWK::Converter::SceneJsonConverter::SerializeNextSceneLoadFilePathMap(const Scene& a_scene) const
{
          auto  l_rootJsonArray            = nlohmann::json::array                 ();
    const auto& l_nextSceneLoadFilePathMap = a_scene.GetREFNextSceneLoadFilePathMap();

    const auto& l_assetFilePathRegistry = a_scene.GetREFAssetFilePathRegistry();

    for (const auto& [l_sceneUUID, l_nextSceneLoadFilePath] : l_nextSceneLoadFilePathMap)
    {
        if (l_sceneUUID.is_nil()) { continue; }

        const auto* l_assetFilePathData = l_assetFilePathRegistry.FindPTRAssetFilePathData(l_sceneUUID);

        if (!l_assetFilePathData) { continue; }

        // ファイルパが一致しなければcontinue
        if (l_assetFilePathData->m_assetFilePath.empty() ||
            l_nextSceneLoadFilePath.empty()              ||
            l_assetFilePathData->m_assetFilePath != l_nextSceneLoadFilePath)
        {
            continue;
        }

        nlohmann::json l_json = {};

        Utility::UpdateJson(l_json, Utility::SerializeUUID(l_sceneUUID, k_sceneUUIDJsonKey));

        l_rootJsonArray.emplace_back(l_json);
    }

    return l_rootJsonArray;
}
nlohmann::json FWK::Converter::SceneJsonConverter::SerializeGameObjectList(const Scene& a_scene) const
{
          auto  l_rootJsonArray  = nlohmann::json::array       ();
    const auto& l_gameObjectList = a_scene.GetREFGameObjectList();

    for (const auto& l_gameObject : l_gameObjectList)
    {
        // プレハブ名がないなら不正なゲームオブジェクトとしてシリアライズしない
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        // 子GameObjectは親GameObjectのChildListへ保存されているため
        // Scene直下にRootGameObjectだけを保存する
        if (!l_gameObject->GetREFParent().expired()) { continue; }

        // PrefabUUIDが無効値、シーンインスタンス名が空、
        // プレハブインスタンスナンバーが無効値ならシリアライズ処理を行わない
        if (l_gameObject->GetREFPrefabUUID().is_nil()       ||
            l_gameObject->GetREFSceneInstanceName().empty() ||
            l_gameObject->GetVALPrefabSceneInstanceNUM() == Constant::k_invalidPrefabSceneInstanceNUM)
        {
            continue;
        }

        nlohmann::json l_json = {};

        l_json[k_gameObjectJsonKey] = l_gameObject->SerializeScene();

        l_rootJsonArray.emplace_back(l_json);
    }

    return l_rootJsonArray;
}