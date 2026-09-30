#include "SceneJsonConverter.h"
#include "../../../../Application/Application.h"

void FWK::Converter::SceneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_application           = Application::GetInstance                 ();
    const auto& l_assetFilePathRegistry = l_application.GetREFAssetFilePathRegistry();

    // シーンチェンジャーのデシリアライズ
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
    // PrefabSystemより後に行う必要がある(PrefabUUIDからPrefab基底Jsonを引くため)
    if (const auto& l_json = a_rootJson.value(k_gameObjectListJsonKey, nlohmann::json{});
        !l_json.is_null() &&
        Utility::IsJsonArray(l_json))
    {
        DeserializeGameObjectList(l_json, a_scene);
    }

    // ライトシステムのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_lightSystemJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_lightSystem = a_scene.GetMutableREFLightSystem();

        l_lightSystem.Deserialize(l_json);
    }
}

nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(Scene& a_scene) const
{
    nlohmann::json l_rootJson = {};
 
    const auto& l_sceneChanger           = a_scene.GetREFSceneChanger                 ();
          auto& l_gameObjectPrefabSystem = a_scene.GetMutableREFGameObjectPrefabSystem();
    const auto& l_lightSystem            = a_scene.GetREFLightSystem                  ();    
    const auto& l_application            = Application::GetInstance                   ();
    const auto& l_assetFilePathRegistry  = l_application.GetREFAssetFilePathRegistry  ();

    // シーン名のシリアライズ
    l_rootJson[k_sceneNameJsonKey] = a_scene.GetREFName();
 
    // シーンチェンジャーのシリアライズ
    l_rootJson[k_sceneChanger] = l_sceneChanger.Serialize(l_assetFilePathRegistry);
 
    // プレハブシステムのシリアライズ
    l_rootJson[k_gameObjectPrefabSystemJsonKey] = l_gameObjectPrefabSystem.Serialize(l_assetFilePathRegistry);
 
    // ゲームオブジェクトリストのシリアライズ
    l_rootJson[k_gameObjectListJsonKey] = SerializeGameObjectList(a_scene);

    // ライトシステムのシリアライズ
    l_rootJson[k_lightSystemJsonKey] = l_lightSystem.Serialize();

    return l_rootJson;
}

void FWK::Converter::SceneJsonConverter::DeserializeGameObjectList(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
    if (!a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson))
    {
        return; 
    }

    const auto& l_prefabSystem = a_scene.GetREFGameObjectPrefabSystem();

    // 配列順に処理 = GameObjectListの並びがそのまま登録順になる
    for (const auto& l_json : a_rootJson)
    {
        // Owner設定等の初期化が必要なのでmake_sharedで生成してINITを呼ぶ
        auto l_gameObject = std::make_shared<GameObject>();

        l_gameObject->INIT();

        // ルートGameObjectは親がいないのでPrefab基底無しで直接DeserializeSceneへ
        // SceneへのAddGameObject登録と子孫の再帰読み込みはGameObject側が行う
        l_gameObject->DeserializeScene(l_json,
                                       nlohmann::json{},
                                       l_prefabSystem,
                                       a_scene);
    }
}

nlohmann::json FWK::Converter::SceneJsonConverter::SerializeGameObjectList(Scene& a_scene) const
{
          auto  l_jsonArray      = nlohmann::json::array                      ();
          auto& l_prefabSystem   = a_scene.GetMutableREFGameObjectPrefabSystem();
    const auto& l_gameObjectList = a_scene.GetREFGameObjectList               ();

    for (const auto& l_gameObject : l_gameObjectList)
    {
        if (!l_gameObject) { continue; }

        // 親がいる = 子なのでスキップ
        // m_gameObjectListは子も含むフラットなリストだが、
        // 子は親のChildList経由で再帰的に保存されるためルートのみを書き出す
        if (const auto& l_hierarchy = l_gameObject->GetREFHierarchy();
            !l_hierarchy.GetREFParent().expired()) 
        {
            continue; 
        }

        // Prefab由来なら差分形式、非Prefabならフル形式になるのはGameObject側で判定する
        l_jsonArray.emplace_back(l_gameObject->SerializeScene(l_prefabSystem));
    }

    return l_jsonArray;
}