#include "SceneGameObjectPrefabSystemJsonConverter.h"

void FWK::Converter::ScenePrefabSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, SceneGameObjectPrefabSystem& a_sceneGameObjectPrefabSystem) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson, k_prefabMapJsonKey))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "RootJsonが無効か配列でないため、GameObjectPrefabSystemのデシリアライズに失敗しました。");

        return;
    }

    const auto& l_jsonArray = a_rootJson[k_prefabMapJsonKey];

    for (const auto& l_json : l_jsonArray)
    {
        if (l_json.is_null()) { continue; }

        const auto& l_prefabUUID = Utility::DeserializeUUID(l_json, k_uuidJsonKey);

        // 保存されていたUUIDを復元できなかった場合
        // ここで新しいUUIDを発行してはいけない
        // Scene上のGameObjectが保持するGameObjectPrefabUUIDとの
        // 対応関係が壊れてしまうため
        // このGameObjectPrefab自体を登録しない
        if (l_prefabUUID.is_nil())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabUUIDが無効のため、GameObjectPrefabDataを登録できませんでした。");

            continue;
        }

        const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_prefabUUID);

        if (!l_assetFilePathData)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryにGameObjectPrefabUUIDからファイルパスの取得に失敗しました。");

            continue;
        }

        // プレハブじゃないファイルパスならcontinue
        if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab) { continue; }

        GameObjectPrefab l_gameObjectPrefab = {};

        const auto& l_assetFilePath = l_assetFilePathData->m_assetFilePath;

        if (!Utility::CanLoadFilePath(l_assetFilePath, Constant::k_lowerJsonExtension)) { continue; }

        // FilePathやGameObjectPrefabNameの復元、
        // 実GameObjectPrefabファイルの読み込みはPrefab自身へ任せる
        l_gameObjectPrefab.Load(l_assetFilePath);

        // GameObjectPrefabファイル自体を読み込めなかった場合は、
        // SceneGameObjectPrefabSystemへ不完全なPrefabを登録しない
        // Scene上のGameObjectにGameObjectPrefabUUIDが残っていれば後から「GameObjectPrefabUUIDはあるがGameObjectPrefabSystemには存在しない」
        // 壊れた参照として判定できる
        if (l_gameObjectPrefab.GetREFJson().is_null())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabのJsonを読み込めなかったため、GameObjectPrefabDataを登録できませんでした。");

            continue;
        }

        a_sceneGameObjectPrefabSystem.AddPrefab(l_prefabUUID, l_gameObjectPrefab);
    }
}

nlohmann::json FWK::Converter::ScenePrefabSystemJsonConverter::Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry, SceneGameObjectPrefabSystem& a_sceneGameObjectPrefabSystem) const
{
    nlohmann::json l_rootJson  = {};
    auto           l_jsonArray = nlohmann::json::array();

    const auto& l_prefabMap = a_sceneGameObjectPrefabSystem.GetREFPrefabMap();

    for (const auto& [l_prefabUUID, l_prefab] : l_prefabMap)
    {
        // nilの場合はGameObjectPrefabMapへ本来登録されないが
        // 異常なデータをJSONへ保存されないように念のため除外する
        if (l_prefabUUID.is_nil()) { continue; }

        const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_prefabUUID);

        if (!l_assetFilePathData)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryにGameObjectPrefabUUIDからファイルパスの取得に失敗しました。");

            continue;
        }

        // プレハブじゃないファイルパスならcontinue
        if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab) { continue; }

        // 読み込めないファイルならシリアライズしない
        if (const auto& l_filePath = l_assetFilePathData->m_assetFilePath;
            !Utility::CanLoadFilePath(l_filePath) ||
            l_prefab.GetREFJson().is_null())
        {
            continue;
        }

        nlohmann::json l_json = {};

        Utility::UpdateJson(l_json, Utility::SerializeUUID(l_prefabUUID, k_uuidJsonKey));
        
        l_jsonArray.emplace_back(l_json);
    }

    l_rootJson[k_prefabMapJsonKey] = l_jsonArray;

    return l_rootJson;
}