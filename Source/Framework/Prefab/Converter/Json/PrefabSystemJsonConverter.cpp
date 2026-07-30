#include "PrefabSystemJsonConverter.h"

void FWK::Converter::PrefabSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, PrefabSystem& a_prefabSystem) const
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効となっており、PrefabSystemのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_jsonArray = a_rootJson.value(k_prefabMapJsonKey, nlohmann::json::array());

	if (!Utility::IsJsonArray(l_jsonArray)) 
	{
		FWK_ADD_LOG("取得したJsonが配列になっておらずプレハブマップのデシリアライズに失敗しました。");

		return; 
	}

	for (const auto& l_json : l_jsonArray)
	{
		if (l_json.is_null()) { continue; }

		const auto& l_filePath   = l_json.value(k_prefabFilePathJsonKey, std::filesystem::path{});
		const auto& l_prefabName = l_json.value(k_prefabNameJsonKey,     std::string{});

		Prefab l_prefab = {};

		// プレハブのJsonを読み込むためのファイルパスをセットして読みこむ
		l_prefab.SetFilePath(l_filePath);
		l_prefab.LoadPrefab ();

		a_prefabSystem.AddPrefabMap(l_prefabName, l_prefab);
	}
}

nlohmann::json FWK::Converter::PrefabSystemJsonConverter::Serialize(const PrefabSystem& a_prefabSystem) const
{
	nlohmann::json l_rootJson  = {};
	auto           l_jsonArray = nlohmann::json::array();

	const auto& l_prefabMap = a_prefabSystem.GetREFPrefabMap();

	for (const auto& [l_prefabName, l_prefab] : l_prefabMap)
	{
		nlohmann::json l_json = {};

		const auto& l_filePath = l_prefab.GetREFFilePath();

		// 読み込めないファイルならシリアライズしない
		if (!Utility::CanLoadFilePath(l_filePath)) { continue; }

		l_json[k_prefabFilePathJsonKey] = l_filePath;
		l_json[k_prefabNameJsonKey]     = l_prefabName;

		l_jsonArray.emplace_back(l_json);
	}

	l_rootJson[k_prefabMapJsonKey] = l_jsonArray;

	return l_rootJson;
}