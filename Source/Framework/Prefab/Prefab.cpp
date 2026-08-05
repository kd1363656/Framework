#include "Prefab.h"

void FWK::Prefab::LoadPrefab()
{
	// 読み込み時に前のJsonデータが残らないように初期化
	m_json = {};

	if (!Utility::CanLoadFilePath(m_filePath, Constant::k_lowerJsonExtension))
	{
		FWK_ADD_LOG("読み込みが不可能なファイルパスになっており、読み込み処理に失敗しました。");

		return; 
	}

	const auto& l_rootJson = Utility::LoadJsonFile(m_filePath);

	if (l_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効となっており、Prefabの読み込みに失敗しました。");

		return; 
	}

	m_jsonConverter.Deserialize(l_rootJson, *this);
}

void FWK::Prefab::SavePrefab()
{
	if (m_filePath.empty() ||
		m_filePath.extension() != Constant::k_lowerJsonExtension)
	{
		FWK_ADD_LOG("書き込みが不可能なファイルパスになっており、書き込み処理に失敗しました。");

		return;
	}

	// シリアライズしたプレハブを保持
	const auto& l_json = m_jsonConverter.Serialize(*this);

	if (l_json.is_null()) { return; }

	// 正常なJsonを生成できた場合だけ、
	// Prefab毎部のキャッシュを更新して保存する
	m_json = l_json;

	Utility::SaveJsonFile(m_json, m_filePath);
}