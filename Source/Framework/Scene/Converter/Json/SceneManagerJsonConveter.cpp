#include "SceneManagerJsonConveter.h"

void FWK::Converter::SceneManagerJsonConverter::Deserialize (const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const
{
	if (a_rootJson.is_null ()) { return; }

	// シーン遷移マップのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_nextSceneLoadFilePathMapJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeNextSceneLoadFilePathMap(l_json, a_sceneManager);
	}

	auto& l_sceneShiftEventObserver = a_sceneManager.GetMutableREFSceneShiftEventObserver();

	if (const auto& l_json = a_rootJson.value(k_sceneShiftEventObserverJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		l_sceneShiftEventObserver.Deserialize(l_json);
	}

	// シーンのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_sceneJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		const auto& l_scene = a_sceneManager.GetVALScene().lock();

		if (!l_scene) { return; }
		
		l_scene->Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::SceneManagerJsonConverter::Serialize(const SceneManager& a_sceneManager) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_scene                   = a_sceneManager.GetVALScene                  ().lock();
	const auto& l_sceneShiftEventObserver = a_sceneManager.GetREFSceneShiftEventObserver();

	if (!l_scene) { return {}; }

	// シーン遷移マップのシリアライズ
	l_rootJson[k_nextSceneLoadFilePathMapJsonKey] = SerializeNextSceneLoadFilePathMap(a_sceneManager);

	// シーンオブザーバーのデシリアライズ
	l_rootJson[k_sceneShiftEventObserverJsonKey] = l_sceneShiftEventObserver.Serialize();

	// シーンのシリアライズ
	l_rootJson[k_sceneJsonKey] = l_scene->Serialize();

	return l_rootJson;
}

void FWK::Converter::SceneManagerJsonConverter::DeserializeNextSceneLoadFilePathMap(const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const
{
	if (a_rootJson.is_null())			   { return; }
	if (!Utility::IsJsonArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{ 
		const auto& l_sceneUUID = Utility::DeserializeUUID(l_json, k_sceneUUIDJsonKey);

		if (l_sceneUUID.is_nil()) { continue; }

		const auto& l_nextSceneLoadFilePath = l_json.value(k_nextSceneLoadFilePathJsonKey, std::filesystem::path());

		a_sceneManager.AddNextSceneLoadFilePath(l_sceneUUID, l_nextSceneLoadFilePath);
	}
}

nlohmann::json FWK::Converter::SceneManagerJsonConverter::SerializeNextSceneLoadFilePathMap(const SceneManager & a_sceneManager) const
{
	      auto  l_rootJsonArray            = nlohmann::json::array				          ();
	const auto& l_nextSceneLoadFilePathMap = a_sceneManager.GetREFNextSceneLoadFilePathMap();

	for (const auto& [l_sceneUUID, l_nextSceneLoadFilePath] : l_nextSceneLoadFilePathMap)
	{
		if (l_sceneUUID.is_nil()) { continue; }

		nlohmann::json l_json = {};

		Utility::UpdateJson(l_json, Utility::SerializeUUID(l_sceneUUID, k_sceneUUIDJsonKey));

		l_json[k_nextSceneLoadFilePathJsonKey] = l_nextSceneLoadFilePath;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}