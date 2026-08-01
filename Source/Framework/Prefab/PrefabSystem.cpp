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

void FWK::PrefabSystem::AddPrefabMap(const std::string& a_prefabName, const Struct::PrefabData& a_prefabData)
{
	m_prefabMap.try_emplace(a_prefabName, a_prefabData);
}

void FWK::PrefabSystem::RemovePrefab(const std::string& a_prefabName)
{
	auto l_itr = m_prefabMap.find(a_prefabName);

	if (l_itr == m_prefabMap.end()) { return; }

	m_prefabMap.erase(l_itr);

	FWK_ADD_LOG("PrefabName : {}\nPrefabを削除しました。", a_prefabName);
}

nlohmann::json FWK::PrefabSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

const FWK::Prefab* FWK::PrefabSystem::FindPTRPrefab(const std::string& a_prefabName) const
{
	auto l_itr = m_prefabMap.find(a_prefabName);

	if (l_itr == m_prefabMap.end()) { return nullptr; }

	return &l_itr->second.m_prefab;
}