#include "SceneJsonConveter.h"

void FWK::Converter::SceneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem();

	// プレハブシステムのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_prefabSystemJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		l_prefabSystem.Deserialize(l_json);
	}

	// ゲームオブジェクトリストのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_gameObjectListJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeGameObjectList(l_json, a_scene);
	}
}
nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(Scene& a_scene) const
{
	nlohmann::json l_rootJson     = {};
	auto&          l_prefabSystem = a_scene.GetMutableREFPrefabSystem();

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

		// Prefab情報を生成した後
		// Scene固有情報としてPrefabInstanceNUMなどを復元する
		// 子ゲームオブジェクトなどをシリアライズしてシーンに登録
		l_gameObject->Deserialize(l_gameObjectJson, a_scene);

	    // PrefabInstanceNUMがDeserializeで有効値にならず無効値のままなら
		// Sceneへ登録しない
		if (l_gameObject->GetREFPrefabName().empty() ||
			l_gameObject->GetVALPrefabInstanceNUM() == Constant::k_invalidPrefabInstanceNUM)
		{
			FWK_ADD_LOG("PrefabNameまたはPrefabInstaneNUMが無効のため、GameObjectをSceneへ追加できませんでした。");

			continue;
		}

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
			l_gameObject->GetVALIsDestroyed())
		{
			continue; 
		}

		// 子GameObjectは親GaameObjectのChildListへ保存されているため
		// Scenec直下にRootGameObjectだけを保存する
		if (!l_gameObject->GetREFParent().expired()) { continue; }

		// プレハブ名が空、プレハブインスタンスうナンバーが無効値ならシリアライズ処理を行わない
		if (l_gameObject->GetREFPrefabName().empty() ||
			l_gameObject->GetVALPrefabInstanceNUM() == Constant::k_invalidPrefabInstanceNUM)
		{
			continue;
		}

		nlohmann::json l_json = {};

		l_json[k_gameObjecJsonKey] = l_gameObject->SerializeScene();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}