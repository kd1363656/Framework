#include "SceneChanger.h"

void FWK::SceneChanger::INIT()
{
    m_nextSceneMap.clear();

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

bool FWK::SceneChanger::AddNextScene(const boost::uuids::uuid& a_sceneUUID, const NextScene& a_nextScene)
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneMapに追加できませんでした。");

        return false;
    }

    if (!m_nextSceneMap.try_emplace(a_sceneUUID, a_nextScene).second)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "同じSceneUUIDが既に登録されており、SceneChangerのNextSceneMapに追加できませんでした。");
    }

    return true;
}
bool FWK::SceneChanger::RemoveNextScene(const boost::uuids::uuid& a_sceneUUID)
{
    // nilの場合hはPrefabSystemへ登録されないので
    // Map検索を行わず終了する
    if (a_sceneUUID.is_nil()) 
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneMapから削除できませんでした。");

        return false; 
    }

    const auto& l_itr = m_nextSceneMap.find(a_sceneUUID);

    if (l_itr == m_nextSceneMap.end()) { return false; }

    m_nextSceneMap.erase(l_itr);

    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "SceneUUID : {}\nのNextSceneを削除しました。", boost::uuids::to_string(a_sceneUUID));

    return true;
}

const FWK::NextScene* FWK::SceneChanger::FetchPTRNexScene(const boost::uuids::uuid& a_sceneUUID) const
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneMapからNextSceneの取得に失敗しました。");

        return nullptr;
    }

    const auto& l_itr = m_nextSceneMap.find(a_sceneUUID);

    if (l_itr == m_nextSceneMap.end()) { return nullptr; }

    return &l_itr->second;
}

FWK::NextScene* FWK::SceneChanger::FetchMutablePTRNextScene(const boost::uuids::uuid& a_sceneUUID)
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDが無効だったため、SceneChangerのNextSceneMapからNextSceneの取得に失敗しました。");

        return nullptr;
    }

    const auto& l_itr = m_nextSceneMap.find(a_sceneUUID);

    if (l_itr == m_nextSceneMap.end()) { return nullptr; }

    return &l_itr->second;
}