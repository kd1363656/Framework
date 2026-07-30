#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのデシリアライズに失敗しました。");
		return; 
	}

	auto& l_scene        = SceneManager::GetInstance        ().GetMutableREFScene();
	auto& l_prefabSystem = l_scene.GetMutableREFPrefabSystem();

	// ゲームオブジェクトのプレハブ名を読み取る
	const std::string l_prefabName = a_rootJson.value(k_prefabNameJsonKey, std::string{});

	if (l_prefabName.empty()) 
	{
		FWK_ADD_LOG("名前が空のプレハブがJsonファイルに含まれています。SceneのJsonファイルを確認してください。");

		return;
	}



}
void FWK::GameObjectJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, std::vector<std::shared_ptr<ComponentBase>>& a_componentList, std::vector<Struct::ChildDeserializeData>& a_childDeserializeData) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");
		return;
	}

}
void FWK::GameObjectJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, std::vector<std::shared_ptr<ComponentBase>>& a_componentList, std::vector<Struct::ChildDeserializeData>& a_childDeserializeData) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");
		return;
	}

}

nlohmann::json FWK::GameObjectJsonConverter::SerializeScene(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializePrefab(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}