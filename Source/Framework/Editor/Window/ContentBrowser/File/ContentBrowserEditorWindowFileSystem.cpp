#include "ContentBrowserEditorWindowFileSystem.h"

std::filesystem::path FWK::Editor::ContentBrowserEditorWindowFileSystem::CreateFolder(const std::filesystem::path& a_parentFolderPath, const std::string& a_folderName) const
{
	if (a_folderName.empty())
	{
		FWK_ADD_LOG("フォルダ名が空のため、フォルダを作成できませんでした。");

		return {};
	}

	std::error_code l_errorCode = {};

	// Folder作成先が実際に存在するFolderか確認する
	if (!std::filesystem::is_directory(a_parentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG("フォルダ作成先が無効です。\nFolderPath : {}", a_parentFolderPath.string());

		return {};
	}

	// 一度のFolder作成で複数階層を作成させない
	const std::filesystem::path& l_folderNamePath = a_folderName;

	if (l_folderNamePath.has_parent_path())
	{
		FWK_ADD_LOG("フォルダ名にFolderPathを含めることはできません。\nFolderName : {}", a_folderName);

		return {};
	}

	const std::filesystem::path& l_newFolderPath = a_parentFolderPath / a_folderName;

	l_errorCode.clear();

	// 同じ名前のファイルまたはフォルダーを上書きしない
	if (std::filesystem::exists(l_newFolderPath, l_errorCode))
	{
		FWK_ADD_LOG("同名のファイルまたはフォルダーが既に存在します。\nFolderPath : {}", l_newFolderPath.string());

		return {};
	}

	if (l_errorCode)
	{
		FWK_ADD_LOG("フォルダー名を確認できませんでした。\nFolderPath : {}", l_newFolderPath.string());

		return {};
	}

	l_errorCode.clear();

	if (!std::filesystem::create_directory(l_newFolderPath, l_errorCode))
	{
	
		FWK_ADD_LOG("フォルダーを作成できませんでした。\nFolderPath : {}", l_newFolderPath.string());

		return {};
	}

	FWK_ADD_LOG("フォルダーを作成しました。\nFolderPath : {}", l_newFolderPath.string());
	
	return l_newFolderPath;
}
std::filesystem::path FWK::Editor::ContentBrowserEditorWindowFileSystem::CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_parentFolderPath, ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		FWK_ADD_LOG("Prefab化するGameObjectが無効のため、Prefabを作成できませんでした。");

		return {};
	}

	// 保存先フォルダ確認
	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_parentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG("Prefab保存先フォルダが無効のため、Prefabを作成できませんでした。\nFolderPath : {}", a_parentFolderPath.string());

		return {};
	}

	// Prefab化されていないことを確認
	if (!l_gameObject->GetREFPrefabUUID().is_nil() ||
		l_gameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabSceneInstanceNUM)
	{
		FWK_ADD_LOG("すでにPrefabInstanceとなっているGameObjectは新規Prefab化できません。");

		return {};
	}

	// Outlinderでf2リネームされたSceneInstanceNameをそのままPrefabNameとして使用する
	const auto& l_prefabName = l_gameObject->GetREFSceneInstanceName();

	if (l_prefabName.empty())
	{
		FWK_ADD_LOG("SceneInstanceNameが空のため、PrefabNameを決定できませんでした。");

		return {};
	}

	std::filesystem::path l_prefabFilePath = a_parentFolderPath / l_prefabName;

	l_prefabFilePath += Constant::k_lowerJsonExtension.string();

	l_errorCode.clear();

	// 既存ファイルを勝手に上書きしない
	if (std::filesystem::exists(l_prefabFilePath, l_errorCode))
	{
		FWK_ADD_LOG("同名Prefabファイルが既に存在するため、新しいPrefabを作成できませんでした。\nFilePath : {}", l_prefabFilePath.string());

		return {};
	}

	if (l_errorCode)
	{
		FWK_ADD_LOG("PrefabFilePathを確認できなかったため、Prefabを作成できませんでした。\nFilePath : {}", l_prefabFilePath.string());

		return {};
	}

	// Registry上でも同じfilePathが使用済みなら作成しない
	if (!a_assetRegistry.FindVALAssetUUID(l_prefabFilePath).is_nil())
	{
		FWK_ADD_LOG("同じFilePathがContentBrowserAssetRegistryへ既に登録されています。\nFilePath : {}", l_prefabFilePath.string());

		return {};
	}

	// PrefabUUIDの生成
	      auto& l_uuidManager = UUIDManager::GetInstance     ();
	const auto& l_prefabUUID  = l_uuidManager.GenerateVALUUID();

	if (l_prefabUUID.is_nil())
	{
		FWK_ADD_LOG("PrefabUUIDを生成できなかったため、Prefabを作成できませんでした。");

		return {};
	}

	const auto& l_sceneManager = SceneManager::GetInstance         ();
	const auto& l_scene        = l_sceneManager.GetVALScene        ().lock();
	      auto& l_prefabSystem = l_scene->GetMutableREFPrefabSystem();

	if (l_scene) { return {}; }

	// PrefabSystem側でもUUIDが使用済みなら登録しない
	if (l_prefabSystem.FindPTRPrefab(l_prefabUUID))
	{
		FWK_ADD_LOG("生成したPrefabUUIDがPrefabSystemですでに使用されています。");

		return {};
	}

	if (!a_assetRegistry.Add(l_prefabUUID, l_prefabFilePath))
	{
		FWK_ADD_LOG("ContentBrowserAssetRegistryへPrefabを登録できませんでした。");

		return {};
	}

	Struct::PrefabData l_prefabData = {};

	auto& l_prefab = l_prefabData.m_prefab;

	l_prefab.SetPrefabName(l_prefabName);
	l_prefab.SetFilePath  (l_prefabFilePath);

	// 今回Prefab化するScene上のGameObjectを
	// このPrefabの保存用代表ゲームオブジェクトとして使用する
	l_prefab.SetGameObject(l_gameObject);

	l_prefabSystem.AddPrefabMap(l_prefabUUID, l_prefabData);

	auto* l_registeredPrefab = l_prefabSystem.FindMutablePTRPrefab(l_prefabUUID);

	if (!l_registeredPrefab)
	{
		a_assetRegistry.Erase(l_prefabFilePath);

		FWK_ADD_LOG("PrefabSystemへのPrefab登録に失敗しました。");

		return {};
	}

	const auto l_prefabSceneInstanceNUM = l_prefabSystem.AllocatePrefabInstanceNUM(l_prefabUUID);

	// シーンインスタンス数が無効値ならAssetRegistryやPrefabSystemから情報を消す
	if (l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
	{
		l_prefabSystem.RemovePrefab(l_prefabUUID);
		a_assetRegistry.Erase      (l_prefabFilePath);

		FWK_ADD_LOG("PrefabInstanceNUMを発行できなかったため、Prefab作成を中止しました。");

		return {};
	}

	l_gameObject->SetPrefabUUID            (l_prefabUUID);
	l_gameObject->SetPrefabSceneInstanceNUM(l_prefabSceneInstanceNUM);

	if (const auto& l_prefabSerializeJson = l_registeredPrefab->Serialize();
		l_prefabSerializeJson.is_null())
	{
		// InstanceNUMをPrefabSystemへ返す
		l_prefabSystem.ReleasePrefabInstanceNUM(l_prefabUUID, l_prefabSceneInstanceNUM);
		
		// GameObjectをPrefab化前に戻す
		l_gameObject->SetPrefabSceneInstanceNUM(Constant::k_invalidPrefabSceneInstanceNUM);

		l_gameObject->SetPrefabUUID({});

		l_prefabSystem.RemovePrefab(l_prefabUUID);

		a_assetRegistry.Erase(l_prefabFilePath);

		// SaveJsonFileの途中でFileだけ生成された場合も
		// 不完全なPrefabファイルを渡さない
		l_errorCode.clear();

		std::filesystem::remove(l_prefabFilePath, l_errorCode);

		FWK_ADD_LOG("Prefabファイルの保存に失敗したため、Prefab作成を取り消しました。");

		return {};
	}

	FWK_ADD_LOG("Prefabを作成しました。\nPrefabName : {}\nFilePath : {}\nPrefabUUID : {}\nPrefabInstanceNUM : {}",
		        l_prefabName,
		        l_prefabFilePath.string(),
		        boost::uuids::to_string(l_prefabUUID),
		        l_prefabSceneInstanceNUM);

	return l_prefabFilePath;
}

