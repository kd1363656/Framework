#include "PrefabJsonConverter.h"

void FWK::Converter::PrefabJsonConverter::Load(const nlohmann::json& a_rootJson, Prefab& a_prefab) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "RootJsonが無効となっており、Prefabのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_prefabName = a_rootJson.value(k_prefabNameJsonKey, std::string{});

	// PrefabNameは、このPrefabから生成される
	// GameObject名の元になるための必須情報
	if (l_prefabName.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabNameが空となっており、Prefabのデシリアライズに失敗しました。");

		return;
	}

	a_prefab.SetPrefabName(l_prefabName);

	const auto& l_json = a_rootJson.value(k_prefabJsonKey, nlohmann::json{});

	if (l_json.is_null()) { return; }

	a_prefab.SetJson(l_json);
}

bool FWK::Converter::PrefabJsonConverter::Save(const std::filesystem::path& a_filePath, Prefab& a_prefab) const
{
	// 読み込めるファイルでなければ保存しない
	if (a_filePath.empty() ||
		a_filePath.extension() != Constant::k_lowerJsonExtension)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "書き込みが不可能なファイルパスになっており、書き込み処理に失敗しました。");

		return false;
	}

	const auto& l_gameObject = a_prefab.GetREFGameObject().lock();

	if (!l_gameObject)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab保存用GameObjectが存在しないため、Prefabファイルを保存しませんでした。\nFilePath : {}", a_filePath.string());

		return false;
	}

	if (l_gameObject->GetREFPrefabUUID().is_nil() ||
		l_gameObject->GetVALPrefabSceneInstanceNUM() == Constant::k_invalidPrefabSceneInstanceNUM)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab保存用GameObjectのPrefab情報が無効なため、Prefabファイルを保存しませんでした。\nFilePath : {}", a_filePath.string());

		return false;
	}

	const auto& l_gameObjectJson = l_gameObject->SerializePrefab();
	      
	// GameObjectJsonを生成できなかった場合は、
	// 不完全なPrefabファイルを書き込まない。
	if (l_gameObjectJson.is_null())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "GameObjectのPrefabJsonを生成できなかったため、Prefabファイルを保存できませんでした。");

		return false; 
	}

	nlohmann::json l_rootJson = {};

	l_rootJson[k_prefabNameJsonKey] = a_prefab.GetREFPrefabName();

	// 実際のPrefabファイルには、
	// GameObjectを復元するための情報を保存する
	l_rootJson[k_prefabJsonKey] = l_gameObjectJson;

	// ファイル書き込みに失敗した場合は
	// Prefab内部のキャッシュも更新しない
	if (!Utility::SaveJsonFile(l_rootJson, a_filePath))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabファイルへのJson書き込みに失敗しました。\nFilePath : {}", a_filePath.string());

		return false;
	}

	// m_jsonは常にGameObjectのプレハブ部分だけ保存する
	a_prefab.SetJson(l_gameObjectJson);

	return true;
}