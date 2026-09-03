#include "AssetBrowserEditorWindowFileSystem.h"

std::filesystem::path FWK::Editor::AssetBrowserEditorWindowFileSystem::CreateFolder(const std::filesystem::path& a_parentFolderPath, const std::filesystem::path& a_folderName) const
{
	if (a_folderName.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダ名が空のため、フォルダを作成できませんでした。");

		return {};
	}

	std::error_code l_errorCode = {};

	// Folder作成先が実際に存在するFolderか確認する
	if (!std::filesystem::is_directory(a_parentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダ作成先が無効です。\nFolderPath : {}", a_parentFolderPath.string());

		return {};
	}

	// 一度のFolder作成で複数階層を作成させない
	if (const std::filesystem::path& l_folderNamePath = a_folderName;
		l_folderNamePath.has_parent_path())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダ名にFolderPathを含めることはできません。\nFolderName : {}", a_folderName.string());

		return {};
	}

	const std::filesystem::path& l_newFolderPath = a_parentFolderPath / a_folderName;

	l_errorCode.clear();

	// 同じ名前のファイルまたはフォルダーを上書きしない
	if (std::filesystem::exists(l_newFolderPath, l_errorCode))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "同名のファイルまたはフォルダーが既に存在します。\nFolderPath : {}", l_newFolderPath.string());

		return {};
	}

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダー名を確認できませんでした。\nFolderPath : {}", l_newFolderPath.string());

		return {};
	}

	l_errorCode.clear();

	if (!std::filesystem::create_directory(l_newFolderPath, l_errorCode))
	{
	
		FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダーを作成できませんでした。\nFolderPath : {}", l_newFolderPath.string());

		return {};
	}

	FWK_ADD_LOG(Constant::k_debugSuccessColor, "フォルダーを作成しました。\nFolderPath : {}", l_newFolderPath.string());
	
	return l_newFolderPath;
}
std::filesystem::path FWK::Editor::AssetBrowserEditorWindowFileSystem::CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab化するGameObjectが無効のため、Prefabを作成できませんでした。");

		return {};
	}

	// 保存先フォルダ確認
	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_parentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab保存先フォルダが無効のため、Prefabを作成できませんでした。\nFolderPath : {}", a_parentFolderPath.string());

		return {};
	}

	// Prefab化されていないことを確認
	if (!l_gameObject->GetREFPrefabUUID().is_nil() ||
		l_gameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabSceneInstanceNUM)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "すでにPrefabInstanceとなっているGameObjectは新規Prefab化できません。");

		return {};
	}

	// OutlinerでF2リネームされたSceneInstanceNameをそのままPrefabNameとして使用する
	const auto& l_prefabName = l_gameObject->GetREFSceneInstanceName();

	if (l_prefabName.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneInstanceNameが空のため、PrefabNameを決定できませんでした。");

		return {};
	}

	std::filesystem::path l_prefabFilePath = a_parentFolderPath / l_prefabName;

	l_prefabFilePath += Constant::k_lowerJsonExtension.string();

	l_errorCode.clear();

	// 既存ファイルを勝手に上書きしない
	if (std::filesystem::exists(l_prefabFilePath, l_errorCode))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "同名Prefabファイルが既に存在するため、新しいPrefabを作成できませんでした。\nFilePath : {}", l_prefabFilePath.string());

		return {};
	}

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabFilePathを確認できなかったため、Prefabを作成できませんでした。\nFilePath : {}", l_prefabFilePath.string());

		return {};
	}

	// Registry上でも同じfilePathが使用済みなら作成しない
	if (const auto* l_uuid = a_assetFilePathRegistry.FindPTRAssetUUID(l_prefabFilePath);
		!l_uuid ||
		l_uuid->is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "同じFilePathがContentBrowserAssetRegistryへ既に登録されています。\nFilePath : {}", l_prefabFilePath.string());

		return {};
	}

	// PrefabUUIDの生成
	auto&              l_uuidManager = UUIDManager::GetInstance ();
	boost::uuids::uuid l_prefabUUID  = {};

	// UUIDが重複しないよにする
	while (true)
	{
		l_prefabUUID = l_uuidManager.GenerateVALUUID();

		if (l_prefabUUID.is_nil()) { continue; }

		if (a_assetFilePathRegistry.ContainsAssetUUID(l_prefabUUID)) { continue; }

		break;
	}

	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene) { return {}; }

	auto& l_prefabSystem = l_scene->GetMutableREFPrefabSystem();

	// PrefabSystem側でもUUIDが使用済みなら登録しない
	if (l_prefabSystem.FindPTRPrefab(l_prefabUUID))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "生成したPrefabUUIDがPrefabSystemですでに使用されています。");

		return {};
	}

	if (!a_assetFilePathRegistry.Add(l_prefabFilePath, l_prefabUUID, Enum::AssetFilePathRegistryType::Prefab))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "ContentBrowserAssetRegistryへPrefabを登録できませんでした。");

		return {};
	}

	Struct::PrefabData l_prefabData = {};

	auto& l_prefab = l_prefabData.m_prefab;

	l_prefab.SetPrefabName(l_prefabName);
	
	// 今回Prefab化するScene上のGameObjectを
	// このPrefabの保存用代表ゲームオブジェクトとして使用する
	l_prefab.SetGameObject(l_gameObject);

	l_prefabSystem.AddPrefab(l_prefabUUID, l_prefabData);

	auto* l_registeredPrefab = l_prefabSystem.FindMutablePTRPrefab(l_prefabUUID);

	if (!l_registeredPrefab)
	{
		a_assetFilePathRegistry.Erase(l_prefabFilePath);

		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabSystemへのPrefab登録に失敗しました。");

		return {};
	}

	const auto l_prefabSceneInstanceNUM = l_prefabSystem.AllocatePrefabInstanceNUM(l_prefabUUID);

	// シーンインスタンス数が無効値ならAssetRegistryやPrefabSystemから情報を消す
	if (l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
	{
		l_prefabSystem.RemovePrefab   (l_prefabUUID);
		a_assetFilePathRegistry.Erase(l_prefabFilePath);

		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabInstanceNUMを発行できなかったため、Prefab作成を中止しました。");

		return {};
	}

	l_gameObject->SetPrefabUUID            (l_prefabUUID);
	l_gameObject->SetPrefabSceneInstanceNUM(l_prefabSceneInstanceNUM);

	// 現在のプレハブ内容を反映するために保存
	l_registeredPrefab->Save(l_prefabFilePath);

	if (const auto& l_prefabSerializeJson = l_registeredPrefab->GetREFJson();
		l_prefabSerializeJson.is_null())
	{
		// InstanceNUMをPrefabSystemへ返す
		l_prefabSystem.ReleasePrefabInstanceNUM(l_prefabUUID, l_prefabSceneInstanceNUM);
		
		// GameObjectをPrefab化前に戻す
		l_gameObject->SetPrefabSceneInstanceNUM(Constant::k_invalidPrefabSceneInstanceNUM);

		l_gameObject->SetPrefabUUID({});

		l_prefabSystem.RemovePrefab(l_prefabUUID);

		a_assetFilePathRegistry.Erase(l_prefabFilePath);

		// SaveJsonFileの途中でFileだけ生成された場合も
		// 不完全なPrefabファイルを渡さない
		l_errorCode.clear();

		std::filesystem::remove(l_prefabFilePath, l_errorCode);

		FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefabファイルの保存に失敗したため、Prefab作成を取り消しました。");

		return {};
	}

	// 新しPrefabの登録・保存が完全に成功した後で、
	// Scene上の現在のPrefab構造を全Prefabへ反映する
	l_prefabSystem.RefreshAllPrefab();

	FWK_ADD_LOG(Constant::k_debugSuccessColor, "Prefabを作成しました。\nPrefabName : {}\nFilePath : {}\nPrefabUUID : {}\nPrefabInstanceNUM : {}",
		        l_prefabName,
		        l_prefabFilePath.string(),
		        boost::uuids::to_string(l_prefabUUID),
		        l_prefabSceneInstanceNUM);

	return l_prefabFilePath;
}
std::filesystem::path FWK::Editor::AssetBrowserEditorWindowFileSystem::CreateSceneFromScene(const std::weak_ptr<Scene>& a_scene, const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
	const auto& l_scene = a_scene.lock();

	if (!l_scene)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Sceneが無効なため、Sceneを作成できませんでした。");

		return {};
	}

	// 保存先が実際に存在するFolderか確認する
	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_parentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Scene保存先フォルダが無効なため、Sceneを作成できませんでした。\nFolderPath : {}", a_parentFolderPath.string());

		return {};
	}

	const auto& l_sceneName = l_scene->GetREFSceneName();

	if (l_sceneName.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneNameが空のため、SceneFilePathを決定できませんでした。");

		return {};
	}

	// "..Stage"や"Scene/Stage"のように
	// SceneNameから別Folderへ保存させない
	if (const std::filesystem::path& l_sceneNamePath = l_sceneName;
		l_sceneNamePath.has_parent_path())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneNameにFolderPathを含めることはできません。\nSceneName : {}", l_sceneName);

		return {};
	}

	std::filesystem::path l_sceneFilePath = a_parentFolderPath / l_sceneName;

	l_sceneFilePath += Constant::k_lowerJsonExtension;
	
	l_errorCode.clear();

	// ドラッグ&ドロップでは既存Sceneファイルを勝手に上書きしない
	if (std::filesystem::exists(l_sceneFilePath, l_errorCode))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "同名Sceneファイルが既に存在するため、新しいSceneを作成できませんでした。\nFilePath : {}", l_sceneFilePath.string());

		return {};
	}

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneFilePathを確認できなかったため、Sceneを作成できませんでした。\nFilePath : {}", l_sceneFilePath.string());

		return {};
	}

	// Registry上でも同じFilePathが使用済みなら作成しない
	if (const auto* l_uuid = a_assetFilePathRegistry.FindPTRAssetUUID(l_sceneFilePath);
		!l_uuid ||
		l_uuid->is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "同じSceneFilePathがContentBrowserAssetRegistryへ既に登録されています。\nFilePath : {}", l_sceneFilePath.string());

		return {};
	}

	      auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_currentScene = l_sceneManager.GetVALScene().lock();

	// OutlinerでDragしたSceneと
	// SceneManagerが現在管理しているSceneが同じであることを確認する
	if (!l_currentScene ||
		l_currentScene != l_scene)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "現在SceneとDrag&DropされたSceneが一致しないため、Sceneを作成できませんでした。");

		return {};
	}

	// Prefab作成と同じく
	// Assetを実際に作成するFileSystem側でUUIDを発行する
	auto& l_uuidManager = UUIDManager::GetInstance();

	boost::uuids::uuid l_sceneUUID = {};

	// 少なくともSceneRegistry内でUUIDが重複しないものを発行する
	while (true)
	{
		l_sceneUUID = l_uuidManager.GenerateVALUUID();

		if (l_sceneUUID.is_nil()) { continue; }

		if (a_assetFilePathRegistry.ContainsAssetUUID(l_sceneUUID)) { continue; }

		break;
	}

	// FilePathに対応するUUIDを発行
	if (!a_assetFilePathRegistry.Add(l_sceneFilePath, l_sceneUUID, Enum::AssetFilePathRegistryType::Scene))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "ContentBrowserAssetRegistryへSceneを登録できませんでした。");
		
		return {};
	}

	// SceneManagerの既存Serialize処理を使用して、
	// 今回決定したFilePathへSceneJsonを作成する
	if (!l_sceneManager.SaveScene(l_sceneFilePath))
	{
		// Json保存に失敗した場合
		// SceneAsset化前の状態へ戻す
		a_assetFilePathRegistry.Erase(l_sceneFilePath);

		// SaveJsonFile途中で不完全なFileだけ作成された場合も削除する
		l_errorCode.clear();

		std::filesystem::remove(l_sceneFilePath, l_errorCode);

		FWK_ADD_LOG(Constant::k_debugWarningColor, "Sceneファイルの保存に失敗したため、Scene作成を取り消しました。");

		return {};
	}

	return l_sceneFilePath;
}

