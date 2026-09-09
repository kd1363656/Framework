
#include "AssetBrowserEditorWindow.h"

void FWK::Editor::AssetBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Editor::AssetBrowserEditorWindow::PostDeserialize()
{
	RefreshFolderTree();

	// AssetRootが正常に取得できた場合だけ、
	// WindowのDirectory変更監視を開始する
	if (!m_rootFolderTreeData.m_folderPath.empty())
	{
		m_directoryWatcher.Prepare(Constant::k_assetRootFolderPath);
	}
}

void FWK::Editor::AssetBrowserEditorWindow::Draw()
{
	// DirectoryWatcherはWindowが折りたたまれている場合でも同期する
	if (auto& l_sceneManager = SceneManager::GetInstance();
		m_directoryWatcher.Synchronize(m_assetFilePathRegistry, l_sceneManager))
	{
		// File / Directory構造田変更された場合だけ
		// 左右Pane用の情報を更新する
		RefreshFolderTree();
	}

	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

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

	// AssetRootが実際にDirectoryとして存在するか確認する
	if (!std::filesystem::is_directory(Constant::k_assetRootFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetBrowserでAsset Root Folderを読み込めませんでした。\nFolderPath : {}", Constant::k_assetRootFolderPath.string());

		return;
	}

	// RootFolder自身をTreeの先頭として登録する
	m_rootFolderTreeData.m_folderPath = Constant::k_assetRootFolderPath;

	// Asset配下のDirectoryだけを再帰収集する
	BuildFolderTreeData(m_rootFolderTreeData);

	l_errorCode.clear();

	// 現在右Paneで開いているDirectoryが、
	// Explorer側で削除・移動されて存在しなくなった場合は
	// AssetRootへ戻す
	if (!std::filesystem::is_directory(m_currentDirectoryPath, l_errorCode) ||
		l_errorCode)
	{
		m_currentDirectoryPath = Constant::k_assetRootFolderPath;
	}
}

void FWK::Editor::AssetBrowserEditorWindow::RefreshCurrentDirectoryEntryDataList()
{
	
}

void FWK::Editor::AssetBrowserEditorWindow::BuildFolderTreeData(FolderTreeData& a_folderTreeData)
{
	std::error_code l_errorCode = {};

	// directory_iteratorは指定Directory直下のEntryだけを列挙する
	// skip_permission_deniedを指定することで、
	// Access権のないDirectoryが存在しても
	// Editor全体を例外終了させない
	auto l_directoryITR = std::filesystem::directory_iterator{ a_folderTreeData.m_folderPath, std::filesystem::directory_options::skip_permission_denied, l_errorCode };

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
			        "AssetBrowserでDirectoryを読み込めませんでした。\nDirectoryPath : {}\nErrorCode : {}",
			        a_folderTreeData.m_folderPath.string(),
			        l_errorCode.value());

		return;
	}

	const std::filesystem::directory_iterator l_directoryEndITR = {};

	// Directory数は事前にわからないため
	// IteratorがEndへ到達するまで走査する
	while (l_directoryITR != l_directoryEndITR)
	{
		l_errorCode.clear();

		// Fileは左側FolderTreeへ表示しない
		// Directoryだけを対象にする
		if (const auto& l_directoryEntry = *l_directoryITR;
			l_directoryEntry.is_directory(l_errorCode) &&
			!l_errorCode)
		{
			FolderTreeData l_childFolderTreeData = {};

			l_childFolderTreeData.m_folderPath = l_directoryEntry.path();

			// ChildDirectoryも同じ方法で再帰取得する
			BuildFolderTreeData(l_childFolderTreeData);

			a_folderTreeData.m_childFolderDataList.emplace_back(std::move(l_childFolderTreeData));
		}

		l_errorCode.clear();

		// error_code版increment()を使用することで
		// Directory走査中のFileSystemErrorを例外にしない
		l_directoryITR.increment(l_errorCode);

		if (l_errorCode)
		{
			FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
				        "AssetBrowserでDirectory走査中にErrorが発生しました。\nDirectoryPath : {}\nErrorCode : {}", 
				        a_folderTreeData.m_folderPath.string(), 
				        l_errorCode.value());

			return;
		}
	}

	// FileSystemが返す順番へ依存すると
	// 起動ごとにFolder表示順が変わる可能性がある
	// FolderPath順へ並べて表示順を安定させる
	std::ranges::sort(a_folderTreeData.m_childFolderDataList, {}, &FolderTreeData::m_folderPath);
}

