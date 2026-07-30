#include "PrefabJsonConverter.h"

nlohmann::json FWK::Converter::PrefabJsonConverter::Serialize(const Prefab& a_prefab) const
{
	const auto& l_gameObject = a_prefab.GetREFGameObject().lock();

	if (!l_gameObject) 
	{
		FWK_ADD_LOG("PrefabにPrefab用のGameObjetがセットされておらずシリアライズに失敗しました。");
		return {}; 
	}

	return l_gameObject->SerializePrefab();
}