#include "ContentBrowserEditorWindow.h"

void FWK::Editor::ContentBrowserEditorWindow::Draw()
{
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	// 左ペイン
	if (const ImVec2 l_directoryTreePaneSize = { k_directoryTreePanelWidth, k_filleRemainingSize };
		ImGui::BeginChild(k_directoryTreeChildString.data(), l_directoryTreePaneSize, true))
	{
		DrawDirectoryTree();
	}
	
	ImGui::EndChild();
	ImGui::SameLine();

	// 右ペイン
	if (const ImVec2 l_currentDirectoryPaneSize = { k_filleRemainingSize, k_filleRemainingSize };
		ImGui::BeginChild(k_currentDirectoryChildString.data(), l_currentDirectoryPaneSize, true))
	{
		DrawCurrentDirectory();
	}

	ImGui::EndChild();
	ImGui::End     ();
}

void FWK::Editor::ContentBrowserEditorWindow::DrawDirectoryTree()
{
	std::error_code l_errorCode = {};

	// ContentBrowserのRootであるContent自体が存在しなければ
	// DirectoryTreeは作成できない
	if (!std::filesystem::exists(k_contentRootDirectoryPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	l_errorCode.clear();

	if (!std::filesystem::is_directory(k_contentRootDirectoryPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	// RootであるContentからFolderTreeでの描画を開始する
	DrawDirectoryTreeNode(k_contentRootDirectoryPath);
}
void FWK::Editor::ContentBrowserEditorWindow::DrawDirectoryTreeNode(const std::filesystem::path& a_directoryPath)
{
	std::error_code l_errorCode = {};

	if (!std::filesystem::is_directory(a_directoryPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	const bool l_hasChildDirectory = HasChildDirectory(a_directoryPath);

	// 矢印部分をクリックすると開閉できるようにする
	ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
		                                 ImGuiTreeNodeFlags_SpanAvailWidth;

	// 現在右ペインで開いているDirectoryを
	// 右側Treeでも選択状態として視覚化する
	if (m_currentDirectoryPath == a_directoryPath)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	// 子Folderを持たないDirectoryには展開用のArrowを表示しない
	if (!l_hasChildDirectory)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
		l_treeNodeFlags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	// ContentBrowserを初めて開いた時だけ
	// RootであるContentを開いた状態にする
	// ImGuiCond_Onceなので、ユーザが後からContentを閉じれば閉じたままになる
	if (a_directoryPath == k_contentRootDirectoryPath)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	}

	// TreeNodeのIDにはDirectory全体のPathを使用する
	// 例えば
	// Content/Model/Actor
	// Content/Texture/Actor
	// のように同じ名前のFolderが別階層にあってる
	// ImGui上では別Nodeとして扱える
	const auto l_directoryPathString = a_directoryPath.generic_string();
	      auto l_directoryName       = a_directoryPath.filename      ().string();

	// 通常Content出は発生しないが
	// filename()が空だった場合の最低限のか表示
	if (l_directoryName.empty())
	{
		l_directoryName = l_directoryPathString;
	}

	const bool l_isNodeOpen = ImGui::TreeNodeEx(l_directoryPathString.c_str(), 
		                                        l_treeNodeFlags,
		                                        "%s %s", 
		                                        Constant::k_fontAwesomeFolderIcon.data(),
		                                        l_directoryName.c_str());

	// Folderの行をClickした場合は、
	// そのFolderを右ペインで開く
	// ArrowをクリックしてTreeNodeを開閉しただけの場合は
	// CurrentDirectoryを変更しない
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) &&
		!ImGui::IsItemToggledOpen())
	{
		ApplyCurrentDirectoryPath(a_directoryPath);
	}

	// フォルダーが閉じている場合は
	// それより下の階層は一切走査しない
	if (!l_isNodeOpen ||
		!l_hasChildDirectory)
	{
		return;
	}

	// 現在Nodeが開いているDirectoryの
	// 「直下」だけを調べる
	std::filesystem::directory_iterator l_directoryITR    = { a_directoryPath, l_errorCode };
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
		// Folderだけを再帰的にTreeへ追加する
		if (l_directoryITR->is_directory(l_entryErrorCode) &&
			!l_entryErrorCode)
		{
			DrawDirectoryTreeNode(l_directoryITR->path());
		}

		l_directoryITR.increment(l_errorCode);

		if (l_errorCode) { break; }
	}

	ImGui::TreePop();
}

void FWK::Editor::ContentBrowserEditorWindow::DrawCurrentDirectory()
{
	std::error_code l_errorCode = {};

	// Explorer等から現在開いていたFolderを削除された場合などに
	// 無効Pathを保持し続けないようにContentへ戻す
	if(!std::filesystem::is_directory(m_currentDirectoryPath, l_errorCode) ||
		l_errorCode)
	{
		ApplyCurrentDirectoryPath(k_contentRootDirectoryPath);

		l_errorCode.clear();
	}

	// Content事態まで存在しない場合は表示できない
	if (!std::filesystem::is_directory(m_currentDirectoryPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	// 現在の右ペインで使用可能な横幅
	const float l_availableWidth = ImGui::GetContentRegionAvail().x;

	// Cardと次のCardの間隔には
	// Editor全体のImTuiItemSpacingをそのまま使用する
	const float l_itemSpacing         = ImGui::GetStyle().ItemSpacing.x;
	const float l_directoryEntryPitch = k_directoryEntryWidth + l_itemSpacing;

	// ペインが広ければColumn数を増やし
	// 狭ければ自動的に少なくする
	const std::uint32_t l_calculatedColumnCount = static_cast<std::uint32_t>(l_availableWidth + l_itemSpacing) / static_cast<std::uint32_t>(l_directoryEntryPitch);
	const std::uint32_t l_columnCount           = std::max                  (k_minDirectoryEntryColumnCount, l_calculatedColumnCount);

	auto l_currentColumn = k_initialDirectoryEntryColumnCount;

	// DoubleClickされた瞬間にm_currentDirectoryPathを置き換えず、
	// 現在Directoryの描画が全部終わってから反映する

	      std::filesystem::path               l_requestedDirectoryPath = {};
	      std::filesystem::directory_iterator l_directoryITR           = { m_currentDirectoryPath, l_errorCode };
	const std::filesystem::directory_iterator l_endDirectoryITR        = {};

	if (l_errorCode) { return; }

	while (l_directoryITR != l_endDirectoryITR)
	{
		std::error_code l_entryErrorCode = {};

		const bool l_isDirectory = l_directoryITR->is_directory(l_entryErrorCode);

		if (!l_entryErrorCode)
		{
			const auto& l_entryPath = l_directoryITR->path();

			DrawDirectoryEntry(l_entryPath, l_isDirectory);

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
				l_currentColumn = k_initialDirectoryEntryColumnCount;
			}
		}

        // イテレーターの更新
		l_directoryITR.increment(l_errorCode);

		if (l_errorCode) { break; }
	}

	// 描画終了後にDirectory変更
	if (l_requestedDirectoryPath.empty()) { return; }

	ApplyCurrentDirectoryPath(l_requestedDirectoryPath);
}

void FWK::Editor::ContentBrowserEditorWindow::DrawDirectoryEntry(const std::filesystem::path& a_entryPath, bool a_isDirectory)
{


	const auto& l_entryPathString = a_entryPath.generic_string();
	const auto& l_entryName       = a_entryPath.filename      ().string();
	const auto& l_icon            = FetchVALDirectoryEntryIcon(a_entryPath, a_isDirectory);

	// 同じ名前のFile/Folderが別Directoryに存在しても
	// ImGui上で別Itemとして扱えるよう、Path全体をIDに使用する
	ImGui::PushID(l_entryPathString.c_str());

	const ImVec2& l_entrySize = { k_directoryEntryWidth, k_directoryEntryHeight };

	// Item全体をClick領域にする
	// InvisibleButton事態は何も描画しない、
	// この後DrawListをつあって背景・Icon・名前を自部で描画する
	ImGui::InvisibleButton(k_directoryEntryButtonString.data(), l_entrySize, ImGuiButtonFlags_MouseButtonLeft);

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
			                      k_directoryEntryRounding);
	}
	else if (l_isHovered)
	{
		const auto l_backgroundColor = ImGui::GetColorU32(ImGuiCol_HeaderHovered);

		l_drawList->AddRectFilled(l_itemMIN,
			                      l_itemMAX,
		                          l_backgroundColor,
			                      k_directoryEntryRounding);
	}

	const auto l_iconSize = ImGui::GetFont()->CalcTextSizeA(k_directoryEntryIconFontSize, 
		                                                    std::numeric_limits<float>::max(),
		                                                    k_filleRemainingSize,
		                                                    l_icon.data());

	const float l_iconPositionX = l_itemMIN.x + (k_directoryEntryWidth - l_iconSize.x) * k_centeringRatio;
	const float l_iconPositionY = l_itemMIN.y + k_directoryEntryIconTopPadding;

	const ImVec2& l_iconPosition = { l_iconPositionX, l_iconPositionY };
	const auto    l_textColor    = ImGui::GetColorU32(ImGuiCol_Text);

	// FontAwesomeGlyphを通常Textより大きく描画する
	l_drawList->AddText(ImGui::GetFont(),
		                k_directoryEntryIconFontSize,
		                l_iconPosition,
		                l_textColor,
		                l_icon.data());

	// 実際のファイル名は変更しない
	// コンテンツブラウザー上で表示する名前だけを必要に応じて省略する
	std::string l_displayEntryName = l_entryName;

	// 9文字以上の場合は
	// hogeeeeeeTest.pngはhogeeeeee...というように表示
	if (l_displayEntryName.size() >= k_directoryEntryNameDisplayCharacterCount)
	{
		l_displayEntryName = l_displayEntryName.substr(static_cast<std::uint32_t>(NULL), k_directoryEntryNameDisplayCharacterCount);

		l_displayEntryName += k_directoryEntryNameEllipsis;
	}

	const ImVec2& l_entryNameSize = ImGui::CalcTextSize(l_displayEntryName.c_str());

	// File名は常にCard中央へ配置する
	const float l_textPositionX = l_itemMIN.x +                         (k_directoryEntryWidth - l_entryNameSize.x) * k_centeringRatio;
	const float l_textPositionY = l_itemMAX.y - ImGui::GetTextLineHeight()                                          - k_directoryEntryTextBottomPadding;

	const ImVec2& l_textPosition = { l_textPositionX, l_textPositionY };

	l_drawList->AddText(l_textPosition, l_textColor, l_displayEntryName.c_str());

	// 左クリックしたItemを現在の選択対象にする
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		m_selectedEntryPath = a_entryPath;
	}

	ImGui::PopID();
}

