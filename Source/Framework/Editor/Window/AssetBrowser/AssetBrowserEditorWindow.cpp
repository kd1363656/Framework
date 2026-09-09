
#include "AssetBrowserEditorWindow.h"

void FWK::Editor::AssetBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Editor::AssetBrowserEditorWindow::PostDeserialize()
{
	RefreshAssetEntryTree();

	// AssetRootを正常にTreeへ登録できた場合だけ、
	// DirectoryWatcherを開始する
	if (!m_rootAssetEntryData.m_filePath.empty())
	{
		m_directoryWatcher.Prepare(Constant::k_assetRootFolderPath);
	}
}

void FWK::Editor::AssetBrowserEditorWindow::Draw()
{
	// File / Directoryに変更があった場合だけ
	// AssetEntryTreeを再構する
	if (auto& l_sceneManager = SceneManager::GetInstance();
		m_directoryWatcher.Synchronize(m_assetFilePathRegistry, l_sceneManager))
	{
		RefreshAssetEntryTree();
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


void FWK::Editor::AssetBrowserEditorWindow::RefreshAssetEntryTree()
{
	// 前回構築したFile / DirectoryTreeを破棄する
	m_rootAssetEntryData = {};

	std::error_code l_errorCode = {};

	// AssetRootが実際にDirectoryとして存在するか確認する
	if (!std::filesystem::is_directory(Constant::k_assetRootFolderPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetBrowserでAssetRootFolderを読み込めませんでした。\nFolderPath : {}", Constant::k_assetRootFolderPath.string());

		return;
	}

	// RootはAssetDirectoryそのもの
	m_rootAssetEntryData.m_filePath    = Constant::k_assetRootFolderPath;
	m_rootAssetEntryData.m_isDirectory = true;

	// Asset配下のDirectory / Fileを1つのTreeへまとめる
	BuildAssetEntryDataTree(m_rootAssetEntryData);

	l_errorCode.clear();

	// 現在開いているDirectoryが削除、移動された場合だけ
	// AssetRootへ戻す
	if (!std::filesystem::is_directory(m_currentDirectoryPath, l_errorCode) ||
		l_errorCode)
	{
		m_currentDirectoryPath = Constant::k_assetRootFolderPath;
	}
}

void FWK::Editor::AssetBrowserEditorWindow::BuildAssetEntryDataTree(AssetEntryData& a_assetEntryData)
{
	// FileはChildを持てないので
	// Directoryだけを再帰操作する
	if (!a_assetEntryData.m_isDirectory) { return; }

	std::error_code l_errorCode = {};


	auto l_directoryITR = std::filesystem::directory_iterator{ a_assetEntryData.m_filePath, std::filesystem::directory_options::skip_permission_denied, l_errorCode };

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
			        "AssetBrowserでDirectoryを読み込めませんでした。\nDirectoryPath : {}\nErrorCode : {}",
			        a_assetEntryData.m_filePath.string(),
			        l_errorCode.value());

		return;
	}

	const std::filesystem::directory_iterator l_directoryEndITR = {};

	// Directory数は事前にわからないため
	// IteratorがEndへ到達するまで走査する
	while (l_directoryITR != l_directoryEndITR)
	{
		l_errorCode.clear();

		const auto& l_directoryEntry = *l_directoryITR;

		AssetEntryData l_childAssetEntryData = {};

		l_childAssetEntryData.m_filePath    = l_directoryEntry.path        ();
		l_childAssetEntryData.m_isDirectory = l_directoryEntry.is_directory(l_errorCode);

		if (!l_errorCode)
		{
			// Directoryだった場合だけ
			if (l_childAssetEntryData.m_isDirectory)
			{
				BuildAssetEntryDataTree(l_childAssetEntryData);
			}

			// FileでもDirectoryでも同じChild配列へ追加する
			auto& l_childAssetEntryDataList = a_assetEntryData.m_childAssetEntryDataList;

			l_childAssetEntryDataList.emplace_back(std::move(l_childAssetEntryData));
		}

		l_errorCode.clear();

		l_directoryITR.increment(l_errorCode);

		if (l_errorCode)
		{
			FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
			            "AssetBrowserでDirectory走査中にErrorが発生しました。\nDirectoryPath : {}\nErrorCode : {}",
			            a_assetEntryData.m_filePath.string(),
			            l_errorCode.value());

			return;
		}
	}

	// FileSystemが返す順番へ依存すると
	// 起動ごとに表示順が変わる可能性がある
	// FolderPath順へ並べて表示順を安定させる
	std::ranges::sort(a_assetEntryData.m_childAssetEntryDataList, CompareAssetEntryData);
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

	ImGui::TextUnformatted(k_folderPaneTitleLabel.data());
	ImGui::Separator      ();

	// AssetRoot事態を取得できなかった場合はTreeを描画できない
	if (m_rootAssetEntryData.m_filePath.empty())
	{
		ImGui::TextDisabled(k_assetRootUnavailableLabel.data());

		ImGui::EndChild();

		return;
	}

	DrawFolderTree(m_rootAssetEntryData);

	ImGui::EndChild();
}
void FWK::Editor::AssetBrowserEditorWindow::DrawFolderTree(const AssetEntryData& a_assetEntryData)
{
	// 左PaneではFileは描画しない
	if (!a_assetEntryData.m_isDirectory) { return; }

	const bool l_hasChildDirectory = std::ranges::any_of(a_assetEntryData.m_childAssetEntryDataList,
		                                                 [](const AssetEntryData& a_childAssetEntryData) 
		                                                 {
																return a_childAssetEntryData.m_isDirectory;
		                                                 });

	ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
		                                 ImGuiTreeNodeFlags_OpenOnArrow    |
		                                 ImGuiTreeNodeFlags_OpenOnDoubleClick;

	// 現在右Paneで開いているDirectoryには
	// ImGui標準の選択色をつける
	if (a_assetEntryData.m_filePath == m_currentDirectoryPath)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (!l_hasChildDirectory)
	{
		// ChildFolderが存在しないDirectoryはLeafノードとして表示する
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
			               ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	// WindowsのFilesystem::pathはwstringを使用する場合があるため、
	// UTF-16 -> UTF-8へ変換してImGuiへ戻す
	const auto& l_folderName  = Utility::WStringToString(a_assetEntryData.m_filePath.filename().wstring());
	const auto& l_folderLabel = std::format             ("{} {}", Constant::k_imguiFontAwesomeFolderIcon, l_folderName);

	// 同じFolder名が別階層に存在してもImGuiIDが衝突しないようにFilePathをIDとして使用する
	const auto& l_folderPathString = Utility::WStringToString(a_assetEntryData.m_filePath.wstring());

	ImGui::PushID(l_folderPathString.c_str());

	const bool l_isTreeNodeOpen = ImGui::TreeNodeEx(l_folderLabel.c_str(), l_treeNodeFlags);

	// Folder名をダブルクリックした場合だけ、
	// そのDirectoryを右Paneで開いているDirectoryとして扱う
	// ArrowクリックではCurrentDirectoryを変更しない
	if (ImGui::IsItemHovered() &&
		ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		m_currentDirectoryPath = a_assetEntryData.m_filePath;
	}

	// LeafではNoTreePushOnOpenを指定しているため、
	// TreeNodeEx()によるTreePushが発生しない
	// そのためTreePop(9を読んではいけない
	if (!l_hasChildDirectory)
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

	for (const auto& l_childAssetEntryData : a_assetEntryData.m_childAssetEntryDataList)
	{
		if (!l_childAssetEntryData.m_isDirectory) { continue; }

		DrawFolderTree(l_childAssetEntryData);
	}

	ImGui::TreePop();
	ImGui::PopID  ();
}
void FWK::Editor::AssetBrowserEditorWindow::DrawAssetPane()
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

	ImGui::TextUnformatted(k_assetPaneTitleLabel.data());
	ImGui::Separator      ();

	if (!DrawCurrentDirectoryAssetEntryList(m_rootAssetEntryData))
	{
		ImGui::TextDisabled(k_assetRootUnavailableLabel.data());
	}

	ImGui::EndChild();
}