void FWK::Editor::AssetBrowserEditorWindow::DrawFolderPane()
{
	// ImGui::BeginChild()は
	// 現在のWindowの内部にもう一つの描画領域を作成するAPI
	if (const ImVec2 l_folderPaneSize = { k_folderPaneWidth, k_fileRemainingArea };
		!ImGui::BeginChild(k_folderPaneChildLabel.data(), l_folderPaneSize, true))
	{
		// BeginChild()もBegin()と同様
		// 呼びだした場合は必ずEndChild()と組み合わせる
		ImGui::EndChild();

		return;
	}

	ImGui::TextUnformatted(k_folderPaneTitle.data());
	ImGui::Separator      ();

	// AssetRoot事態を取得できなかった場合はTreeを描画できない
	if (m_rootFolderTreeData.m_folderPath.empty())
	{
		ImGui::TextDisabled(k_assetRootUnavailableLabel.data());

		ImGui::EndChild();

		return;
	}

	DrawFolderTree(m_rootFolderTreeData);

	ImGui::EndChild();
}
void FWK::Editor::AssetBrowserEditorWindow::DrawFolderTree(const FolderTreeData& a_folderTreeData)
{
	const bool l_hasChildFolder = !a_folderTreeData.m_childFolderDataList.empty();

	ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
		                                 ImGuiTreeNodeFlags_OpenOnArrow    |
		                                 ImGuiTreeNodeFlags_OpenOnDoubleClick;

	// 現在右Paneで開いているDirectoryには
	// ImGui標準の選択色をつける
	if (a_folderTreeData.m_folderPath == m_currentDirectoryPath)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (!l_hasChildFolder)
	{
		// ChildFolderが存在しないDirectoryはLeafノードとして表示する
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
			               ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	// WindowsのFilesystem::pathはwstringを使用する場合があるため、
	// UTF-16 -> UTF-8へ変換してImGuiへ戻す
	const auto& l_folderName  = Utility::WStringToString(a_folderTreeData.m_folderPath.filename().wstring());
	const auto& l_folderLabel = std::format             ("{} {}", Constant::k_imguiFontAwesomeFolderIcon, l_folderName);

	// 同じFolder名が別階層に存在してもImGuiIDが衝突しないようにFilePathをIDとして使用する
	const auto& l_folderPathString = Utility::WStringToString(a_folderTreeData.m_folderPath.wstring());

	ImGui::PushID(l_folderPathString.c_str());

	const bool l_isTreeNodeOpen = ImGui::TreeNodeEx(l_folderLabel.c_str(), l_treeNodeFlags);

	// Folder部分をClickした場合、
	// 右Paneで表示するDirectoryを変更する
	// Arrowを押してTreeを開閉しただけの場合は
	// CurrentDirectoryを変更しない
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) &&
		!ImGui::IsItemToggledOpen())
	{
		m_currentDirectoryPath = a_folderTreeData.m_folderPath;
	}

	// LeafではNoTreePushOnOpenを指定しているため、
	// TreeNodeEx()によるTreePushが発生しない
	// そのためTreePop(9を読んではいけない
	if (!l_hasChildFolder)
	{
		ImGui::PopID();

		return;
	}

	// Treeが閉じている場合はChildFolderを描画しない
	if (!l_isTreeNodeOpen)
	{
		ImGui::PopID();

		return;
	}

	for (const auto& l_childFolderData : a_folderTreeData.m_childFolderDataList)
	{
		DrawFolderTree(l_childFolderData);
	}

	ImGui::TreePop();
	ImGui::PopID  ();

}
void FWK::Editor::AssetBrowserEditorWindow::DrawAssetPane() const
{
	// X = 0.0F
	// Y = 0.0F
	// とすることで、左Paneを配置した後に残っている横幅と高さを全て右Paneが使用する
	if (const ImVec2 l_assetPaneSize = { k_fileRemainingArea, k_fileRemainingArea };
		!ImGui::BeginChild(k_assetPaneChildLabel.data(), l_assetPaneSize, true))
	{
		ImGui::EndChild();

		return;
	}

	ImGui::TextUnformatted("アセット");
	ImGui::EndChild();
}

bool FWK::Editor::AssetBrowserEditorWindow::DrawAssetEntryCard() const
{


	return false;
}