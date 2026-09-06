
#include "AssetBrowserEditorWindow.h"

void FWK::Editor::AssetBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Editor::AssetBrowserEditorWindow::PostDeserialize()
{
	RefreshFolderTree();

	// AssetRootFolder以下のFileSystem変更監視を開始する
	m_directoryWatcher.Prepare(Constant::k_assetRootFolderPath);
}

void FWK::Editor::AssetBrowserEditorWindow::Draw()
{
	auto& l_sceneManager = SceneManager::GetInstance();

	// DirectoryWatcherは毎フレーム
	// WindowsEventの状態を0msで確認する
	// 変更がない場合は即Returnするため通常Frameでは非常に軽い
	// FolderのAdd / Delete / Rename / Moveが発生した場合、
	// FolderTreeを現在のFilesystem状態から更新する
	if (m_directoryWatcher.Synchronize(m_assetFilePathRegistry, l_sceneManager))
	{
		RefreshFolderTree();
	}

	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	// 左ペイン、フォルダのみを表示する
	// 左クリックで右ペインの開いているフォルダを変更したりできる
	DrawFolderPane ();
	ImGui::SameLine();

	// 左ペインと同じ高さで右ペインを描画する
	// 右ペインではファイルを見ることなどができる
	DrawAssetPane();
	ImGui::End   ();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindow::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::AssetBrowserEditorWindow::RefreshFolderTree()
{
	// RefreshFolderTree()が何度も呼ばれた場合に備えて
	// 前回作成したTreeを最初に破棄する
	m_rootFolderTreeData = {};

	std::error_code l_errorCode = {};

	// is_directory()は指定したPathが実際にDirectoryであるかを調べる
	// Editorでは外部からAssetFolderが削除されたり
	// 一時的に悪背できなくなる可能性もあるため
	// FileSystem走査失敗だけ例外を発生させないようにする
	if (const bool l_isAssetRootDirectory = std::filesystem::is_directory(Constant::k_assetRootFolderPath, l_errorCode);
		l_errorCode ||
		!l_isAssetRootDirectory)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetBrowserでAsset Root Folderを読み込めませんでした。FolderPath : {}", Constant::k_assetRootFolderPath.string());

		return;
	}
}

void FWK::Editor::AssetBrowserEditorWindow::DrawFolderPane()
{
	const ImVec2 l_folderPaneSize = { k_folderPaneWidth, k_fileRemainingArea };

	// ImGui::BeginChild()は
	// 現在のWindowの内部にもう一つの描画領域を作成するAPI
	if (!ImGui::BeginChild(k_folderPaneChildLabel.data(), l_folderPaneSize, true))
	{
		// BeginChild()もBegin()と同様
		// 呼びだした場合は必ずEndChild()と組み合わせる
		ImGui::EndChild();

		return;
	}

	ImGui::TextUnformatted("フォルダ");
	ImGui::EndChild();
}
void FWK::Editor::AssetBrowserEditorWindow::DrawFolderTree(const FolderTreeData& a_folderTreeData, bool a_isRootFolder)
{
	
}
void FWK::Editor::AssetBrowserEditorWindow::DrawAssetPane() const
{
	// X = 0.0F
	// Y = 0.0F
	// とすることで、左Paneを配置した後に残っている横幅と高さを全て右Paneが使用する
	const ImVec2 l_assetPaneSize = { k_fileRemainingArea, k_fileRemainingArea };

	if (!ImGui::BeginChild(k_assetPaneChildLabel.data(), l_assetPaneSize, true))
	{
		ImGui::EndChild();

		return;
	}

	ImGui::TextUnformatted("アセット");
	ImGui::EndChild();
}