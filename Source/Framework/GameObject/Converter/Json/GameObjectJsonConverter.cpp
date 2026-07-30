#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのデシリアライズに失敗しました。");
		return; 
	}
}

nlohmann::json FWK::GameObjectJsonConverter::Serialize(const GameObject & a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializePrefab(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}