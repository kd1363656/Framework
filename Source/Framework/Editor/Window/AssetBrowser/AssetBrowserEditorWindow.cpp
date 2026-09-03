#include "AssetBrowserEditorWindow.h"

void FWK::Editor::AssetBrowserEditorWindow::CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_directoryPath)
{
	// 実際のPrefabファイルを作成しPrefabSystemへ登録する
	const auto& l_prefabFilePath = m_fileSystem.CreatePrefabFromGameObject(a_gameObject, a_directoryPath, m_assetFilePathRegistry);

	if (l_prefabFilePath.empty()) { return; }

	// 左FolderTreeなど
	// 現在開いているFolderとは別FolderへPrefabを作成した場合は、
	// 現在右ペインのEntry一覧を更新する必要はない
	if (a_directoryPath != m_currentFolderPath) { return; }

	// CurrentFolder内に新しいPrefabFileが追加されたため
	// 次回DrawCurrentFolder()でEntry一覧を再構築する
	m_entryController.SetCurrentFolderEntryListDirty(true);

	// Refresh後に作成したPrefabを選択する
	m_requestedSelectEntryPath = l_prefabFilePath;
}
void FWK::Editor::AssetBrowserEditorWindow::CreateSceneFromScene(const std::weak_ptr<Scene>& a_scene, const std::filesystem::path& a_directoryPath)
{
	const auto& l_sceneFilePath = m_fileSystem.CreateSceneFromScene(a_scene, a_directoryPath, m_assetFilePathRegistry);

	if (l_sceneFilePath.empty()) { return; }

	// 現在開いているFolder以外へDropした場合は
	// 現在右側に表示しているEntryを更新する必要はない
	if (a_directoryPath != m_currentFolderPath) { return; }

	m_entryController.SetCurrentFolderEntryListDirty(true);

	m_requestedSelectEntryPath = l_sceneFilePath;
}

