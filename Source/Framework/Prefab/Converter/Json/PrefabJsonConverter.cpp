#include "PrefabJsonConverter.h"

void FWK::Converter::PrefabJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Prefab& a_prefab) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効となっており、Prefabのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_filePath = a_rootJson.value(k_filePathJsonKey, nlohmann::json{});

	a_prefab.SetFilePath(l_filePath);

	LoadGameObjectPrefab(a_prefab);
}

nlohmann::json FWK::Converter::PrefabJsonConverter::Serialize(const Prefab& a_prefab) const
{
	const auto& l_filePath   = a_prefab.GetREFFilePath  ();
	
	if (l_filePath.empty() ||
		l_filePath.extension() != Constant::k_lowerJsonExtension)
	{
		FWK_ADD_LOG("書き込みが不可能なファイルパスになっており、書き込み処理に失敗しました。");

		return {};
	}

	nlohmann::json l_rootJson = {};

	l_rootJson[k_filePathJsonKey] = l_filePath.string();

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

	const auto& l_json = l_rootJson.value(k_gameObjectJsonKey, nlohmann::json{});

	if (l_json.is_null()) { return; }

	a_prefab.SetJson(l_json);
}

void FWK::Converter::PrefabJsonConverter::SaveGameObjectPrefab(Prefab& a_prefab) const
{
	      nlohmann::json l_rootJson   = {};
	const auto&          l_gameObject = a_prefab.GetREFGameObject().lock();

	if (!l_gameObject)
	{
		FWK_ADD_LOG("Prefab保存用GameObjectが存在しないため、Prefabファイルを保存しませんでした。\nFilePath : {}", a_prefab.GetREFFilePath().string());

		return;
	}

	const auto& l_filePath = a_prefab.GetREFFilePath();

	l_rootJson[k_gameObjectJsonKey] = l_gameObject->SerializePrefab();

	if (l_rootJson.is_null()) { return; }

	// 正常なJsonを生成できた場合だけ、
	// Prefab毎部のキャッシュを更新して保存する
	a_prefab.SetJson(l_rootJson);

	Utility::SaveJsonFile(l_rootJson, l_filePath);
}