bool FWK::Editor::ContentBrowserEditorWindow::HasChildDirectory(const std::filesystem::path& a_directoryPath) const
{
	      std::error_code                     l_errorCode       = {};
	      std::filesystem::directory_iterator l_directoryITR    = { a_directoryPath, l_errorCode };
	const std::filesystem::directory_iterator l_endDirectoryITR = {};

	if (l_errorCode) { return false; }

	while (l_directoryITR != l_endDirectoryITR)
	{
		std::error_code l_entryErrorCode = {};

		const bool l_isDirectory = l_directoryITR->is_directory(l_entryErrorCode);

		// 一つでも子Folderがあれば
		// TreeNodeを展開可能にする
		if (!l_entryErrorCode &&
			l_isDirectory)
		{
			return true;
		}

		l_directoryITR.increment(l_errorCode);

		if (l_errorCode) { return false; }
	}

	return false;
}

void FWK::Editor::ContentBrowserEditorWindow::ApplyCurrentDirectoryPath(const std::filesystem::path& a_directoryPath)
{
	// FileをCurrentDirectoryとして設定することは許可しない
	if (std::error_code l_errorCode = {};
		!std::filesystem::is_directory(a_directoryPath, l_errorCode) ||
		l_errorCode)
	{
		return;
	}

	// "."や".."が混ざったPathを
	// Path文字列上だけ正規化して保持する
	m_currentDirectoryPath = a_directoryPath.lexically_normal();
}

std::string_view FWK::Editor::ContentBrowserEditorWindow::FetchVALDirectoryEntryIcon(const std::filesystem::path& a_entryPaath, bool a_isDirectory) const
{
	if (a_isDirectory) { return Constant::k_fontAwesomeFolderIcon; }

	const auto& l_extension = a_entryPaath.extension();

	if (l_extension == Constant::k_lowerFBXExtension) { return Constant::k_fontAwesomeCubeIcon; }
	if (l_extension == Constant::k_lowerPNGExtension) { return Constant::k_fontAwesomeImageIcon; }

	return Constant::k_fontAwesomeFileIcon;
}