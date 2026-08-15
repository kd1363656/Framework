#include "ContentBrowserEditorWindow.h"

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
	if (const ImVec2 l_folderTreePaneSize = { k_folderTreePanelWidth, k_filleRemainingSize };
		ImGui::BeginChild(k_folderTreeChildLabel.data(), l_folderTreePaneSize, true))
	{
		DrawFolderTree();
	}
	
	ImGui::EndChild();
	ImGui::SameLine();

	// 右ペイン
	if (const ImVec2 l_currentFolderPaneSize = { k_filleRemainingSize, k_filleRemainingSize };
		ImGui::BeginChild(k_currentFolderChildLabel.data(), l_currentFolderPaneSize, true))
	{
		DrawCurrentFolder();
	}

	ImGui::EndChild();

	// EndChild()した直後は右currentFolderのChildWindow全体がImGuiの直前Itemとして扱われる
	// そのため右ペインの空白部分へGameObjectをDropした場合も、
	// 現在開いているFolderへPrefabを作成できる
	DrawGameObjectPrefabDragDropTarget(m_currentFolderPath);

	ImGui::End();
}

nlohmann::json FWK::Editor::ContentBrowserEditorWindow::Serialize()
{
	return m_jsonConverter.Serialize(*this);;
}

void FWK::Editor::ContentBrowserEditorWindow::DrawFolderTree()
{
	std::error_code l_errorCode = {};

	// ContentBrowserのRootであるContent自体が存在しなければ
	// FolderTreeは作成できない
	if (!std::filesystem::exists(k_contentRootFolderPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	l_errorCode.clear();

	if (!std::filesystem::is_directory(k_contentRootFolderPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	// RootであるContentからFolderTreeでの描画を開始する
	DrawFolderTreeNode(k_contentRootFolderPath);
}
void FWK::Editor::ContentBrowserEditorWindow::DrawFolderTreeNode(const std::filesystem::path & a_folderPath)
{
	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_folderPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	const bool l_hasChildFolder = m_fileSystem.HasChildFolder(a_folderPath);

	// 矢印部分をクリックすると開閉できるようにする
	ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
		                                 ImGuiTreeNodeFlags_SpanAvailWidth;

	// 現在右ペインで開いているフォルダを
	// 右側Treeでも選択状態として視覚化する
	if (m_currentFolderPath == a_folderPath)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	// 子Folderを持たないフォルダには展開用のArrowを表示しない
	if (!l_hasChildFolder)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
		l_treeNodeFlags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	// ContentBrowserを初めて開いた時だけ
	// RootであるContentを開いた状態にする
	// ImGuiCond_Onceなので、ユーザが後からContentを閉じれば閉じたままになる
	if (a_folderPath == k_contentRootFolderPath)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	}

	// TreeNodeのIDにはフォルダ全体のPathを使用する
	// 例えば
	// Content/Model/Actor
	// Content/Texture/Actor
	// のように同じ名前のフォルダが別階層にあってる
	// ImGui上では別Nodeとして扱える
	const auto l_folderPathString = a_folderPath.generic_string();
	      auto l_folderName       = a_folderPath.filename      ().string();

	// 通常Content出は発生しないが
	// filename()が空だった場合の最低限のか表示
	if (l_folderName.empty())
	{
		l_folderName = l_folderPathString;
	}

	const bool l_isNodeOpen = ImGui::TreeNodeEx(l_folderPathString.c_str(),
		                                        l_treeNodeFlags,
		                                        "%s %s", 
		                                        Constant::k_fontAwesomeFolderIcon.data(),
		                                        l_folderName.c_str());

	// 左側フォルダツリー側もPrefab保存先をしてできるようにする
	DrawGameObjectPrefabDragDropTarget(a_folderPath);

	// フォルダの行をClickした場合は、
	// そのFolderを右ペインで開く
	// ArrowをクリックしてTreeNodeを開閉しただけの場合は
	// CurrentFolderを変更しない
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) &&
		!ImGui::IsItemToggledOpen())
	{
		ApplyCurrentFolderPath(a_folderPath);
	}

	// フォルダーが閉じている場合は
	// それより下の階層は一切走査しない
	if (!l_isNodeOpen ||
		!l_hasChildFolder)
	{
		return;
	}

	// 現在Nodeが開いているフォルダの
	// 「直下」だけを調べる
	std::filesystem::directory_iterator l_directoryITR    = { a_folderPath, l_errorCode };
	std::filesystem::directory_iterator l_endDirectoryITR = {};

	if (l_errorCode)
	{
		ImGui::TreePop();

		return;
	}

	while (l_directoryITR != l_endDirectoryITR)
	{
		std::error_code l_entryErrorCode = {};

		// 左ペインではFileを描画しない
		// フォルダだけを再帰的にTreeへ追加する
		if (l_directoryITR->is_directory(l_entryErrorCode) &&
			!l_entryErrorCode)
		{
			DrawFolderTreeNode(l_directoryITR->path());
		}

		l_directoryITR.increment(l_errorCode);

		if (l_errorCode) { break; }
	}

	ImGui::TreePop();
}
void FWK::Editor::ContentBrowserEditorWindow::DrawCurrentFolder()
{
	std::error_code l_errorCode = {};

	// Explorer等から現在開いているフォルダを削除された場合などに
	// 無効Pathを保持し続けないようにContentへ戻す
	if(!std::filesystem::is_directory(m_currentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		ApplyCurrentFolderPath(k_contentRootFolderPath);

		l_errorCode.clear();
	}

	// Content事態まで存在しない場合は表示できない
	if (!std::filesystem::is_directory(m_currentFolderPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	// 現在の右ペインで使用可能な横幅
	const float l_availableWidth = ImGui::GetContentRegionAvail().x;

	// Cardと次のCardの間隔には
	// Editor全体のImTuiItemSpacingをそのまま使用する
	const float l_itemSpacing      = ImGui::GetStyle().ItemSpacing.x;
	const float l_folderEntryPitch = k_folderEntryWidth + l_itemSpacing;

	// ペインが広ければColumn数を増やし
	// 狭ければ自動的に少なくする
	const std::uint32_t l_calculatedColumnCount = static_cast<std::uint32_t>(l_availableWidth + l_itemSpacing) / static_cast<std::uint32_t>(l_folderEntryPitch);
	const std::uint32_t l_columnCount           = std::max                  (k_minFolderEntryColumnCount, l_calculatedColumnCount);

	auto l_currentColumn = k_initialFolderEntryColumnCount;

	// DoubleClickされた瞬間にm_currentFolderPathを置き換えず、
	// 現在フォルダの描画が全部終わってから反映する
	      std::filesystem::path               l_requestedDirectoryPath = {};
	      std::filesystem::directory_iterator l_directoryITR           = { m_currentFolderPath, l_errorCode };
	const std::filesystem::directory_iterator l_endDirectoryITR        = {};

	if (l_errorCode) { return; }

	while (l_directoryITR != l_endDirectoryITR)
	{
		std::error_code l_entryErrorCode = {};

		const bool l_isDirectory = l_directoryITR->is_directory(l_entryErrorCode);

		if (!l_entryErrorCode)
		{
			const auto& l_entryPath = l_directoryITR->path();

			DrawFolderEntry(l_entryPath, l_isDirectory);

			if (l_isDirectory          &&
				ImGui::IsItemHovered() &&
				ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				l_requestedDirectoryPath = l_entryPath;
			}

			++l_currentColumn;

			if (l_currentColumn < l_columnCount)
			{
				ImGui::SameLine();
			}
			else
			{
				l_currentColumn = k_initialFolderEntryColumnCount;
			}
		}

        // イテレーターの更新
		l_directoryITR.increment(l_errorCode);

		if (l_errorCode) { break; }
	}

	// 描画終了後にフォルダパスを変更
	if (l_requestedDirectoryPath.empty()) { return; }

	ApplyCurrentFolderPath(l_requestedDirectoryPath);
}
void FWK::Editor::ContentBrowserEditorWindow::DrawFolderEntry(const std::filesystem::path & a_entryPath, bool a_isFolder)
{
	const auto& l_entryPathString = a_entryPath.generic_string();
	const auto& l_entryName       = a_entryPath.filename      ().string();
	const auto& l_icon            = FetchVALFolderEntryIcon   (a_entryPath, a_isFolder);

	// 同じ名前のファイル/フォルダが別Folderに存在しても
	// ImGui上で別Itemとして扱えるよう、Path全体をIDに使用する
	ImGui::PushID(l_entryPathString.c_str());

	const ImVec2& l_entrySize = { k_folderEntryWidth, k_folderEntryHeight };

	// Item全体をClick領域にする
	// InvisibleButton事態は何も描画しない、
	// この後DrawListをつあって背景・Icon・名前を自部で描画する
	ImGui::InvisibleButton(k_folderEntryButtonLabel.data(), l_entrySize, ImGuiButtonFlags_MouseButtonLeft);

	// フォルダカードへOutlinerのGameObjectがDropされた場合、
	// このフォルダをPrefabの保存先として使用する
	if (a_isFolder)
	{
		DrawGameObjectPrefabDragDropTarget(a_entryPath);
	}

	const bool  l_isHovered  = ImGui::IsItemHovered();
	const bool  l_isSelected = m_selectedEntryPath == a_entryPath;
	const auto& l_itemMIN    = ImGui::GetItemRectMin   ();
	const auto& l_itemMAX    = ImGui::GetItemRectMax   ();
	      auto* l_drawList   = ImGui::GetWindowDrawList();

    if (!l_drawList) 
	{
		ImGui::PopID();

		return; 
	}

	if (l_isSelected)
	{
		const auto l_backgroundColor = ImGui::GetColorU32(l_isHovered ? ImGuiCol_HeaderActive : ImGuiCol_Header);

		l_drawList->AddRectFilled(l_itemMIN,
			                      l_itemMAX,
			                      l_backgroundColor,
			                      k_folderEntryRounding);
	}
	else if (l_isHovered)
	{
		const auto l_backgroundColor = ImGui::GetColorU32(ImGuiCol_HeaderHovered);

		l_drawList->AddRectFilled(l_itemMIN,
			                      l_itemMAX,
		                          l_backgroundColor,
			                      k_folderEntryRounding);
	}

	const auto l_iconSize = ImGui::GetFont()->CalcTextSizeA(k_folderEntryIconFontSize, 
		                                                    std::numeric_limits<float>::max(),
		                                                    k_filleRemainingSize,
		                                                    l_icon.data());

	const float l_iconPositionX = l_itemMIN.x + (k_folderEntryWidth - l_iconSize.x) * k_centeringRatio;
	const float l_iconPositionY = l_itemMIN.y + k_folderEntryIconTopPadding;

	const ImVec2& l_iconPosition = { l_iconPositionX, l_iconPositionY };
	const auto    l_textColor    = ImGui::GetColorU32(ImGuiCol_Text);

	// FontAwesomeGlyphを通常Textより大きく描画する
	l_drawList->AddText(ImGui::GetFont(),
		                k_folderEntryIconFontSize,
		                l_iconPosition,
		                l_textColor,
		                l_icon.data());

	// 実際のファイル名は変更しない
	// コンテンツブラウザー上で表示する名前だけを必要に応じて省略する
	std::string l_displayEntryName = l_entryName;

	// 9文字以上の場合は
	// hogeeeeeeTest.pngはhogeeeeee...というように表示
	if (l_displayEntryName.size() >= k_folderEntryNameDisplayCharacterCount)
	{
		l_displayEntryName = l_displayEntryName.substr(static_cast<std::uint32_t>(NULL), k_folderEntryNameDisplayCharacterCount);

		l_displayEntryName += k_folderEntryNameEllipsis;
	}

	const ImVec2& l_entryNameSize = ImGui::CalcTextSize(l_displayEntryName.c_str());

	// ファイル名は常にCard中央へ配置する
	const float l_textPositionX = l_itemMIN.x +                         (k_folderEntryWidth - l_entryNameSize.x) * k_centeringRatio;
	const float l_textPositionY = l_itemMAX.y - ImGui::GetTextLineHeight()                                       - k_folderEntryTextBottomPadding;

	const ImVec2& l_textPosition = { l_textPositionX, l_textPositionY };

	l_drawList->AddText(l_textPosition, l_textColor, l_displayEntryName.c_str());

	// 左クリックしたItemを現在の選択対象にする
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		m_selectedEntryPath = a_entryPath;
	}

	ImGui::PopID();
}
void FWK::Editor::ContentBrowserEditorWindow::DrawGameObjectPrefabDragDropTarget(const std::filesystem::path& a_directoryPath)
{
	auto& l_dragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

	std::weak_ptr<GameObject> l_gameObject = {};

	// Outlinerが送信しているGameObjectPayloadを受け取る
	if (!l_dragDropPayloadStorage.DragDropTarget(Constant::k_gameObjectDragDropPayloadLabel, l_gameObject)) { return; }

	// DropされたフォルダをPrefabの保存先としてPrefab化する
	const auto& l_prefabFilePath = m_fileSystem.CreatePrefabFromGameObject(l_gameObject, a_directoryPath, m_assetRegistry);

	// 新しく生成されたPrefabをContentBrowser上でも選択対象にしておく
	if (std::filesystem::exists(l_prefabFilePath))
	{
		m_selectedEntryPath = l_prefabFilePath;
	}
}
void FWK::Editor::ContentBrowserEditorWindow::DrawFolderCreateEntry()
{
}
void FWK::Editor::ContentBrowserEditorWindow::DrawCurrentFolderContextMenu()
{

}
void FWK::Editor::ContentBrowserEditorWindow::DrawFolderEntryContextMenu(const std::filesystem::path& a_folderPath)
{

}

void FWK::Editor::ContentBrowserEditorWindow::RequestFolderCreate(const std::filesystem::path& a_parentFolderPath)
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

void FWK::Editor::ContentBrowserEditorWindow::ApplyFolderCreateShortCut()
{
	
}

void FWK::Editor::ContentBrowserEditorWindow::ApplyCurrentFolderPath(const std::filesystem::path& a_folderPath)
{
	// FileをCurrentDirectoryとして設定することは許可しない
	if (std::error_code l_errorCode = {};
		!std::filesystem::is_directory(a_folderPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	// "."や".."が混ざったPathを
	// Path文字列上だけ正規化して保持する
	m_currentFolderPath = a_folderPath;
}

void FWK::Editor::ContentBrowserEditorWindow::ApplyFolderDeleteRequest()
{}

std::string_view FWK::Editor::ContentBrowserEditorWindow::FetchVALFolderEntryIcon(const std::filesystem::path& a_entryPath, bool a_isFolder) const
{
	if (a_isFolder) { return Constant::k_fontAwesomeFolderIcon; }

	const auto& l_extension = a_entryPath.extension();

	if (l_extension == Constant::k_lowerFBXExtension) { return Constant::k_fontAwesomeCubeIcon; }
	if (l_extension == Constant::k_lowerPNGExtension) { return Constant::k_fontAwesomeImageIcon; }

	return Constant::k_fontAwesomeFileIcon;
}