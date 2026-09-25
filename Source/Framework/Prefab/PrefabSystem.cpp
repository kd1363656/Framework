#include "PrefabSystem.h"

void FWK::PrefabSystem::INIT()
{
    m_prefabMap.clear();
}
void FWK::PrefabSystem::Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this, a_assetFilePathRegistry);
}

void FWK::PrefabSystem::AddPrefab(const boost::uuids::uuid& a_prefabUUID, const Prefab& a_prefab)
{
    if (a_prefabUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUIDが無効だったため、PrefabSystemのプレハブマップに追加できませんでした。");

        return;
    }

    if (!m_prefabMap.try_emplace(a_prefabUUID, a_prefab).second)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "同じPrefabUUIDが既に登録されており、PrefabSystemのプレハブマップに追加できませんでした。");
    }
}
void FWK::PrefabSystem::RemovePrefab(const boost::uuids::uuid& a_prefabUUID)
{
    // NilUUIDはPrefabSystemへ登録されないので
    // Map検索を行わず終了する
    if (a_prefabUUID.is_nil()) { return; }

    auto l_itr = m_prefabMap.find(a_prefabUUID);

    if (l_itr == m_prefabMap.end()) { return; }

    m_prefabMap.erase(l_itr);

    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "PrefabUUID : {}\nのプレハブを削除しました。", boost::uuids::to_string(a_prefabUUID));
}

nlohmann::json FWK::PrefabSystem::Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry)
{
    return m_jsonConverter.Serialize(a_assetFilePathRegistry, *this);
}

const FWK::Prefab* FWK::PrefabSystem::FindPTRPrefab(const boost::uuids::uuid& a_prefabUUID) const
{
    // NilUUIDからPrefabを検索することはできない
    if (a_prefabUUID.is_nil()) { return nullptr; }

    auto l_itr = m_prefabMap.find(a_prefabUUID);

    if (l_itr == m_prefabMap.end()) { return nullptr; }

    return &l_itr->second;
}

FWK::Prefab* FWK::PrefabSystem::FindMutablePTRPrefab(const boost::uuids::uuid& a_prefabUUID)
{
    // NilUUIDからPrefabを検索することはできない
    if (a_prefabUUID.is_nil()) { return nullptr; }

    auto l_itr = m_prefabMap.find(a_prefabUUID);

    if (l_itr == m_prefabMap.end()) { return nullptr; }

    return &l_itr->second;
}