void FWK::Editor::AssetBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::AssetBrowserEditorWindow::Draw()
{
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	// 左ペイン
	m_panel.DrawFolderTree(*this);

	ImGui::SameLine();

	// 右ペイン
	m_panel.DrawCurrentFolder(*this);

	// Panel描画後にFileSystem変更を適用する
	ApplySelectedEntryDeleteRequest();
	ApplyFolderCreateRequest       ();

	ImGui::End();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindow::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::AssetBrowserEditorWindow::RequestFolderCreate(const std::filesystem::path& a_parentFolderPath)
{
	if (m_isFolderCreateActive) { return; }

	if (std::error_code l_errorCode = {};
		!std::filesystem::is_directory(a_parentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	m_folderCreateParentPath = a_parentFolderPath.lexically_normal();

	m_folderCreateNameBuffer.clear();

	m_isFolderCreateActive = true;

	// 次FrameでInputTextへKeyboardFocusを移す
	m_isFolderCreateInputFocusRequested = true;
}

void FWK::Editor::AssetBrowserEditorWindow::RefreshCurrentFolderEntries()
{
	// currentFolder直下のFile/Forder一覧を再構築する
	m_entryController.RefreshCurrentFolderEntryList(m_currentFolderPath);

	// Prefab作成直後など
	// Refresh後に選択したいEntryが指定されている場合だけ選択する
	if (m_requestedSelectEntryPath.empty()) { return; }

	m_entryController.SelectSingleEntry(m_requestedSelectEntryPath);
	m_requestedSelectEntryPath.clear   ();
}

void FWK::Editor::AssetBrowserEditorWindow::ApplyCurrentFolderPath(const std::filesystem::path & a_folderPath)
{
	if (!m_isFolderCreateRequested) { return; }

	if (m_folderCreateParentPath.empty() ||
		m_folderCreateNameBuffer.empty())
	{
		ClearFolderCreateState();

		return;
	}

	const auto& l_createdFolderPath = m_fileSystem.CreateFolder(m_folderCreateParentPath, m_folderCreateNameBuffer);

	if (l_createdFolderPath.empty())
	{
		ClearFolderCreateState();

		return;
	}

	// currentFolder直下へ作成した場合だけ
	// 右ペイン一覧を更新する
	if (m_folderCreateParentPath == m_currentFolderPath)
	{
		m_entryController.SetCurrentFolderEntryListDirty(true);

		m_requestedSelectEntryPath = l_createdFolderPath;
	}

	ClearFolderCreateState();
}

void FWK::Editor::AssetBrowserEditorWindow::RequestSelectedEntryDelete()
{
	m_isSelectedEntryDeleteRequested = true;
}

void FWK::Editor::AssetBrowserEditorWindow::ConfirmFolderCreate()
{
	if (!m_isFolderCreateActive ||
		m_folderCreateNameBuffer.empty())
	{
		return; 
	}

	// 実際のFilesystem変更はDraw途中で行わず遅延要求として保存
	m_isFolderCreateRequested           = true;
	m_isFolderCreateActive              = false;
	m_isFolderCreateInputFocusRequested = false;
}

void FWK::Editor::AssetBrowserEditorWindow::CancelFolderCreate()
{
	if (!m_isFolderCreateActive) { return; }

	ClearFolderCreateState();
}

void FWK::Editor::AssetBrowserEditorWindow::ClearFolderCreateState()
{
	m_folderCreateParentPath.clear();
	m_folderCreateNameBuffer.clear();

	m_isFolderCreateActive              = false;
	m_isFolderCreateInputFocusRequested = false;
	m_isFolderCreateRequested           = false;
}

void FWK::Editor::AssetBrowserEditorWindow::ApplySelectedEntryDeleteRequest()
{
	// 削除要求がなければreturn
	if (!m_isSelectedEntryDeleteRequested) { return; }

	// 削除要求はこのFrameで消費する
	// 途中で削除失敗した場合でも、
	// 次Frameで同じ削除処理を勝手に繰り返さない
	m_isSelectedEntryDeleteRequested = false;

	const auto& l_selectedEntryPathSet = m_entryController.GetREFSelectedEntryPathSet();

	if (l_selectedEntryPathSet.empty()) { return; }

	bool l_isAllDeleteSucceeded = true;

	// SelectionはこのForが終了するまで変更しない
	// unordered_setをconst参照で捜査しているため、
	// Path一覧のvectorコピー等も必要ない
	for (const auto& l_entryPath : l_selectedEntryPathSet)
	{
		      std::error_code l_errorCode = {};
		const bool            l_isFolder  = std::filesystem::is_directory(l_entryPath, l_errorCode);

		if (l_errorCode)
		{
			l_isAllDeleteSucceeded = false;

			continue;
		}

		// Folderの場合の削除処理
		if (l_isFolder)
		{
			// DeleteFolder()内部では
			// Folder配下にPrefabが存在した場合
			// DeletePrefabFile(9を通して
			// AssetRegistry,PrefabSystem,PrefabInstanceまで同期してからFolderを削除する
			if (!m_fileSystem.DeleteFolder(l_entryPath, m_assetFilePathRegistry))
			{
				l_isAllDeleteSucceeded = false;
			}

			continue;
		}

		// RegularFileの場合の削除処理
		l_errorCode.clear();

		if (const bool l_isRegularFile = std::filesystem::is_regular_file(l_entryPath, l_errorCode);
			l_errorCode ||
			!l_isRegularFile)
		{
			l_isAllDeleteSucceeded = false;

			continue;
		}

		// Prefab,RegularFile振り分け
		// 現在AssetRegistryへ登録しているFileはPrefabなので、
		// UUIDが存在するかどうかでPrefabを判定する
		// 無効値でなければプレハブ化されたファイルパスなので削除する
		if (const auto& l_prefabUUID = m_assetFilePathRegistry.FindPTRAssetUUID(l_entryPath);
			l_prefabUUID &&
			!l_prefabUUID->is_nil())
		{
			if (!m_fileSystem.DeletePrefabFile(l_entryPath, m_assetFilePathRegistry))
			{
				l_isAllDeleteSucceeded = false;
			}

			continue;
		}

		// Prefabではなく、SceneとしてAssetRegistryへ登録されているか確認する
		if (const auto& l_sceneUUID = m_assetFilePathRegistry.FindPTRAssetUUID(l_entryPath);
			l_sceneUUID &&
			!l_sceneUUID->is_nil())
		{
			if (!m_fileSystem.DeleteSceneFile(l_entryPath, m_assetFilePathRegistry))
			{
				l_isAllDeleteSucceeded = false;
			}

			continue;
		}

		// Registryへ登録されていない
		// FBX/PNG等の通常Fileはこちらで削除する
		if (!m_fileSystem.DeleteRegularFile(l_entryPath))
		{
			l_isAllDeleteSucceeded = false;
		}
	}

	// 削除対象PathをSelectionとして保持し続けない
	m_entryController.ClearSelectedEntries();

	// 一部駆除失敗の場合も
	// 実FileSystem状態を再取得する必要があるためDirtyにする
	m_entryController.SetCurrentFolderEntryListDirty(true);

	if (!l_isAllDeleteSucceeded)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "選択Entryの一部を削除できませんでした。");

		return;
	}

	FWK_ADD_LOG(Constant::k_debugSuccessColor, "選択Entryを削除しました。");
}
void FWK::Editor::AssetBrowserEditorWindow::ApplyFolderCreateRequest()
{
	if (!m_isFolderCreateRequested) { return; }

	if (m_folderCreateParentPath.empty() ||
		m_folderCreateNameBuffer.empty())
	{
		ClearFolderCreateState();

		return;
	}

	const auto& l_createdFolderPath = m_fileSystem.CreateFolder(m_folderCreateParentPath, m_folderCreateNameBuffer);

	if (l_createdFolderPath.empty())
	{
		ClearFolderCreateState();

		return;
	}

	// currentFolder直下へ作成した場合だけ
	// 右ペイン一覧を更新する
	if (m_folderCreateParentPath == m_currentFolderPath)
	{
		m_entryController.SetCurrentFolderEntryListDirty(true);

		m_requestedSelectEntryPath = l_createdFolderPath;
	}

	ClearFolderCreateState();
}