bool FWK::Editor::AssetBrowserEditorWindowFileSystem::DeleteFolder(const std::filesystem::path& a_folderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
	if (a_folderPath.empty()) { return false; }

	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_folderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "削除対象Folderが無効です。\nFolderPath : {}", a_folderPath.string());

		return false;
	}

	std::vector<std::filesystem::path> l_prefabFilePathList = {};
	std::vector<std::filesystem::path> l_sceneFilePathList  = {};

	l_errorCode.clear();

	      std::filesystem::recursive_directory_iterator  l_folderEntryITR    = { a_folderPath, l_errorCode };
	const std::filesystem::recursive_directory_iterator& l_endFolderEntryITR = {};

	while (l_folderEntryITR != l_endFolderEntryITR)
	{
		std::error_code l_entryErrorCode = {};

		const bool l_isFile = l_folderEntryITR->is_regular_file(l_entryErrorCode);

		if (!l_entryErrorCode &&
			l_isFile)
		{ 
			const auto& l_filePath = l_folderEntryITR->path();

			// AssetRegistryにUUIDが登録されているFileだけを現在管理対象のPrefabとして扱う
			if (const auto& l_prefabUUID = a_assetFilePathRegistry.FindPTRAssetUUID(l_filePath);
				l_prefabUUID &&
				!l_prefabUUID->is_nil())
			{
				l_prefabFilePathList.emplace_back(l_filePath);
			}
			else if (const auto& l_sceneUUID = a_assetFilePathRegistry.FindPTRAssetUUID(l_filePath);
				l_sceneUUID &&
				!l_sceneUUID->is_nil())
			{
				l_sceneFilePathList.emplace_back(l_filePath);
			}
		}

		l_folderEntryITR.increment(l_errorCode);

		if (l_errorCode)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "削除対象フォルダーの走査中にエラーが発生しました。\nFolderPath : {}", a_folderPath.string());

			return false;
		}
	}

	// recursive_directory_iteratorで走査子ながらFileを削除すると、
	// Iteratorへ影響する可能性がある
	// そのためPath収集が完全に終わった後で削除する
	for (const auto& l_prefabFilePath : l_prefabFilePathList)
	{
		if (!DeletePrefabFile(l_prefabFilePath, a_assetFilePathRegistry))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダー配下のPrefab削除に失敗したため、Folder削除を中止しました。\nPrefabFilePath : {}", l_prefabFilePath.string());

			return false;
		}
	}

	for (const auto& l_sceneFilePath : l_sceneFilePathList)
	{
		if (!DeleteSceneFile(l_sceneFilePath, a_assetFilePathRegistry))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダー配下のScene削除に失敗したため、Folder削除を中止しました。\nSceneFilePath : {}", l_sceneFilePath.string());

			return false;
		}
	}

	// Prefabファイルについては
	// 上ですでに正規のDeletePrefabFile()を通している
	// PNG/FBX等の残りFileと子フォルダーはフォルダー削除の対象なのでまとめて削除する
	l_errorCode.clear();

	const auto& l_removedEntryCount = std::filesystem::remove_all(a_folderPath, l_errorCode);

	if (l_errorCode ||
		l_removedEntryCount == k_notRemovedEntryCount)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "フォルダーを削除できませんでした。\nFolderPath : {}", a_folderPath.string());

		return false;
	}
	
	FWK_ADD_LOG(Constant::k_debugSuccessColor, "フォルダーを削除しました。\nFolderPath : {}", a_folderPath.string());

	return true;
}

