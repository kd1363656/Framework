#include "PrefabJsonConverter.h"

void FWK::Converter::PrefabJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Prefab& a_prefab) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効となっており、Prefabのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_filePath   = a_rootJson.value(k_filePathJsonKey,   std::filesystem::path{});
	const auto& l_prefabName = a_rootJson.value(k_prefabNameJsonKey, std::string{});
	const auto& l_prefabType = a_rootJson.value(k_prefabTypeJsonKey, Enum::PrefabType::GameObject);

	// PrefabSystem側に保存されているFilePathが無効なら
	// どのPrefabファイルを読み込むべきか判断できない
	if (!Utility::CanLoadFilePath(l_filePath))
	{
		FWK_ADD_LOG("PrefabのFilePathが空となっており、Prefabのデシリアライズに失敗しました。");

		return;
	}

	// PrefabNameは、このPrefabから生成される
	// GameObject名の元になるための必須情報
	if (l_prefabName.empty())
	{
		FWK_ADD_LOG("PrefabNameが空となっており、Prefabのデシリアライズに失敗しました。");

		return;
	}

	a_prefab.SetFilePath  (l_filePath);
	a_prefab.SetPrefabName(l_prefabName);
	a_prefab.SetPrefabType(l_prefabType);

	// 実際のプレハブ読み込みはデシリアライズで行う(デシリアライズで取得したファイルパスを元に行うものだから)
	LoadGameObjectPrefab(a_prefab);
}

nlohmann::json FWK::Converter::PrefabJsonConverter::Serialize(Prefab& a_prefab) const
{
	const auto& l_filePath   = a_prefab.GetREFFilePath  ();
	
	if (l_filePath.empty() ||
		l_filePath.extension() != Constant::k_lowerJsonExtension)
	{
		FWK_ADD_LOG("書き込みが不可能なファイルパスになっており、書き込み処理に失敗しました。");

		return {};
	}

	if (!SaveGameObjectPrefab(a_prefab)) { return {}; }

	nlohmann::json l_rootJson = {};

	l_rootJson[k_filePathJsonKey]   = l_filePath;
	l_rootJson[k_prefabNameJsonKey] = a_prefab.GetREFPrefabName();
	l_rootJson[k_prefabTypeJsonKey] = a_prefab.GetVALPrefabType();

	return l_rootJson;
}

void FWK::Converter::PrefabJsonConverter::LoadGameObjectPrefab(Prefab& a_prefab) const
{
	const auto& l_filePath = a_prefab.GetREFFilePath();

	// Jsonでデシリアライズしたファイルパスからjsonファイルをロードして保持する
	if (!Utility::CanLoadFilePath(l_filePath, Constant::k_lowerJsonExtension))
	{
		FWK_ADD_LOG("読み込みが不可能なファイルパスになっており、読み込み処理に失敗しました。");

		return;
	}

	const auto& l_rootJson = Utility::LoadJsonFile(l_filePath);

	if (l_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効となっており、Prefabの読み込みに失敗しました。");

		return;
	}

	const auto& l_json = l_rootJson.value(k_prefabJsonKey, nlohmann::json{});

	if (l_json.is_null()) { return; }

	a_prefab.SetJson(l_json);
}

bool FWK::Converter::PrefabJsonConverter::SaveGameObjectPrefab(Prefab& a_prefab) const
{
	const auto& l_gameObject = a_prefab.GetREFGameObject().lock();

	if (!l_gameObject)
	{
		FWK_ADD_LOG("Prefab保存用GameObjectが存在しないため、Prefabファイルを保存しませんでした。\nFilePath : {}", a_prefab.GetREFFilePath().string());

		return false;
	}

	const auto& l_filePath       = a_prefab.GetREFFilePath      ();
	const auto& l_gameObjectJson = l_gameObject->SerializePrefab();

	// GameObjectJsonを生成できなかった場合は、
	// 不完全なPrefabファイルを書き込まない。
	if (l_gameObjectJson.is_null())
	{
		FWK_ADD_LOG("GameObjectのPrefabJsonを生成できなかったため、Prefabファイルを保存できませんでした。");

		return false;
	}

	nlohmann::json l_rootJson = {};

	// 実際のPrefabファイルには、
	// GameObjectを復元するための情報を保存する
	l_rootJson[k_prefabJsonKey] = l_gameObjectJson;

	// ファイル書き込みに失敗した場合は
	// Prefab内部のキャッシュも更新しない
	if (!Utility::SaveJsonFile(l_rootJson, l_filePath))
	{
		FWK_ADD_LOG("PrefabファイルへのJson書き込みに失敗しました。\nFilePath : {}", l_filePath.string());

		return false;
	}

	// m_jsonは常にGameObjectのプレハブ部分だけ保存する
	a_prefab.SetJson(l_gameObjectJson);
	
	return true;
}