bool FWK::Editor::ContentBrowserEditorWindowFileSystem::DeleteFolder(const std::filesystem::path& a_folderPath, ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const
{
	if (a_folderPath.empty()) { return false; }

	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_folderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG("削除対象Fodlerが無効です。\nFolderPath : {}", a_folderPath.string());

		return false;
	}

	std::vector<std::filesystem::path> l_prefabFilePathList = {};

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
			const auto& l_assetUUID = a_assetRegistry.FindVALAssetUUID(l_filePath);

			if (!l_assetUUID.is_nil())
			{
				l_prefabFilePathList.emplace_back(l_filePath);
			}
		}

		l_folderEntryITR.increment(l_errorCode);

		if (l_errorCode)
		{
			FWK_ADD_LOG("削除対象フォルダーの走査中にエラーが発生しました。\nFolderPath : {}", a_folderPath.string());

			return false;
		}
	}

	// recursive_directory_iteratorで走査子ながらFileを削除すると、
	// Iteratorへ影響する可能性がある
	// そのためPath収集が完全に終わった後で削除する
	for (const auto& l_prefabFilePath : l_prefabFilePathList)
	{
		if (!DeletePrefabFile(l_prefabFilePath, a_assetRegistry))
		{
			FWK_ADD_LOG("フォルダー配下のPrefab削除に失敗したため、Folder削除を中止しました。\nPrefabFilePath : {}", l_prefabFilePath.string());

			return false;
		}
	}

	// Prefabファイルについては
	// 上ですでに正規のDeletePrefabFile(9を通している
	// PNG/FBX等の残りFileと子フォルダーはフォルダー削除の対象なのでまとめて削除する
	l_errorCode.clear();

	const auto& l_removedEntryCount = std::filesystem::remove_all(a_folderPath, l_errorCode);

	if (l_errorCode ||
		l_removedEntryCount == k_notRemovedEntryCount)
	{
		FWK_ADD_LOG("フォルダーを削除できませんでした。\nFolderPath : {}", a_folderPath.string());

		return false;
	}
	
	FWK_ADD_LOG("フォルダーを削除しました。\nFolderPath : {}", a_folderPath.string());

	return true;
}

