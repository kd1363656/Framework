#include "SceneJsonConveter.h"

void FWK::Converter::SceneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (!a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_gameObjectListJsonKey, nlohmann::json{});
		l_json.is_null())
	{
		DeserializeGameObjectList(a_rootJson, a_scene);
	}
}
nlohmann::json FWK::Converter::SceneJsonConverter::Serialize(const Scene& a_scene) const
{
	auto l_rootJson = nlohmann::json{};

	l_rootJson[k_gameObjectListJsonKey] = SerializeGameObjectList(a_scene);

	return l_rootJson;
}

void FWK::Converter::SceneJsonConverter::DeserializeGameObjectList(const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (a_rootJson.is_null()) { return; }

	
}

nlohmann::json FWK::Converter::SceneJsonConverter::SerializeGameObjectList(const Scene& a_scene) const
{
		  auto  l_rootJsonArray  = nlohmann::json::array	   ();
	const auto& l_gameObjectList = a_scene.GetREFGameObjectList();
	
	for (const auto& l_gameObject : l_gameObjectList)
	{
		if (!l_gameObject.m_gameObject ||
			!l_gameObject.m_gameObjectAddress) 
		{
			continue; 
		}

		nlohmann::json l_json = {};
	}

	return l_rootJsonArray;
}