bool FWK::Editor::AssetBrowserEditorWindowFileSystem::DeletePrefabFile(const std::filesystem::path& a_prefabFilePath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
	if (a_prefabFilePath.empty()) { return false; }

	const auto& l_sceneManager   = SceneManager::GetInstance        ();
	const auto& l_scene          = l_sceneManager.GetVALScene().lock();
	
	if (!l_scene) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Sceneが無効のため、Prefabを削除できませんでした。\nFilePath : {}", a_prefabFilePath.string());

		return false; 
	}

	const auto l_prefabUUID = a_assetFilePathRegistry.FindPTRAssetUUID(a_prefabFilePath);

	if (!l_prefabUUID ||
		l_prefabUUID->is_nil()) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "削除対象PrefabのUUIDを取得できませんでした。\nFilePath : {}", a_prefabFilePath.string());

		return false;
	}

	// ファイル削除に失敗した場合にregistryを戻せるように
	// UUIDを取得した後でRegistryから削除する
	if (!a_assetFilePathRegistry.Erase(a_prefabFilePath))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "PrefabをAssetRegistryから削除できませんでした。\nFilePath : {}", a_prefabFilePath.string());

		return false;
	}

	if (std::error_code l_errorCode = {};
		!std::filesystem::remove(a_prefabFilePath, l_errorCode) ||
		l_errorCode)
	{
		// 実ファイル削除に失敗したので
		// AssetRegistryを削除前の状態に戻す
		if (!a_assetFilePathRegistry.Add(a_prefabFilePath, *l_prefabUUID, Enum::AssetFilePathRegistryType::Prefab))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab削除失敗後のAssetRegistry復元にも失敗しました。\nFilePath : {}", a_prefabFilePath.string());
		}

		FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefabファイルを削除できませんでした。\nFilePath : {}", a_prefabFilePath.string());

		return false;
	}

	// シーンにある現在削除したプレハブのゲームオブジェクトをすべて削除する
	const auto& l_gameObjectList = l_scene->GetREFGameObjectList();

	for (const auto& l_gameObject : l_gameObjectList)
	{
		if (!l_gameObject ||
			l_gameObject->GetVALIsDestroyed())
		{
			continue;
		}

		// 削除したPrefabと関係ないGameObjectは何もしない
		if (l_gameObject->GetREFPrefabUUID() != *l_prefabUUID)
		{
			continue;
		}

		l_gameObject->Destroy();
	}

	auto& l_prefabSystem = l_scene->GetMutableREFPrefabSystem();

	l_prefabSystem.RemovePrefab    (*l_prefabUUID);
	l_prefabSystem.RefreshAllPrefab();

	FWK_ADD_LOG(Constant::k_debugSuccessColor, "Prefabを削除しました。\nFilePath : {}\nPrefabUUID : {}", a_prefabFilePath.string(), boost::uuids::to_string(*l_prefabUUID));

	return true;
}

