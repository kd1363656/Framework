#include "ContentBrowserEditorWindowFileSystem.h"

std::filesystem::path FWK::Editor::ContentBrowserEditorWindowFileSystem::CreateVALFolder(const std::filesystem::path& a_parentFolderPath) const
{
	std::error_code l_errorCode = {};

	// Folder作成先が実際に存在するFolderか確認する
	if (!std::filesystem::is_directory(a_parentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG("Folder作成先が無効です。\nFolderPath : {}", a_parentFolderPath);

		return {};
	}

	std::filesystem::path l_newFolderPath = a_parentFolderPath / k_newFolderName.data();

	auto l_folderNameIndex = k_firstFolderNameIndex;

	// NewFolderが既に存在する場合は
	// NewFolder_1, NewFolder_2の順に未使用名を探す
	while (true)
	{
		l_errorCode.clear();

		const bool l_isAlreadyExist = std::filesystem::exists(l_newFolderPath, l_errorCode);

		if (l_errorCode)
		{
			FWK_ADD_LOG("新しいFolder名を確認できませんでした。\nFolderPath : {}", l_newFolderPath);

			return {};
		}

		if (!l_isAlreadyExist) { break; }

		l_newFolderPath = a_parentFolderPath / std::format("{}_{}", k_newFolderName, l_folderNameIndex);

		++l_folderNameIndex;
	}

	l_errorCode.clear();

	if (!std::filesystem::create_directory(l_newFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG("Folderを作成できませんでした。\nFolderPath : {}", l_newFolderPath);

		return {};
	}

	FWK_ADD_LOG("Folderを作成しました。\nFolderPath : {}", l_newFolderPath);

	return l_newFolderPath;
}

bool FWK::Editor::ContentBrowserEditorWindowFileSystem::DeleteFolder(const std::filesystem::path& a_folderPath, ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const
{
	if (a_folderPath.empty()) { return false; }

	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_folderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG("削除対象Fodlerが無効です。\nFolderPath : {}", a_folderPath);

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
			FWK_ADD_LOG("削除対象Folderの走査中にエラーが発生しました。\nFolderPath : {}", a_folderPath);

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
			FWK_ADD_LOG("Folder配下のPrefab削除に失敗したため、Folder削除を中止しました。\nPrefabFilePath : {}", l_prefabFilePath);

			return false;
		}
	}

	// Prefabファイルについては
	// 上ですでに正規のDeletePrefabFile(9を通している
	// PNG/FBX等の残りFileと子FolderはFolder削除の対象なのでまとめて削除する
	l_errorCode.clear();

	const auto& l_removedEntryCount = std::filesystem::remove_all(a_folderPath, l_errorCode);

	if (l_errorCode ||
		l_removedEntryCount == k_notRemovedEntryCount)
	{
		FWK_ADD_LOG("Folderを削除できませんでした。\nFolderPath : {}", a_folderPath);

		return false;
	}
	
	FWK_ADD_LOG("Folderを削除しました。\nFolderPath : {}", a_folderPath);

	return true;
}
bool FWK::Editor::ContentBrowserEditorWindowFileSystem::DeletePrefabFile(const std::filesystem::path& a_prefabFilePath, ContentBrowserEditorWindowAssetRegistry& a_assetRegistry) const
{
	if (a_prefabFilePath.empty()) { return false; }

	const auto l_prefabUUID = a_assetRegistry.FindVALAssetUUID(a_prefabFilePath);

	if (l_prefabUUID.is_nil()) 
	{
		FWK_ADD_LOG("削除対象PrefabのUUIDを取得できませんでした。\nFilePath : {}", a_prefabFilePath);

		return false;
	}

	// ファイル削除に失敗した場合にregistryを戻せるように
	// UUIDを取得した後でRegistryから削除する
	if (!a_assetRegistry.Erase(a_prefabFilePath))
	{
		FWK_ADD_LOG("PrefabをAssetRegistryから削除できませんでした。\nFilePath : {}", a_prefabFilePath);

		return false;
	}

	std::error_code l_errorCode = {};

	if (!std::filesystem::remove(a_prefabFilePath, l_errorCode) ||
		l_errorCode)
	{
		// 実ファイル削除に失敗したので
		// AssetRegistryを削除前の状態に戻す
		if (!a_assetRegistry.Add(l_prefabUUID, a_prefabFilePath))
		{
			FWK_ADD_LOG("Prefab削除失敗後のAssetRegistry復元にも失敗しました。\nFilePath : {}", a_prefabFilePath);
		}

		FWK_ADD_LOG("Prefabファイルを削除できませんでした。\nFilePath : {}", a_prefabFilePath);

		return false;
	}

	// シーンにある現在削除したプレハブのゲームオブジェクトをすべて削除する
	      auto& l_sceneManager   = SceneManager::GetInstance        ();
	      auto& l_scene          = l_sceneManager.GetMutableREFScene();
	const auto& l_gameObjectList = l_scene.GetREFGameObjectList     ();

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

	l_scene.GetMutableREFPrefabSystem().RemovePrefab(l_prefabUUID);

	FWK_ADD_LOG(
		"Prefabを削除しました。\nFilePath : {}\nPrefabUUID : {}", a_prefabFilePath, boost::uuids::to_string(l_prefabUUID));

	return true;
}
bool FWK::Editor::ContentBrowserEditorWindowFileSystem::HasChildFolder(const std::filesystem::path& a_folderPath) const
{
	std::error_code                     l_errorCode         = {};
	std::filesystem::directory_iterator l_folderEntryITR    = { a_folderPath, l_errorCode };
	std::filesystem::directory_iterator l_endFolderEntryITR = {};

	if (l_errorCode) 
	{
		FWK_ADD_LOG("子Folderの確認に失敗しました。\nFolderPath : {}", a_folderPath.string());

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
			FWK_ADD_LOG("子Folderの走査中にエラーが発生しました。\nFolderPath : {}", a_folderPath.string());

			return false;
		}
	}

	return false;
}