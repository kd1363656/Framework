#include "PrefabSystem.h"

void FWK::PrefabSystem::INIT()
{
	m_prefabMap.clear();
}
void FWK::PrefabSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
}

void FWK::PrefabSystem::AddPrefabMap(const std::filesystem::path& a_filePath, const Prefab& a_prefab)
{
	if (a_prefab.GetREFGameObject().expired())
	{
		FWK_ADD_LOG("Prefab用のGameObjectが設定されていないためPrefabSystemに追加できませんでした。");
		return;
	}

	m_prefabMap.try_emplace(a_filePath, a_prefab);
}

void FWK::PrefabSystem::RemovePrefab(const std::filesystem::path& a_filePath)
{
	auto l_itr = m_prefabMap.find(a_filePath);

	if (l_itr != m_prefabMap.end()) { return; }

	m_prefabMap.erase(l_itr);

	FWK_ADD_LOG("FilePath : {}\nPrefabを削除しました。", a_filePath.c_str());
}

nlohmann::json FWK::PrefabSystem::Serialize() const
{
	return nlohmann::json();
}