bool FWK::Editor::ContentBrowserEditorWindowFileSystem::DeletePrefabFile(const std::filesystem::path& a_prefabFilePath, ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const
{
	if (a_prefabFilePath.empty()) { return false; }

	const auto l_prefabUUID = a_assetRegistry.FindVALAssetUUID(a_prefabFilePath);

	if (l_prefabUUID.is_nil()) 
	{
		FWK_ADD_LOG("削除対象PrefabのUUIDを取得できませんでした。\nFilePath : {}", a_prefabFilePath.string());

		return false;
	}

	// ファイル削除に失敗した場合にregistryを戻せるように
	// UUIDを取得した後でRegistryから削除する
	if (!a_assetRegistry.Erase(a_prefabFilePath))
	{
		FWK_ADD_LOG("PrefabをAssetRegistryから削除できませんでした。\nFilePath : {}", a_prefabFilePath.string());

		return false;
	}

	if (std::error_code l_errorCode = {};
		!std::filesystem::remove(a_prefabFilePath, l_errorCode) ||
		l_errorCode)
	{
		// 実ファイル削除に失敗したので
		// AssetRegistryを削除前の状態に戻す
		if (!a_assetRegistry.Add(l_prefabUUID, a_prefabFilePath))
		{
			FWK_ADD_LOG("Prefab削除失敗後のAssetRegistry復元にも失敗しました。\nFilePath : {}", a_prefabFilePath.string());
		}

		FWK_ADD_LOG("Prefabファイルを削除できませんでした。\nFilePath : {}", a_prefabFilePath.string());

		return false;
	}

	// シーンにある現在削除したプレハブのゲームオブジェクトをすべて削除する
	const auto& l_sceneManager   = SceneManager::GetInstance        ();
	const auto& l_scene          = l_sceneManager.GetVALScene().lock();
	const auto& l_gameObjectList = l_scene->GetREFGameObjectList    ();

	if (!l_scene) { return false; }

	for (const auto& l_gameObject : l_gameObjectList)
	{
		if (!l_gameObject ||
			l_gameObject->GetVALIsDestroyed())
		{
			continue;
		}

		// 削除したPrefabと関係ないGameObjectは何もしない
		if (l_gameObject->GetREFPrefabUUID() !=
			l_prefabUUID)
		{
			continue;
		}

		l_gameObject->Destroy();
	}

	l_scene->GetMutableREFPrefabSystem().RemovePrefab(l_prefabUUID);

	FWK_ADD_LOG("Prefabを削除しました。\nFilePath : {}\nPrefabUUID : {}", a_prefabFilePath.string(), boost::uuids::to_string(l_prefabUUID));

	return true;
}
bool FWK::Editor::ContentBrowserEditorWindowFileSystem::HasChildFolder(const std::filesystem::path& a_folderPath) const
{
	std::error_code                     l_errorCode         = {};
	std::filesystem::directory_iterator l_folderEntryITR    = { a_folderPath, l_errorCode };
	std::filesystem::directory_iterator l_endFolderEntryITR = {};

	if (l_errorCode) 
	{
		FWK_ADD_LOG("子フォルダーの確認に失敗しました。\nFolderPath : {}", a_folderPath.string());

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
			FWK_ADD_LOG("子フォルダーの走査中にエラーが発生しました。\nFolderPath : {}", a_folderPath.string());

			return false;
		}
	}

	return false;
}