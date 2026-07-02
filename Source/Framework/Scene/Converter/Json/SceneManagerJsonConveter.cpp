#include "SceneManagerJsonConveter.h"

void FWK::Converter::SceneManagerJsonConverter::Deserialize (const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const
{
	if (a_rootJson.is_null ()) { return; }

	// シーン遷移マップのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_sceneShiftMapJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeSceneShiftMap(l_json, a_sceneManager);
	}

	// シーンのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_sceneJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_scene = a_sceneManager.GetMutableREFScene();

		l_scene.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::SceneManagerJsonConverter::Serialize(const SceneManager& a_sceneManager) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_scene = a_sceneManager.GetREFScene();

	// シーン遷移マップのシリアライズ
	l_rootJson[k_sceneShiftMapJsonKey] = SerializeSceneShiftMap(a_sceneManager);

	// シーンのシリアライズ
	l_rootJson[k_sceneJsonKey] = l_scene.Serialize();

	return l_rootJson;
}

void FWK::Converter::SceneManagerJsonConverter::DeserializeSceneShiftMap(const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const
{
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		const auto& l_sceneName     = l_json.value(k_sceneNameJsonKey,     std::string{});
		const auto& l_sceneFilePath = l_json.value(k_sceneFilePathJsonKey, std::string{});

		a_sceneManager.AddSceneShiftMap(l_sceneName, l_sceneFilePath);
	}
}

nlohmann::json FWK::Converter::SceneManagerJsonConverter::SerializeSceneShiftMap(const SceneManager& a_sceneManager) const
{
		  auto  l_rootJsonArray = nlohmann::json::array				();
	const auto& l_sceneShiftMap = a_sceneManager.GetREFSceneShiftMap();

	for (const auto& [l_sceneName, l_sceneFilePath] : l_sceneShiftMap)
	{
		nlohmann::json l_json = {};

		l_json[k_sceneNameJsonKey]     = l_sceneName;
		l_json[k_sceneFilePathJsonKey] = l_sceneFilePath;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}