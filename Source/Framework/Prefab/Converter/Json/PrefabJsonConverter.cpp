#include "PrefabJsonConverter.h"

void FWK::Converter::PrefabJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Prefab& a_prefab)
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効となっており、Prefabのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_json = a_rootJson.value(k_gameObjectJsonKey, nlohmann::json{});

	if (l_json.is_null()) { return; }

	a_prefab.SetJson(l_json);
}

nlohmann::json FWK::Converter::PrefabJsonConverter::Serialize(const Prefab& a_prefab) const
{
	const auto&          l_gameObject = a_prefab.GetREFGameObject().lock();
	      nlohmann::json l_rootJson   = {};

	if (!l_gameObject) 
	{
		FWK_ADD_LOG("Prefab保存用GameObjectが存在しないため、Prefabファイルを保存しませんでした。\nFilePath : {}", a_prefab.GetREFFilePath());

		return {}; 
	}

	l_rootJson[k_gameObjectJsonKey] = l_gameObject->SerializePrefab();

	return l_rootJson;
}