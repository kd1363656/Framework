#include "SceneChangerJsonConverter.h"

void FWK::Converter::SceneChangerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, SceneChanger& a_sceneChanger) const
{
    if (a_rootJson.is_null()) { return; }

    if (const auto& l_json = a_rootJson.value(k_nextSceneMapJsonKey, nlohmann::json{});
        !l_json.is_null() &&
        Utility::IsJsonArray(l_json))
    {
        DeserializeNextSceneMap(l_json, a_assetFilePathRegistry, a_sceneChanger);
    }

    if (const auto& l_json = a_rootJson.value(k_sceneChangeEventObserverJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_sceneChangeEventObserver = a_sceneChanger.GetMutableREFSceneChangeEventObserver();

        l_sceneChangeEventObserver.Deserialize(l_json);
    }
}

nlohmann::json FWK::Converter::SceneChangerJsonConverter::Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry, const SceneChanger& a_sceneChanger) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_sceneChangeEventObserver = a_sceneChanger.GetREFSceneChangeEventObserver();

    l_rootJson[k_nextSceneMapJsonKey]             = SerializeNextSceneMap               (a_assetFilePathRegistry, a_sceneChanger);
    l_rootJson[k_sceneChangeEventObserverJsonKey] = l_sceneChangeEventObserver.Serialize();

    return l_rootJson;
}

void FWK::Converter::SceneChangerJsonConverter::DeserializeNextSceneMap(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, SceneChanger& a_sceneChanger) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "RootJsonが無効か配列でないため、PrefabSystemのデシリアライズに失敗しました。");

        return;
    }

    for (const auto& l_json : a_rootJson)
    {
        if (l_json.is_null()) { continue; }

        const auto& l_sceneUUID = Utility::DeserializeUUID(l_json, k_uuidJsonKey);

        // 保存されていたUUIDを復元できなかった場合
        // ここで新しいUUIDを発行してはいけない
        // Scene上のGameObjectが保持するSceneUUIDとの
        // 対応関係が壊れてしまうため
        // このPrefab自体を登録しない
        if (l_sceneUUID.is_nil())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効のため、NextSceneを登録できませんでした。");

            continue;
        }

        const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_sceneUUID);

        if (!l_assetFilePathData)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryにSceneUUIDからファイルパスの取得に失敗しました。");

            continue;
        }

        // プレハブじゃないファイルパスならcontinue
        if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene) { continue; }

        Struct::NextSceneData l_nextSceneData = {};

        const auto& l_assetFilePath = l_assetFilePathData->m_assetFilePath;

        if (!Utility::CanLoadFilePath(l_assetFilePath, Constant::k_lowerJsonExtension)) { continue; }

        // シーンの読み込むファイrパスと名前を
        l_nextSceneData.m_name     = l_json.value(k_nextSceneNameJsonKey, std::string{});
        l_nextSceneData.m_filePath = l_assetFilePath;
        
        a_sceneChanger.AddNextSceneData(l_sceneUUID, l_nextSceneData);
    }
}

nlohmann::json FWK::Converter::SceneChangerJsonConverter::SerializeNextSceneMap(const AssetFilePathRegistry & a_assetFilePathRegistry, const SceneChanger & a_sceneChanger) const
{
    auto l_rootJsonArray = nlohmann::json::array();

    auto& l_nextSceneDataMap = a_sceneChanger.GetREFNextSceneDataMap();

    for (auto& [l_sceneUUID, l_nextSceneData] : l_nextSceneDataMap)
    {
        // nilの場合はSceneChangerMapへ本来登録されないが
        // 異常なデータをJSONへ保存されないように念のため除外する
        if (l_sceneUUID.is_nil()) { continue; }

        const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_sceneUUID);

        if (!l_assetFilePathData)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryにSceneUUIDからファイルパスの取得に失敗しました。");

            continue;
        }

        // プレハブじゃないファイルパスならcontinue
        if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene) { continue; }

        // 読み込めないファイルならシリアライズしない
        if (const auto& l_filePath = l_assetFilePathData->m_assetFilePath;
            !Utility::CanLoadFilePath(l_filePath)) 
        {
            continue; 
        }

        nlohmann::json l_json = {};

        Utility::UpdateJson(l_json, Utility::SerializeUUID(l_sceneUUID, k_uuidJsonKey));
        
        l_json[k_nextSceneNameJsonKey] = l_nextSceneData.m_name;

        l_rootJsonArray.emplace_back(l_json);
    }

    return l_rootJsonArray;
}