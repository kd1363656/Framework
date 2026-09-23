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

void FWK::PrefabSystem::CachePrefabGameObjectIfNeeded(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return;
    }

    const auto& l_prefabUUID = l_gameObject->GetREFPrefabUUID();
    
    auto l_itr = m_prefabMap.find(l_prefabUUID);

    if (l_itr == m_prefabMap.end()) { return; }

    auto& l_prefab = l_itr->second;

    // 既に有効な代表GameObjectが存在しており、
    // 同じPrefabUUIDを参照している場合は
    // 現在の代表GameObjectをそのまま維持する
    if (const auto& l_cachedGameObject = l_prefab.GetREFGameObject().lock();
        l_cachedGameObject                       &&
        !l_cachedGameObject->GetVALIsDestroyed() &&
        Utility::IsSamePrefab(*l_cachedGameObject, *l_gameObject))
    {
        return;
    }

    // キャッシュが空、削除予定、または異なるPrefabを示していた場合に
    // 新しい代表GameObjectへ差し替える
    l_prefab.SetGameObject(l_gameObject);
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