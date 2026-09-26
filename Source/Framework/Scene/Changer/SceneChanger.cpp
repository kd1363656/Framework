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

    return false;
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