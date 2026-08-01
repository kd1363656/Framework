#include "SceneJsonConveter.h"

void FWK::Converter::SceneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_prefabSystemJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem();

		l_prefabSystem.Deserialize(l_json);
	}

	if (const auto& l_json = a_rootJson.value(k_gameObjectListJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeGameObjectList(l_json, a_scene);
	}
}
nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(const Scene& a_scene) const
{
	      nlohmann::json l_rootJson     = {};
	const auto&          l_prefabSystem = a_scene.GetREFPrefabSystem();

	l_rootJson[k_prefabSystemJsonKey]   = l_prefabSystem.Serialize();
	l_rootJson[k_gameObjectListJsonKey] = SerializeGameObjectList (a_scene);

	return l_rootJson;
}

void FWK::Converter::SceneJsonConverter::DeserializeGameObjectList(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (a_rootJson.is_null())		       { return; }
	if (!Utility::IsJsonArray(a_rootJson)) { return; }
	
	for (const auto& l_json : a_rootJson)
	{
		const auto& l_gameObjectJson = l_json.value(k_gameObjecJsonKey, nlohmann::json{});

		if (l_gameObjectJson.is_null()) { continue; }

		auto l_gameObject = std::make_shared<GameObject>();

		// 子ゲームオブジェクトなどをシリアライズしてシーンに登録
		l_gameObject->Deserialize(l_gameObjectJson, a_scene);

		// シーンに親ゲームオブジェクトを追加
		a_scene.AddGameObject(l_gameObject);
	}
}

nlohmann::json FWK::Converter::SceneJsonConverter::SerializeGameObjectList(const Scene& a_scene) const
{
		  auto  l_rootJsonArray  = nlohmann::json::array	   ();
	const auto& l_gameObjectList = a_scene.GetREFGameObjectList();
	
	for (const auto& l_gameObject : l_gameObjectList)
	{
		// プレハブ名がないなら不正なゲームオブジェクトとしてシリアライズしない
		if (!l_gameObject ||
			l_gameObject->GetREFPrefabName().empty())
		{
			continue; 
		}

		nlohmann::json l_json = {};

		l_json[k_gameObjecJsonKey] = l_gameObject->SerializeScene();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}