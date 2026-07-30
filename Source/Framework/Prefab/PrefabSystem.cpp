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

void FWK::PrefabSystem::AddPrefabMap(const std::string& a_prefabName, const Prefab& a_prefab)
{
	if (a_prefab.GetREFGameObject().expired())
	{
		FWK_ADD_LOG("Prefab用のGameObjectが設定されていないためPrefabSystemに追加できませんでした。");
		return;
	}

	m_prefabMap.try_emplace(a_prefabName, a_prefab);
}

void FWK::PrefabSystem::RemovePrefab(const std::string& a_prefabName)
{
	auto l_itr = m_prefabMap.find(a_prefabName);

	if (l_itr != m_prefabMap.end()) { return; }

	m_prefabMap.erase(l_itr);

	FWK_ADD_LOG("PrefabName : {}\nPrefabを削除しました。", a_prefabName);
}

nlohmann::json FWK::PrefabSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}