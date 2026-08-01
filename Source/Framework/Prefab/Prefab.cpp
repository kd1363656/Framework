#include "Prefab.h"

void FWK::Prefab::LoadPrefab()
{
	// 読み込み時に前のJsonデータが残らないように初期化
	m_json = {};

	const auto& l_rootJson = Utility::LoadJsonFile(m_filePath);

	if (l_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効となっており、Prefabの読み込みに失敗しました。");

		return; 
	}

	m_jsonConverter.Deserialize(l_rootJson, *this);
}

void FWK::Prefab::SavePrefab() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, m_filePath);
}