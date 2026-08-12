#include "PrefabSystem.h"

void FWK::PrefabSystem::INIT()
{
	m_prefabMap.clear();
}
void FWK::PrefabSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::PrefabSystem::CachePrefabGameObjectIfNeeded(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed()) 
	{
		return; 
	}

	const auto& l_prefabName = l_gameObject->GetREFPrefabName();

	if (l_prefabName.empty() ||
		l_gameObject->GetVALPrefabInstanceNUM() == Constant::k_invalidPrefabInstanceNUM)
	{
		return;
	}

	auto l_itr = m_prefabMap.find(l_prefabName);

	if (l_itr == m_prefabMap.end()) { return; }

	      auto& l_prefab           = l_itr->second.m_prefab;
	const auto& l_cachedGameObject = l_prefab.GetREFGameObject().lock();

	// 現在のキャッシュが有効なら変更しない
	if (l_cachedGameObject                       &&
		!l_cachedGameObject->GetVALIsDestroyed() &&
		l_cachedGameObject->GetREFPrefabName() == l_prefabName)
	{
		return;
	}

	// キャッシュが空、削除予定、または異なるPrefabを示していた場合に
	// 新しい代表GameObjectへ差し替える
	l_prefab.SetGameObject(l_gameObject);
}

void FWK::PrefabSystem::AddPrefabMap(const TypeAlias::UUID& a_prefabUUID, const Struct::PrefabData& a_prefabData)
{
	if (a_prefabUUID == GUID_NULL)
	{
		FWK_ADD_LOG("PrefabUUIDが無効だったため、PrefabSystemのプレハブマップに追加できませんでした。");

		return;
	}

	if (m_prefabMap.try_emplace(a_prefabUUID, a_prefabData).second)
	{
		FWK_ADD_LOG("同じPrefabUUIDが既に登録されており、PrefabSystemのプレハブマップに追加できませんでした。");
	}
}
void FWK::PrefabSystem::RemovePrefab(const UUID& a_prefabUUID)
{
	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return; }

	m_prefabMap.erase(l_itr);

	FWK_ADD_LOG("PrefabUUID : {}\nPrefabを削除しました。", Utility::UUIDToString(a_prefabUUID));
}

nlohmann::json FWK::PrefabSystem::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::PrefabInstanceNUM FWK::PrefabSystem::AllocatePrefabInstanceNUM(const UUID& a_prefabUUID)
{
	if (a_prefabUUID == GUID_NULL)
	{
		FWK_ADD_LOG("PrefabNameが空のため、PrefabInstanceNUMを発行できませんでした。");

		return Constant::k_invalidPrefabInstanceNUM;
	}

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end())
	{
		FWK_ADD_LOG("PrefabName : {}\nPrefabが登録されていないため、PrefabInstanceNU<を発行できませんでした。");

		return Constant::k_invalidPrefabInstanceNUM;
	}

	auto& l_prefabInstanceNUMAllocator = l_itr->second.m_prefabInstanceNUMAllocator;
		
	// 使用可能なInstanceNUMを発行
	return l_prefabInstanceNUMAllocator.Allocate();
}

void FWK::PrefabSystem::ReleasePrefabInstanceNUM(const UUID& a_prefabUUID, const TypeAlias::PrefabInstanceNUM a_prefabInstanceNUM)
{
	if (a_prefabUUID == GUID_NULL ||
		a_prefabInstanceNUM == Constant::k_invalidPrefabInstanceNUM)
	{
		return;
	}

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end())
	{
		FWK_ADD_LOG("PrefabUUID : {}\nPrefabが登録されていないため、PrefabInstanceNUMを解放できませんでした。", Utility::UUIDToString(a_prefabUUID));

		return;
	}

	auto& l_prefabInstanceNUMAllocator = l_itr->second.m_prefabInstanceNUMAllocator;

	l_prefabInstanceNUMAllocator.Release(a_prefabInstanceNUM);
}

const FWK::Prefab* FWK::PrefabSystem::FindPTRPrefab(const UUID& a_prefabUUID) const
{
	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return nullptr; }

	return &l_itr->second.m_prefab;
}