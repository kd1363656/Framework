#include "PrefabSystemJsonConverter.h"

void FWK::Converter::PrefabSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, PrefabSystem& a_prefabSystem) const
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "RootJsonが無効となっており、PrefabSystemのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_jsonArray = a_rootJson.value(k_prefabMapJsonKey, nlohmann::json::array());

	if (!Utility::IsJsonArray(l_jsonArray)) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "取得したJsonが配列になっておらずプレハブマップのデシリアライズに失敗しました。");

		return; 
	}

	for (const auto& l_json : l_jsonArray)
	{
		if (l_json.is_null()) { continue; }

		const auto& l_prefabUUID = Utility::DeserializeUUID(l_json, k_prefabUUIDJsonKey);

		// 保存されていたUUIDを復元できなかった場合
		// ここで新しいUUIDを発行してはいけない
		// Scene上のGameObjectが保持するPrefabUUIDとの
		// 対応関係が壊れてしまうため
		// このPrefab自体を登録しない
		if (l_prefabUUID.is_nil())
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabUUIDが無効のため、PrefabDataを登録できませんでした。");

			continue;
		}

		Struct::PrefabData l_prefabData = {};

		      auto& l_prefab                     = l_prefabData.m_prefab;
		      auto& l_prefabInstanceNUMAllocator = l_prefabData.m_prefabInstanceNUMAllocator;
		const auto& l_prefabJson                 = l_json.value(k_prefabJsonKey, nlohmann::json{});


		// Prefab自身へ渡すJsonが存在しない場合は、
	    // Prefab内部のDeserializeを実行しない。
		if (l_prefabJson.is_null())
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabJsonが無効のため、PrefabDataを登録できませんでした。");

			continue;
		}

		// FilePathやPrefabNameの復元、
		// 実Prefabファイルの読み込みはPrefab自身へ任せる
		l_prefab.Deserialize(l_prefabJson);

		// Prefabファイル自体を読み込めなかった場合は、
		// PrefabSystemへ不完全なPrefabを登録しない
		// Scene上のGameObjectにPrefabUUIDが残っていれば後から「PrefabUUIDはあるがPrefabSystemには存在しない」
		// 壊れた参照として判定できる
		if (l_prefab.GetREFJson().is_null())
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabのJsonを読み込めなかったため、PrefabDataを登録できませんでした。");

			continue;
		}

		const auto& l_prefabNUMInstanceAllocatorJson =  l_json.value(k_prefabInstanceNUMAllocatorJsonKey, nlohmann::json{});

		if (l_prefabNUMInstanceAllocatorJson.is_null())
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabInstanceNUMAllocatorのJsonが無効となっており、PrefabDataの登録に失敗しました。");

			continue;
		}

		l_prefabInstanceNUMAllocator.Deserialize(l_prefabNUMInstanceAllocatorJson);
		
		a_prefabSystem.AddPrefab(l_prefabUUID, l_prefabData);
	}
}

nlohmann::json FWK::Converter::PrefabSystemJsonConverter::Serialize(PrefabSystem& a_prefabSystem) const
{
	nlohmann::json l_rootJson  = {};
	auto           l_jsonArray = nlohmann::json::array();

	auto& l_prefabMap = a_prefabSystem.GetMutableREFPrefabMap();

	for (auto& [l_prefabUUID, l_prefabData] : l_prefabMap)
	{
		// NilUUIDはPrefabMapへ本来登録されないが
		// 異常なデータをJsonへ保存されないように念のため除外する
		if (l_prefabUUID.is_nil()) { continue; }

		      auto& l_prefab                     = l_prefabData.m_prefab;
		const auto& l_prefabInstanceNUMAllocator = l_prefabData.m_prefabInstanceNUMAllocator;
		
		// 読み込めないファイルならシリアライズしない
		if (const auto& l_filePath = l_prefab.GetREFFilePath();
			!Utility::CanLoadFilePath(l_filePath) ||
			l_prefab.GetREFJson().is_null())
		{
			continue; 
		}
		
		nlohmann::json l_json = {};

		const auto& l_prefabJson = l_prefab.Serialize();

		if (l_prefabJson.is_null()) { continue; }

		Utility::UpdateJson(l_json, Utility::SerializeUUID(l_prefabUUID, k_prefabUUIDJsonKey));
		l_json[k_prefabJsonKey]                     = l_prefabJson;
		l_json[k_prefabInstanceNUMAllocatorJsonKey] = l_prefabInstanceNUMAllocator.Serialize();

		l_jsonArray.emplace_back(l_json);
	}

	l_rootJson[k_prefabMapJsonKey] = l_jsonArray;

	return l_rootJson;
}