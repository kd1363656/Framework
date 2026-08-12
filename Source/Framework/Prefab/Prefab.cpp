#include "Prefab.h"

void FWK::Prefab::Deserialize(const nlohmann::json& a_rootJson)
{
	// 読み込み時に前のJsonデータが残らないように初期化
	m_json = {};

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Prefab::Serialize()
{
	// シリアライズしたプレハブを保持
	const auto& l_json = m_jsonConverter.Serialize(*this);

	return l_json;
}