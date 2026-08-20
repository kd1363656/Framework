#include "Prefab.h"

void FWK::Prefab::LoadGameObjectPrefab()
{
	m_jsonConverter.LoadGameObjectPrefab(*this);
}

void FWK::Prefab::Deserialize(const nlohmann::json& a_rootJson)
{
	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Prefab::Serialize()
{
	// シリアライズしたプレハブを保持
	return m_jsonConverter.Serialize(*this);
}