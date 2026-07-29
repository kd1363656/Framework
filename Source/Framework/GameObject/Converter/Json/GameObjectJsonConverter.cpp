#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("Jsonが無効のため、ゲームオブジェクトの読み込みに失敗しました。");
		return; 
	}
}