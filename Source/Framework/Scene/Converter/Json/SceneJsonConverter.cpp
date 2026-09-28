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
    if (const auto& l_json = a_rootJson.value(k_gameObjectListJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializeGameObjectList(l_json, a_scene);
    }
}

nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(Scene& a_scene) const
{
    nlohmann::json l_rootJson = {};
 
    const auto& l_sceneChanger           = a_scene.GetREFSceneChanger                 ();
          auto& l_gameObjectPrefabSystem = a_scene.GetMutableREFGameObjectPrefabSystem();
    const auto& l_assetFilePathRegistry  = a_scene.GetREFAssetFilePathRegistry        ();
 
    // シーン名のシリアライズ
    l_rootJson[k_sceneNameJsonKey] = a_scene.GetREFName();
 
    // アセットレジストリのシリアライズ
    l_rootJson[k_assetFilePathRegistryJsonKey] = l_assetFilePathRegistry.Serialize();
 
    // シーンチェンジャーのシリアライズ
    l_rootJson[k_sceneChanger] = l_sceneChanger.Serialize(l_assetFilePathRegistry);
 
    // プレハブシステムのシリアライズ
    l_rootJson[k_gameObjectPrefabSystemJsonKey] = l_gameObjectPrefabSystem.Serialize(l_assetFilePathRegistry);
 
    // ゲームオブジェクトリストのシリアライズ
    l_rootJson[k_gameObjectListJsonKey] = SerializeGameObjectList(a_scene);
 
    return l_rootJson;
}

void FWK::Converter::SceneJsonConverter::DeserializeGameObjectList(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson)) 
    {
        return; 
    }
 
    const auto& l_gameObjectPrefabSystem = a_scene.GetREFGameObjectPrefabSystem();
 
    for (const auto& l_json : a_rootJson)
    {
        if (l_json.is_null()) { continue; }
 
        // prefabUUIDが存在しなければSceneで作成されたゲームオブジェクトなので
        // PrefabSystemからjsonを取得してデシリアライズする処理を避ける
        if (const auto& l_prefabUUID = Utility::DeserializeUUID(l_json, Constant::k_gameObjectJsonConverterPrefabUUIDJsonKey);
            !l_prefabUUID.is_nil())
        {
            // プレハブシステムから該当するプレハブを取得
            const auto* l_prefab = l_gameObjectPrefabSystem.FindPTRPrefab(l_prefabUUID);
 
            if (!l_prefab)
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUIDに対応するPrefabが見つからないため、GameObjectの生成をスキップしました。");
 
                continue;
            }
 
            auto l_gameObject = std::make_shared<GameObject>();
 
            l_gameObject->INIT();
 
            // PrefabJSONをベースにGameObjectを生成
            const auto& l_prefabJson = l_prefab->GetREFJson();

            l_gameObject->DeserializePrefab(l_prefabJson);
 
            // 差分JSONを適用して変更を反映
            if (const auto& l_diffJson = l_json.value(k_diffJsonKey, nlohmann::json{});
                !l_diffJson.is_null())
            {
                nlohmann::json l_mergedJson = l_prefabJson;

                // 差分JSONをPrefabJSONに適用
                ApplyJsonDiff(l_diffJson, l_mergedJson);

                // 差分とプレハブデータを含んだJsonをゲームオブジェクトに渡して復元
                l_gameObject->DeserializeScene(l_mergedJson);
            }
 
            // シーンにゲームオブジェクトを追加
            a_scene.AddGameObject(l_gameObject);
 
            continue;
        }
 
        auto l_gameObject = std::make_shared<GameObject>();
 
        // プレハブが存在しなければシーン情報のみのゲームオブジェクトなので初期化とデシリアライズをしてシーンに追加する
        l_gameObject->INIT();
        l_gameObject->DeserializeScene(l_json);
 
        a_scene.AddGameObject(l_gameObject);
    }
}

