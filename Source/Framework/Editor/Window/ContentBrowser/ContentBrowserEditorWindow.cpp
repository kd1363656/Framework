#include "ContentBrowserEditorWindow.h"

void FWK::Editor::ContentBrowserEditorWindow::CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_directoryPath)
{
	// 実際のPrefabファイルを作成しPrefabSystemへ登録する
	const auto& l_prefabFilePath = m_fileSystem.CreatePrefabFromGameObject(a_gameObject, a_directoryPath, m_assetRegistry);

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

void FWK::Editor::ContentBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::ContentBrowserEditorWindow::Draw()
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

	// 操作による遅延要求処理
	ApplySelectedEntryDeleteRequest();
	
	ImGui::End();
}

nlohmann::json FWK::Editor::ContentBrowserEditorWindow::Serialize()
{
	return m_jsonConverter.Serialize(*this);;
}

void FWK::Editor::ContentBrowserEditorWindow::RequestFolderCreate(const std::filesystem::path& a_parentFolderPath)
{

}

void FWK::Editor::ContentBrowserEditorWindow::RefreshCurrentFolderEntries()
{
	// currentFolder直下のFile/Forder一覧を再構築する
	m_entryController.RefreshCurrentFolderEntryList(m_assetRegistry, m_currentFolderPath);

	// Prefab作成直後など
	// Refresh後に選択したいEntryが指定されている場合だけ選択する
	if (m_requestedSelectEntryPath.empty()) { return; }

	m_entryController.SelectSingleEntry(m_requestedSelectEntryPath);
	m_requestedSelectEntryPath.clear   ();
}

void FWK::Editor::ContentBrowserEditorWindow::DrawFolderCreateEntry()
{
}

void FWK::Editor::ContentBrowserEditorWindow::ConfirmFolderCreate()
{
}

void FWK::Editor::ContentBrowserEditorWindow::CancelFolderCreate()
{
}

void FWK::Editor::ContentBrowserEditorWindow::ClearFolderCreateState()
{
}

void FWK::Editor::ContentBrowserEditorWindow::ApplyFolderCreateShortcut()
{

}

void FWK::Editor::ContentBrowserEditorWindow::ApplySelectedEntryDeleteRequest()
{

}
void FWK::Editor::ContentBrowserEditorWindow::ApplyFolderCreateRequest()
{

}
void FWK::Editor::ContentBrowserEditorWindow::ApplyCurrentFolderPath(const std::filesystem::path& a_folderPath)
{
	// FileをCurrentFolderとして設定することは許可しない(ディレクトリのみ)
	if (std::error_code l_errorCode = {}; !std::filesystem::is_directory(a_folderPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	// ".."や"."などを整理したPathへ統一する
	const auto& l_normalizedFolderPath = a_folderPath.lexically_normal();

	// 同じFolderを指定された場合は、
	// Entry一覧を不必要に再構築しない
	if (m_currentFolderPath == l_normalizedFolderPath) { return; }

	m_currentFolderPath = l_normalizedFolderPath;

	// CurrentFolder画変更されたので、
	// 次のCurrentFolder描画時にEntry一覧を再構築する
	m_entryController.SetCurrentFolderEntryListDirty(true);

	// 前FolderのSelectionを新しいFolderへ持ち越さない
	m_entryController.ClearSelectedEntries();

	// 前Folderで作成されたPrefabに対する
	// 選択要求も新しいFolderへ持ち越さない
	m_requestedSelectEntryPath.clear();
}

void FWK::Editor::ContentBrowserEditorWindow::RequestSelectedEntryDelete()
{
	m_isSelectedEntryDeleteRequested = true;
}

std::filesystem::path FWK::Editor::ContentBrowserEditorWindow::FetchVALFolderCreateParentPath() const
{
	return std::filesystem::path();
}