bool FWK::Editor::AssetBrowserEditorWindowFileSystem::DeleteSceneFile(const std::filesystem::path& a_sceneFilePath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
	if (a_sceneFilePath.empty()) { return false; }

	// SceneFilePathに対応しているUUIDを先に取得する
	// ファイル削除に失敗した場合、このUUIDを使ってRegistryを復元する
	const auto& l_sceneUUID = a_assetFilePathRegistry.FindPTRAssetUUID(a_sceneFilePath);

	if (!l_sceneUUID ||
		l_sceneUUID->is_nil()) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor,"削除対象SceneのUUIDを取得できませんでした。\nFilePath : {}", a_sceneFilePath.string());

		return false;
	}

	// SceneUUIDとFilePathの関連をAssetRegistryから解除する
	if (!a_assetFilePathRegistry.Erase(a_sceneFilePath))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneをAssetRegistryから削除できませんでした。\nFilePath : {}", a_sceneFilePath.string());

		return false;
	}

	if (std::error_code l_errorCode = {};
		!std::filesystem::remove(a_sceneFilePath, l_errorCode) ||
		l_errorCode)
	{
		// 実ファイルを削除できなかった場合、
		// Registryだけ削除済みになるとFileとUUIDの状態が壊れる
		// そのため削除前のUUIDとFilePathをRegistryへ戻す
		if (!a_assetFilePathRegistry.Add(a_sceneFilePath, *l_sceneUUID, Enum::AssetFilePathRegistryType::Scene))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "Scene削除失敗後のAssetRegistry復元にも失敗しました。\nFilePath : {}", a_sceneFilePath.string());
		}

		FWK_ADD_LOG(Constant::k_debugWarningColor, "Sceneファイルを削除できませんでした。\nFilePath : {}", a_sceneFilePath.string());

		return false;
	}

	FWK_ADD_LOG(Constant::k_debugSuccessColor,
		        "Sceneを削除しました。\nFilePath : {}\nSceneUUID : {}",
		        a_sceneFilePath.string(),
		        boost::uuids::to_string(*l_sceneUUID));

	return true;
}