nlohmann::json FWK::Converter::SceneJsonConverter::SerializeGameObjectList(const Scene& a_scene) const
{
          nlohmann::json l_gameObjectListJson     = nlohmann::json::array               ();
    const auto&          l_gameObjectPrefabSystem = a_scene.GetREFGameObjectPrefabSystem();

    for (const auto& l_gameObject : a_scene.GetREFGameObjectList())
    {   
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        // プレハブだった場合PrefabUUID,PrefabHierarchyNodeUUID,SceneInstanceUUIDのシリアライズ
        // 差分jsonのシリアライズを行う
        if (l_gameObject->GetVALIsPrefabOrigin())
        {
            const auto& l_prefabUUID = l_gameObject->GetREFPrefabUUID        ();
            const auto* l_prefab     = l_gameObjectPrefabSystem.FindPTRPrefab(l_prefabUUID);
 
            if (!l_prefab)
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUIDに対応するPrefabが見つからないため、GameObjectのシリアライズをスキップしました。");
 
                continue;
            }
 
            nlohmann::json l_json = {};
 
            // PrefabUUIDをシリアライズ
            Utility::UpdateJson(l_json, Utility::SerializeUUID(l_gameObject->GetREFPrefabUUID(), Constant::k_gameObjectJsonConverterPrefabUUIDJsonKey));

            // PrefabHierarchyNodeUUIDをシリアライズ
            Utility::UpdateJson(l_json, Utility::SerializeUUID(l_gameObject->GetREFPrefabHierarchyNodeUUID(), Constant::k_gameObjectJsonConverterPrefabHierarchyNodeUUIDJsonKey));

            // SceneInstanceUUIDをシリアライズ
            Utility::UpdateJson(l_json, Utility::SerializeUUID(l_gameObject->GetREFSceneInstanceUUID(), Constant::k_gameObjectJsonConverterSceneInstanceUUIDJsonKey));
 
            // プレハブゲームオブジェクトなのでtrueで保存
            l_json[Constant::k_gameObjectJsonConverterIsPrefabOriginJsonKey] = true;
 
            // 差分を検出して保存
            const auto& l_prefabJson = l_prefab->GetREFJson();
 
            if (const auto& l_diffJson = DetectPrefabDiff(*l_gameObject, l_prefabJson);
                !l_diffJson.empty())
            {
                l_json[k_diffJsonKey] = l_diffJson;
            }
 
            l_gameObjectListJson.emplace_back(std::move(l_json));
 
            continue;
        }
 
        // プレハブ情報がなければシーンのみのゲームオブジェクトなのでシリアライズして全ての情報を書き出し保存
        l_gameObjectListJson.emplace_back(l_gameObject->Serialize());
    }

    return l_gameObjectListJson;
}

void FWK::Converter::SceneJsonConverter::ApplyJsonDiff(const nlohmann::json& a_diffJson, nlohmann::json& a_baseJson) const
{
    for (const auto& [l_key, l_value] : a_diffJson.items())
    {
        if (l_value.is_object() && 
            a_baseJson.contains(l_key) && 
            a_baseJson[l_key].is_object())
        {
            // オブジェクトは再帰的に適用
            ApplyJsonDiff(l_value, a_baseJson[l_key]);
        }
        else
        {
            // プリミティブまたは配列はそのまま上書き
            a_baseJson[l_key] = l_value;
        }
    }
}

nlohmann::json FWK::Converter::SceneJsonConverter::DetectPrefabDiff(const GameObject& a_gameObject, const nlohmann::json& a_prefabJson) const
{
    const auto& l_currentJson = a_gameObject.Serialize();

    // 渡したゲームオブジェクトとプレハブの違いの部分だけをjsonとして出力する
    return DetectJsonDiff(a_prefabJson, l_currentJson);
}

nlohmann::json FWK::Converter::SceneJsonConverter::DetectJsonDiff(const nlohmann::json& a_baseJson, const nlohmann::json& a_currentJson) const
{
    nlohmann::json l_diffJson = {};

    // オブジェクトでない場合はreturn
    // オブジェクトはキーとペアを持つ
    if (!a_baseJson.is_object() ||
        !a_currentJson.is_object())
    {
        return l_diffJson;
    }

    for (const auto& [l_key, l_value] : a_currentJson.items())
    {
        // 新規追加されたプロパティなら違いのみを集めるjsonに吸収する
        if (!a_baseJson.contains(l_key))
        {
            l_diffJson[l_key] = l_value;

            continue;
        }

        // 値が変更されていないプロパティなら違いはないためcontinue
        if (a_baseJson[l_key] == l_value) { continue; }

        // オブジェクトの場合は再帰的に比較
        if (l_value.is_object() && 
            a_baseJson[l_key].is_object())
        {
            if (const auto& l_subDiff = DetectJsonDiff(a_baseJson[l_key], l_value);
                !l_subDiff.empty())
            {
                l_diffJson[l_key] = l_subDiff;
            }

            continue;
        }

        // プリミティブまたは配列はそのまま保存
        l_diffJson[l_key] = l_value;
    }
}