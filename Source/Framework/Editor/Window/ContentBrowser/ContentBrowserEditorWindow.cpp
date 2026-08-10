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
	// そのFolderを右ペインでも開く
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
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
	const float l_itemSpacing         = ImGui::GetStyle                                   ().ItemSpacing.x;
	const float l_directoryEntryPitch = k_directoryEntryWidth + static_cast<std::uint32_t>(l_itemSpacing);

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

			// FoldlerをDoubleClickしてCurrentDirectoryが要求された場合、
			// 古いDirectoryのEntryをこのフレームで描画し続けない

			const auto& l_entryName = l_entryPath.filename().string();
			const auto& l_icon      = l_isDirectory ? Constant::k_fontAwesomeFolderIcon : Constant::k_fontAwesomeFileIcon;

			const auto l_itemLabel = std::format("{} {}", l_icon, l_entryName);

			ImGui::Selectable(l_itemLabel.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);

			// ここではCurrentDirectoryを変更しない
			// 開きたいFolderだけ保存しておく
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

std::string FWK::Editor::ContentBrowserEditorWindow::FetchVALDirectoryEntryIcon(const std::filesystem::path& a_entryPaath, bool a_isDirectory) const
{
	return std::string();
}