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

	const auto& l_prefabUUID             = l_gameObject->GetREFPrefabUUID            ();
	const auto  l_prefabSceneInstanceNUM = l_gameObject->GetVALPrefabSceneInstanceNUM();

	// PrefabUUIDとPrefabInstanceのNUMの両方を持つGameObjectだけを、
	// Prefabの代表GameObject候補として扱う
	if (l_prefabUUID.is_nil() ||
		l_prefabSceneInstanceNUM == Constant::k_invalidPrefabInstanceNUM)
	{
		return;
	}

	auto l_itr = m_prefabMap.find(l_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return; }

	auto& l_prefab = l_itr->second.m_prefab;
	
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

void FWK::PrefabSystem::AddPrefabMap(const boost::uuids::uuid& a_prefabUUID, const Struct::PrefabData& a_prefabData)
{
	if (a_prefabUUID.is_nil())
	{
		FWK_ADD_LOG("PrefabUUIDが無効だったため、PrefabSystemのプレハブマップに追加できませんでした。");

		return;
	}

	if (!m_prefabMap.try_emplace(a_prefabUUID, a_prefabData).second)
	{
		FWK_ADD_LOG("同じPrefabUUIDが既に登録されており、PrefabSystemのプレハブマップに追加できませんでした。");
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

	FWK_ADD_LOG("PrefabUUID : {}\nのプレハブを削除しました。", boost::uuids::to_string(a_prefabUUID));
}

nlohmann::json FWK::PrefabSystem::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::PrefabSceneInstanceNUM FWK::PrefabSystem::AllocatePrefabInstanceNUM(const boost::uuids::uuid& a_prefabUUID)
{
	if (a_prefabUUID.is_nil())
	{
		FWK_ADD_LOG("PrefabUUIDが無効のため、PrefabInstanceNUMを発行できませんでした。");

		return Constant::k_invalidPrefabInstanceNUM;
	}

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end())
	{
		FWK_ADD_LOG("PrefabUUID : {}\nのPrefabが登録されていないため、PrefabInstanceNUMを発行できませんでした。", boost::uuids::to_string(a_prefabUUID));

		return Constant::k_invalidPrefabInstanceNUM;
	}

	auto& l_prefabInstanceNUMAllocator = l_itr->second.m_prefabInstanceNUMAllocator;
		
	// 使用可能なInstanceNUMを発行
	return l_prefabInstanceNUMAllocator.Allocate();
}

void FWK::PrefabSystem::ReleasePrefabInstanceNUM(const boost::uuids::uuid& a_prefabUUID, const TypeAlias::PrefabSceneInstanceNUM a_prefabInstanceNUM)
{
	if (a_prefabUUID.is_nil() ||
		a_prefabInstanceNUM == Constant::k_invalidPrefabInstanceNUM)
	{
		return;
	}

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end())
	{
		FWK_ADD_LOG("PrefabUUID : {}\nのPrefabが登録されていないため、PrefabInstanceNUMを解放できませんでした。", boost::uuids::to_string(a_prefabUUID));

		return;
	}

	auto& l_prefabInstanceNUMAllocator = l_itr->second.m_prefabInstanceNUMAllocator;

	l_prefabInstanceNUMAllocator.Release(a_prefabInstanceNUM);
}

const FWK::Prefab* FWK::PrefabSystem::FindPTRPrefab(const boost::uuids::uuid& a_prefabUUID) const
{
	// NilUUIDからPrefabを検索することはできない
	if (a_prefabUUID.is_nil()) { return nullptr; }

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return nullptr; }

	return &l_itr->second.m_prefab;
}

FWK::Prefab* FWK::PrefabSystem::FindMutablePTRPrefab(const boost::uuids::uuid& a_prefabUUID)
{
	// NilUUIDからPrefabを検索することはできない
	if (a_prefabUUID.is_nil()) { return nullptr; }

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return nullptr; }

	return &l_itr->second.m_prefab;
}