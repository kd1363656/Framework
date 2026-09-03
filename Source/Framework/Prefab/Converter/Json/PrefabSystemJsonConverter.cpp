#include "PrefabSystemJsonConverter.h"

void FWK::Converter::PrefabSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, PrefabSystem& a_prefabSystem, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson, k_prefabMapJsonKey))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "RootJsonが無効か配列でないため、PrefabSystemのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_jsonArray = a_rootJson[k_prefabMapJsonKey];
		  
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

		const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_prefabUUID);

		if (!l_assetFilePathData) 
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryにPrefabUUIDからファイルパスの取得に失敗しました。");

			continue;
		}

		// プレハブじゃないファイルパスならcontinue
		if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab) { continue; }

		Struct::PrefabData l_prefabData = {};

		      auto& l_prefab                     = l_prefabData.m_prefab;
		      auto& l_prefabInstanceNUMAllocator = l_prefabData.m_prefabInstanceNUMAllocator;
		const auto& l_assetFilePath              = l_assetFilePathData->m_assetFilePath;
		
		if (!Utility::CanLoadFilePath(l_assetFilePath, Constant::k_lowerJsonExtension)) { continue; }

		// FilePathやPrefabNameの復元、
		// 実Prefabファイルの読み込みはPrefab自身へ任せる
		l_prefab.Load(l_assetFilePath);

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

nlohmann::json FWK::Converter::PrefabSystemJsonConverter::Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry, PrefabSystem& a_prefabSystem) const
{
	nlohmann::json l_rootJson  = {};
	auto           l_jsonArray = nlohmann::json::array();

	auto& l_prefabMap = a_prefabSystem.GetMutableREFPrefabMap();
	
	for (auto& [l_prefabUUID, l_prefabData] : l_prefabMap)
	{
		// NilUUIDはPrefabMapへ本来登録されないが
		// 異常なデータをJsonへ保存されないように念のため除外する
		if (l_prefabUUID.is_nil()) { continue; }

		const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_prefabUUID);

		if (!l_assetFilePathData) 
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryにPrefabUUIDからファイルパスの取得に失敗しました。");

			continue;
		}

		// プレハブじゃないファイルパスならcontinue
		if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab) { continue; }

		      auto& l_prefab                     = l_prefabData.m_prefab;
		const auto& l_prefabInstanceNUMAllocator = l_prefabData.m_prefabInstanceNUMAllocator;
		const auto& l_filePath                   = l_assetFilePathData->m_assetFilePath;

		// 読み込めないファイルならシリアライズしない
		if (!Utility::CanLoadFilePath(l_filePath) ||
			l_prefab.GetREFJson().is_null())
		{
			continue; 
		}
		
		l_prefab.Save(l_filePath);

		nlohmann::json l_json = {};
		
		Utility::UpdateJson(l_json, Utility::SerializeUUID(l_prefabUUID, k_prefabUUIDJsonKey));
		l_json[k_prefabInstanceNUMAllocatorJsonKey] = l_prefabInstanceNUMAllocator.Serialize();

		l_jsonArray.emplace_back(l_json);
	}

	l_rootJson[k_prefabMapJsonKey] = l_jsonArray;

	return l_rootJson;
}