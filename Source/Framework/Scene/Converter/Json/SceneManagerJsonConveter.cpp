#include "SceneManagerJsonConveter.h"

void FWK::Converter::SceneManagerJsonConverter::Load(SceneManager& a_sceneManager) const
{
	const auto& l_currentSceneFilePath = a_sceneManager.GetREFCurrentSceneFilePath();

	if (!Utility::CanLoadFilePath(l_currentSceneFilePath))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "読み込めるファイルパスでないため、シーンの読み込みに失敗しました。");

		return;
	}

	const auto& l_rootJson = Utility::LoadJsonFile(l_currentSceneFilePath);

	if (l_rootJson.is_null ()) { return; }

	auto& l_assetFilePathRegistry = a_sceneManager.GetMutableREFAssetFilePathRegistry();

	// アセットファイルパスレジストリーのデシリアライズ
	if (const auto& l_json = l_rootJson.value(k_assetFilePathRegistryJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		l_assetFilePathRegistry.Deserialize(l_json);
	}

	// シーン遷移マップのデシリアライズ
	if (const auto& l_json = l_rootJson.value(k_nextSceneLoadFilePathMapJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeNextSceneLoadFilePathMap(l_json, a_sceneManager);
	}

	if (const auto& l_json = l_rootJson.value(k_sceneShiftEventObserverJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_sceneShiftEventObserver = a_sceneManager.GetMutableREFSceneShiftEventObserver();

		l_sceneShiftEventObserver.Deserialize(l_json);
	}

	// シーンのデシリアライズ
	if (const auto& l_json = l_rootJson.value(k_sceneJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		const auto& l_scene = a_sceneManager.GetVALScene().lock();

		if (!l_scene) { return; }
		
		l_scene->Deserialize(l_json, l_assetFilePathRegistry);
	}
}

void FWK::Converter::SceneManagerJsonConverter::Save(const SceneManager& a_sceneManager) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_assetFilePathRegistry   = a_sceneManager.GetREFAssetFilePathRegistry  ();
	const auto& l_scene                   = a_sceneManager.GetVALScene                  ().lock();
	const auto& l_sceneShiftEventObserver = a_sceneManager.GetREFSceneShiftEventObserver();
	const auto& l_currentSceneFilePath    = a_sceneManager.GetREFCurrentSceneFilePath   ();

	if (!l_scene ||
		l_currentSceneFilePath.empty())
	{
		return; 
	}

	l_rootJson[k_assetFilePathRegistryJsonKey] = l_assetFilePathRegistry.Serialize();

	// シーン遷移マップのシリアライズ
	l_rootJson[k_nextSceneLoadFilePathMapJsonKey] = SerializeNextSceneLoadFilePathMap(a_sceneManager);

	// シーンオブザーバーのデシリアライズ
	l_rootJson[k_sceneShiftEventObserverJsonKey] = l_sceneShiftEventObserver.Serialize();

	// シーンのシリアライズ
	l_rootJson[k_sceneJsonKey] = l_scene->Serialize(l_assetFilePathRegistry);

	Utility::SaveJsonFile(l_rootJson, l_currentSceneFilePath);
}

void FWK::Converter::SceneManagerJsonConverter::DeserializeNextSceneLoadFilePathMap(const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const
{
	if (a_rootJson.is_null())			   { return; }
	if (!Utility::IsJsonArray(a_rootJson)) { return; }

	const auto& l_assetFilePathRegistry = a_sceneManager.GetREFAssetFilePathRegistry();

	for (const auto& l_json : a_rootJson)
	{ 
		const auto& l_sceneUUID = Utility::DeserializeUUID(l_json, k_sceneUUIDJsonKey);

		if (l_sceneUUID.is_nil()) { continue; }

		a_sceneManager.AddNextSceneLoadFilePath(l_sceneUUID);
	}
}

nlohmann::json FWK::Converter::SceneManagerJsonConverter::SerializeNextSceneLoadFilePathMap(const SceneManager & a_sceneManager) const
{
	      auto  l_rootJsonArray            = nlohmann::json::array				          ();
	const auto& l_nextSceneLoadFilePathMap = a_sceneManager.GetREFNextSceneLoadFilePathMap();

	const auto& l_assetFilePathRegistry = a_sceneManager.GetREFAssetFilePathRegistry();

	for (const auto& [l_sceneUUID, l_nextSceneLoadFilePath] : l_nextSceneLoadFilePathMap)
	{
		if (l_sceneUUID.is_nil()) { continue; }

		const auto* l_assetFilePathData = l_assetFilePathRegistry.FindPTRAssetFilePathData(l_sceneUUID);

		if (!l_assetFilePathData) { continue; }

		// ファイルパが一致しなければcontinue
		if (l_assetFilePathData->m_assetFilePath.empty() ||
			l_nextSceneLoadFilePath.empty()              ||
			l_assetFilePathData->m_assetFilePath != l_nextSceneLoadFilePath)
		{
			continue; 
		}

		nlohmann::json l_json = {};

		Utility::UpdateJson(l_json, Utility::SerializeUUID(l_sceneUUID, k_sceneUUIDJsonKey));

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}