bool FWK::Editor::AssetBrowserEditorWindow::DrawCurrentDirectoryAssetEntryList(const AssetEntryData& a_assetEntryData)
{
	// FileはDirectoryとして開くことができない
	if (!a_assetEntryData.m_isDirectory) { return false; }

	// CurrentDirectoryを発見した
	if (a_assetEntryData.m_filePath == m_currentDirectoryPath)
	{
		const auto& l_childAssetEntryDataList = a_assetEntryData.m_childAssetEntryDataList;

		// 子ファイルパスが存在しなければディレクトリのみ描画する
		if (l_childAssetEntryDataList.empty())
		{
			ImGui::TextDisabled(k_emptyDirectoryLabel.data());

			return true;
		}

		// 右Paneの現在利用可能な横幅を取得する
		const float l_availableWidth = ImGui::GetContentRegionAvail().x;

		// Card一枚分 + Card同士の間隔
		const float l_assetCardPitch = k_assetCardSize.x + k_assetCardSpacing;
		
		const auto& l_calculatedColumnCount = static_cast<std::size_t>((l_availableWidth + k_assetCardSpacing) / l_assetCardPitch);

		// WindowsがCard1毎より狭い場合
		// 計算結果が0列になる可能性がある
		// 0列では剰余計算などが成立しないため
		// 最低でも1列になるよう保証する
		const auto& l_columnCount = std::max(l_calculatedColumnCount, k_minAssetCardColumnCount);

		// DirectoryCardをダブルクリックした場合の移動先
		// Card一覧を描画している途中でCurrentDirectoryを変更せず
		// すべてのCardを描画した後に反映する
		std::filesystem::path l_nextDirectoryPath = {};

		// CurrentDirectory直下のFile / Directoryを先頭から順番にCardとして描画する
		// 先頭から順番にCardとして
		for (std::size_t l_index = 0ULL; l_index < l_childAssetEntryDataList.size(); ++l_index)
		{
			// DirectoryCardをダブルクリックした場合だけtrueを返す
			if (const auto& l_childAssetEntryData = l_childAssetEntryDataList[l_index];
				DrawAssetEntryCard(l_childAssetEntryData))
			{
				l_nextDirectoryPath = l_childAssetEntryData.m_filePath;
			}

			// 現在のCardの次に存在するEntryのIndex
			const auto& l_nextIndex = l_index + k_nextAssetEntryIndexOffset;

			if (l_nextIndex >= l_childAssetEntryDataList.size()) { continue; }

			// 次のIndexがColumn数で割り切れる場合、
			// 現在のCardがそのDrawの最後のCardということになる
			// 例 : 
			// ColumnCount = 3
			// 0 1 2
			// 3 4 5
			// といった並びになる
			if (l_nextIndex % l_columnCount == k_assetCardRowEndRemainder) { continue; }

			// Row最後ではないので
			// 次のCardを現在のCardの右側へ配置する
			ImGui::SameLine(k_assetCardsSameLineOffsetX, k_assetCardSpacing);
		}

		// DirectoryCardがダブルクリックされていた場合は、
		// Card一覧の描画が終わった後でCurrentDirectoryを変更する
		if (!l_nextDirectoryPath.empty())
		{
			m_currentDirectoryPath = std::move(l_nextDirectoryPath);
		}

		return true;
	}

	// このAssetEntryData自身がCurrentDirectoryではなかったので
	// ChildDirectoryだけを再帰的に探索する
	// FileはCurrentDirectoryにはなれないため探索対象外
	for (const auto& l_childAssetEntryData : a_assetEntryData.m_childAssetEntryDataList) 
	{
		if (!l_childAssetEntryData.m_isDirectory) { continue; }

		// Child以下でCurrentDirectoryが見つかった場合は
		// それ以降のSiblingDirectoryを探索する必要がない
		if (DrawCurrentDirectoryAssetEntryList(l_childAssetEntryData)) { return true; }
	}

	// このAssetEntryData以下には
	// CurrentDirectoryが存在しなかった
	return false;
}

bool FWK::Editor::AssetBrowserEditorWindow::DrawAssetEntryCard(const AssetEntryData& a_assetEntryData) const
{
	return false;
}

bool FWK::Editor::AssetBrowserEditorWindow::CompareAssetEntryData(const AssetEntryData& a_leftAssetEntryData, const AssetEntryData& a_rightAssetEntryData)
{
	// DirectoryとFileならDirectoryを前へ並べる
	if (a_leftAssetEntryData.m_isDirectory !=
		a_rightAssetEntryData.m_isDirectory)
	{
		return a_leftAssetEntryData.m_isDirectory;
	}

	// 同じ種類同士ならPath順へ並べる
	return a_leftAssetEntryData.m_filePath < a_rightAssetEntryData.m_filePath;
}