bool FWK::Editor::AssetBrowserEditorWindowFileSystem::DeleteRegularFile(const std::filesystem::path& a_filePath) const
{
	if (a_filePath.empty()) { return false; }

	std::error_code l_errorCode = {};

	// Folderや特殊なPathを通常File削除処理へ渡さない
	if (!std::filesystem::is_regular_file(a_filePath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "削除対象Fileが無効です。\nFilePath : {}", a_filePath.string());

		return false;
	}

	l_errorCode.clear();

	if (!std::filesystem::remove(a_filePath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Fileを削除できませんでした。\nFilePath : {}", a_filePath.string());

		return false;
	}

	FWK_ADD_LOG(Constant::k_debugSuccessColor, "Fileを削除しました。\nFilePath : {}", a_filePath.string());

	return true;
}
bool FWK::Editor::AssetBrowserEditorWindowFileSystem::HasChildFolder(const std::filesystem::path& a_folderPath) const
{
	std::error_code                     l_errorCode         = {};
	std::filesystem::directory_iterator l_folderEntryITR    = { a_folderPath, l_errorCode };
	std::filesystem::directory_iterator l_endFolderEntryITR = {};

	if (l_errorCode) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "子フォルダーの確認に失敗しました。\nFolderPath : {}", a_folderPath.string());

		return false;
	}

	while (l_folderEntryITR != l_endFolderEntryITR)
	{
		if (std::error_code l_entryErrorCode = {};
			l_folderEntryITR->is_directory(l_entryErrorCode) &&
			!l_entryErrorCode)
		{
			return true;
		}

		l_folderEntryITR.increment(l_errorCode);

		if (l_errorCode)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "子フォルダーの走査中にエラーが発生しました。\nFolderPath : {}", a_folderPath.string());

			return false;
		}
	}

	return false;
}