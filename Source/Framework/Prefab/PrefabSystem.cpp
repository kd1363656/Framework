#include "PrefabSystem.h"

void FWK::PrefabSystem::INIT()
{
	m_prefabMap.clear();
}
void FWK::PrefabSystem::Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this, a_assetFilePathRegistry);
}

void FWK::PrefabSystem::CachePrefabGameObjectIfNeeded(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObject = a_gameObject.lock();
	
	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed()) 
	{
		return; 
	}

	const auto& l_prefabUUID             = l_gameObject->GetREFPrefabUUID            ();
	const auto  l_prefabSceneInstanceNUM = l_gameObject->GetVALPrefabSceneInstanceNUM();

	// PrefabUUIDとPrefabInstanceのNUMの両方を持つGameObjectだけを、
	// Prefabの代表GameObject候補として扱う
	if (l_prefabUUID.is_nil() ||
		l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
	{
		return;
	}

	auto l_itr = m_prefabMap.find(l_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return; }

	auto& l_prefab = l_itr->second.m_prefab;
	
	// 既に有効な代表GameObjectが存在しており、
	// 同じPrefabUUIDを参照している場合は
	// 現在の代表GameObjectをそのまま維持する
	if (const auto& l_cachedGameObject = l_prefab.GetREFGameObject().lock();
		l_cachedGameObject                       &&
		!l_cachedGameObject->GetVALIsDestroyed() &&
		Utility::IsSamePrefab(*l_cachedGameObject, *l_gameObject))
	{
		return;
	}

	// キャッシュが空、削除予定、または異なるPrefabを示していた場合に
	// 新しい代表GameObjectへ差し替える
	l_prefab.SetGameObject(l_gameObject);
}

void FWK::PrefabSystem::RefreshAllPrefab()
{
	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene) { return; }

	const auto& l_assetFilePathRegistry = l_sceneManager.GetREFAssetFilePathRegistry();
	const auto& l_gameObjectList        = l_scene->GetREFGameObjectList             ();

	for (auto& [l_prefabUUID, l_prefabData] : m_prefabMap)
	{
		if (l_prefabUUID.is_nil()) { continue; }

		auto& l_prefab = l_prefabData.m_prefab;
		
		bool l_isPrefabGameObjectFound = false;

		// プレハブ構築用ゲームオブジェクトが無効ならプレハブ構築用ゲームオブジェクトをシーンから探す
		if (const auto& l_prefabGameObject = l_prefab.GetREFGameObject().lock();
			 l_prefabGameObject                                     &&
			!l_prefabGameObject->GetVALIsDestroyed()                &&
			 l_prefabUUID == l_prefabGameObject->GetREFPrefabUUID() &&
			 l_prefabGameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabSceneInstanceNUM)
		{
			l_isPrefabGameObjectFound = true;
		}
		// 現在のPrefab構築用GameObjectが無効なら
		// 同じPrefabUUIDを持つGameObjectをシーンから探しなおす
		else
		{
			// 代表GameObjectが存在しないPrefabは
			// 現在のScene状態から新しいPrefab情報を生成できない
			// そのためPrefabゲームオブジェクトをもう一度探し直し設定する
			for (const auto& l_gameObject : l_gameObjectList)
			{
				if (!l_gameObject                                    || 
					l_gameObject->GetVALIsDestroyed()                ||
					l_prefabUUID != l_gameObject->GetREFPrefabUUID() ||
					l_gameObject->GetVALPrefabSceneInstanceNUM() == Constant::k_invalidPrefabSceneInstanceNUM) 
				{
					continue; 
				}

				l_prefab.SetGameObject(l_gameObject);

				l_isPrefabGameObjectFound = true;

				// 代表Prefab構築用ゲームオブジェクトを見つけたら走査する必要がないためbreak
				break;
			}	
		}

		// Scene上に子のPrefabの有効なGameObjectが存在しない場合は
		// Destroy済みGameObjectからPrefabファイルを書き換えない
		if (!l_isPrefabGameObjectFound) { continue; }

		const auto* l_assetFilePathData = l_assetFilePathRegistry.FindPTRAssetFilePathData(l_prefabUUID);

		// Prefab用のAssetFilePath出ないなら処理を飛ばす
		if (!l_assetFilePathData || 
			l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab) 
		{
			continue; 
		}

		// シリアライズでファイルを保存するがそれとともに戻り値で
		// nlohmann::jsonで保存内容が空かどうかを確認する
		l_prefab.Save(l_assetFilePathData->m_assetFilePath);
	}
}

