#include "SceneChanger.h"

void FWK::SceneChanger::INIT()
{
    m_nextSceneDataMap.clear();

    m_sceneChangeEventObserver.INIT();
}

void FWK::SceneChanger::Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, a_assetFilePathRegistry, *this);
}

nlohmann::json FWK::SceneChanger::Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    return m_jsonConverter.Serialize(a_assetFilePathRegistry, *this);
}

bool FWK::SceneChanger::AddNextSceneData(const boost::uuids::uuid& a_sceneUUID, const Struct::NextSceneData& a_nextSceneData)
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneDataMapに追加できませんでした。");

        return false;
    }

    if (!m_nextSceneDataMap.try_emplace(a_sceneUUID, a_nextSceneData).second)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "同じSceneUUIDが既に登録されており、SceneChangerのNextSceneDataMapに追加できませんでした。");
    }

    return true;
}
bool FWK::SceneChanger::AddNextSceneData(const boost::uuids::uuid& a_sceneUUID, const AssetFilePathRegistry& a_assetFilePathRegistry)
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneDataMapに追加できませんでした。");
 
        return false;
    }
 
    const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID);
 
    if (!l_assetFilePathData)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryでSceneUUIDに対応するAssetFilePathDataの取得に失敗しました。");
 
        return false;
    }
 
    // Scene以外のAssetはシーン遷移へ追加できない
    if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Scene以外のAssetをシーン遷移へ追加しようとしました。");
 
        return false;
    }
 
    const auto& l_nextSceneFilePath = l_assetFilePathData->m_assetFilePath;
 
    if (!Utility::CanLoadFilePath(l_nextSceneFilePath, Constant::k_lowerJsonExtension))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスがjsonファイルでないか、無効な形式のファイルです。\nFilePath : {}", l_nextSceneFilePath.string());
 
        return false;
    }
 
    Struct::NextSceneData l_nextSceneData = {};
 
    l_nextSceneData.m_filePath = l_nextSceneFilePath;

    // シーン名はファイル名(stem)を使う
    // CreateSceneもStemをScene名として保存しているため一致する
    l_nextSceneData.m_name = l_nextSceneFilePath.stem().string();
 
    return AddNextSceneData(a_sceneUUID, l_nextSceneData);
}

bool FWK::SceneChanger::RemoveNextSceneData(const boost::uuids::uuid& a_sceneUUID)
{
    // nilの場合hはPrefabSystemへ登録されないので
    // Map検索を行わず終了する
    if (a_sceneUUID.is_nil()) 
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneDataMapから削除できませんでした。");

        return false; 
    }

    const auto& l_itr = m_nextSceneDataMap.find(a_sceneUUID);

    if (l_itr == m_nextSceneDataMap.end()) { return false; }

    m_nextSceneDataMap.erase(l_itr);

    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "SceneUUID : {}\nのNextSceneDataを削除しました。", boost::uuids::to_string(a_sceneUUID));

    return true;
}
bool FWK::SceneChanger::ReplaceNextSceneFilePath(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const boost::uuids::uuid& a_sceneUUID)
{
    if (a_oldFilePath.empty() ||
        a_newFilePath.empty() ||
        a_sceneUUID.is_nil())
    {
        return false;
    }
 
    if (a_oldFilePath == a_newFilePath) { return true; }
 
    const auto& l_nextSceneDataITR = m_nextSceneDataMap.find(a_sceneUUID);
 
    if (l_nextSceneDataITR == m_nextSceneDataMap.end())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneChangerのNextSceneDataMapにSceneが存在しません。\nFilePath : {}", a_oldFilePath.string());
 
        return false;
    }
 
    auto& l_nextSceneData = l_nextSceneDataITR->second;

    if (l_nextSceneData.m_filePath != a_oldFilePath)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryとSceneChangerのNextSceneDataMapでSceneFilePathが一致していません。\nFilePath : {}", a_oldFilePath.string());
 
        return false;
    }
 
    l_nextSceneData.m_filePath = a_newFilePath;

    // シーン名はファイル名(stem)に追従させる
    l_nextSceneData.m_name = a_newFilePath.stem().string();
 
    return true;
}

const FWK::Struct::NextSceneData* FWK::SceneChanger::FetchPTRNexSceneData(const boost::uuids::uuid& a_sceneUUID) const
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneMapからNextSceneの取得に失敗しました。");

        return nullptr;
    }

    const auto& l_itr = m_nextSceneDataMap.find(a_sceneUUID);

    if (l_itr == m_nextSceneDataMap.end()) { return nullptr; }

    return &l_itr->second;
}