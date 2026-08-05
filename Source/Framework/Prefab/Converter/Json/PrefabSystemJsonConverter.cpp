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

		Struct::PrefabData l_prefabData = {};

		auto& l_prefab                     = l_prefabData.m_prefab;
		auto& l_prefabInstanceNUMAllocator = l_prefabData.m_prefabInstanceNUMAllocator;

		// プレハブのJsonを読み込むためのファイルパスをセットして読みこむ
		l_prefab.SetFilePath(l_filePath);
		l_prefab.LoadPrefab ();

		const auto& l_prefabNUMInstanceAllocatorJson =  l_json.value(k_prefabInstanceNUMAllocatorJsonKey, nlohmann::json{});

		if (l_prefabNUMInstanceAllocatorJson.is_null())
		{
			FWK_ADD_LOG("PrefabNumberIDAllocatorのJsonが無効となっておりPrefabDataの登録に失敗しました。");

			continue;
		}

		l_prefabInstanceNUMAllocator.Deserialize(l_prefabNUMInstanceAllocatorJson);
		
		a_prefabSystem.AddPrefabMap(l_prefabName, l_prefabData);
	}
}

nlohmann::json FWK::Converter::PrefabSystemJsonConverter::Serialize(PrefabSystem& a_prefabSystem) const
{
	nlohmann::json l_rootJson  = {};
	auto           l_jsonArray = nlohmann::json::array();

	auto& l_prefabMap = a_prefabSystem.GetMutableREFPrefabMap();

	for (auto& [l_prefabName, l_prefabData] : l_prefabMap)
	{
		      nlohmann::json l_json                       = {};
		      auto&          l_prefab                     = l_prefabData.m_prefab;
		const auto&          l_prefabInstanceNUMAllocator = l_prefabData.m_prefabInstanceNUMAllocator;

		const auto& l_filePath = l_prefab.GetREFFilePath();

		// 読み込めないファイルならシリアライズしない
		if (!Utility::CanLoadFilePath(l_filePath)) { continue; }

		l_json[k_prefabFilePathJsonKey]             = l_filePath;
		l_json[k_prefabNameJsonKey]                 = l_prefabName;
		l_json[k_prefabInstanceNUMAllocatorJsonKey] = l_prefabInstanceNUMAllocator.Serialize();

		l_prefab.SavePrefab();
		
		// 保存時に更新されるJsonが空なら保存処理をスキップする
		if (l_prefab.GetREFJson().is_null()) { continue; }

		l_jsonArray.emplace_back(l_json);
	}

	l_rootJson[k_prefabMapJsonKey] = l_jsonArray;

	return l_rootJson;
}