void FWK::PrefabSystem::AddPrefab(const boost::uuids::uuid& a_prefabUUID, const Struct::PrefabData& a_prefabData)
{
	if (a_prefabUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabUUIDが無効だったため、PrefabSystemのプレハブマップに追加できませんでした。");

		return;
	}

	if (!m_prefabMap.try_emplace(a_prefabUUID, a_prefabData).second)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "同じPrefabUUIDが既に登録されており、PrefabSystemのプレハブマップに追加できませんでした。");
	}
}
void FWK::PrefabSystem::RemovePrefab(const boost::uuids::uuid& a_prefabUUID)
{
	// NilUUIDはPrefabSystemへ登録されないので
	// Map検索を行わず終了する
	if (a_prefabUUID.is_nil()) { return; }

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return; }

	m_prefabMap.erase(l_itr);

	FWK_ADD_LOG(Constant::k_debugSuccessColor, "PrefabUUID : {}\nのプレハブを削除しました。", boost::uuids::to_string(a_prefabUUID));
}

nlohmann::json FWK::PrefabSystem::Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry)
{
	return m_jsonConverter.Serialize(a_assetFilePathRegistry, *this);
}

FWK::TypeAlias::PrefabSceneInstanceNUM FWK::PrefabSystem::AllocatePrefabInstanceNUM(const boost::uuids::uuid& a_prefabUUID)
{
	if (a_prefabUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabUUIDが無効のため、PrefabInstanceNUMを発行できませんでした。");

		return Constant::k_invalidPrefabSceneInstanceNUM;
	}

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabUUID : {}\nのPrefabが登録されていないため、PrefabInstanceNUMを発行できませんでした。", boost::uuids::to_string(a_prefabUUID));

		return Constant::k_invalidPrefabSceneInstanceNUM;
	}

	auto& l_prefabInstanceNUMAllocator = l_itr->second.m_prefabInstanceNUMAllocator;
		
	// 使用可能なInstanceNUMを発行
	return l_prefabInstanceNUMAllocator.Allocate();
}

void FWK::PrefabSystem::ReleasePrefabInstanceNUM(const boost::uuids::uuid& a_prefabUUID, const TypeAlias::PrefabSceneInstanceNUM a_prefabInstanceNUM)
{
	if (a_prefabUUID.is_nil() ||
		a_prefabInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
	{
		return;
	}

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return; }

	auto& l_prefabInstanceNUMAllocator = l_itr->second.m_prefabInstanceNUMAllocator;

	l_prefabInstanceNUMAllocator.Release(a_prefabInstanceNUM);
}

const FWK::Prefab* FWK::PrefabSystem::FindPTRPrefab(const boost::uuids::uuid& a_prefabUUID) const
{
	// NilUUIDからPrefabを検索することはできない
	if (a_prefabUUID.is_nil()) { return nullptr; }

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return nullptr; }

	return &l_itr->second.m_prefab;
}

FWK::Prefab* FWK::PrefabSystem::FindMutablePTRPrefab(const boost::uuids::uuid& a_prefabUUID)
{
	// NilUUIDからPrefabを検索することはできない
	if (a_prefabUUID.is_nil()) { return nullptr; }

	auto l_itr = m_prefabMap.find(a_prefabUUID);

	if (l_itr == m_prefabMap.end()) { return nullptr; }

	return &l_itr->second.m_prefab;
}