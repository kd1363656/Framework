#include "ContentBrowserEditorWindowEntryController.h"

void FWK::Editor::ContentBrowserEditorWindowEntryController::RefreshCurrentFolderEntryList(const ContentBrowserEditorWindowAssetRegistry& a_assetRegistry, const std::filesystem::path& a_currentFolderPath)
{
	m_currentFolderEntryDataList.clear();

	      std::error_code                      l_errorCode       = {};
	      std::filesystem::directory_iterator  l_directoryITR    = { a_currentFolderPath, l_errorCode };
	const std::filesystem::directory_iterator& l_endDirectoryITR = {};

	if (l_errorCode)
	{
		// 読み込みに失敗した場合は中途半端な一覧を残さない
		// 型の一覧へ同期すことで、以前のSelectionも残さない
		m_isCurrentFolderEntryListDirty = false;

		SynchronizeCurrentFolderEntries();

		return;
	}

	// ディレクトリイテレーターが終端に行くまでループ
	while (l_directoryITR != l_endDirectoryITR)
	{
		std::error_code l_entryErrorCode = {};

		const bool l_isFolder = l_directoryITR->is_directory(l_entryErrorCode);

		// エラーが発生した場合そのEntryを飛ばして走査を継続する
		if (l_entryErrorCode)
		{
			// エラーコードをクリアしてからインクリメント
			l_errorCode.clear       ();
			l_directoryITR.increment(l_errorCode);

			if (l_errorCode)
			{
				// Iterator事態の継続に失敗した場合は
				// 中途半端な一覧を使用しないように全Entryを破棄する
				m_currentFolderEntryDataList.clear();

				break;
			}

			continue;
		}

		bool l_isFile = false;

		// フォルダならis_regular_file(9を追加で確認する必要はない
		if (!l_isFolder)
		{
			l_isFile = l_directoryITR->is_regular_file(l_entryErrorCode);
		}

		// エラーが起こっておらずフォルダかファイルならエントリーデータを構築
		if (!l_entryErrorCode &&
			(l_isFolder      ||
			 l_isFile))
		{
			Struct::ContentBrowserEntryData l_entryData = {};

			// 正規化を行い余計な..などを削除したファイルパスを格納
			l_entryData.m_entryPath = l_directoryITR->path().lexically_normal();

			// フォルダかどうかの確認、フォルダじゃなければファイルとみなす
			l_entryData.m_isFolder = l_isFolder;

			// Folderはすべて選択可能
			// FileはAssetRegistryへ登録済みのものだけ
			// 現在ContentBrowserが管理しているAssetとして選択花押にする
			l_entryData.m_isSelectable = l_isFolder || 
				                         !a_assetRegistry.FindVALAssetUUID(l_entryData.m_entryPath).is_nil();

			m_currentFolderEntryDataList.emplace_back(std::move(l_entryData));
		}

		// イテレーターの更新
		l_directoryITR.increment(l_errorCode);

		if (l_errorCode)
		{
			// directory_iteratorの列挙途中で失敗した場合も、
			// 一部だけ表示されたCacheを使用しない
			m_currentFolderEntryDataList.clear();

			break;
		}
	}

	// directory_iteratorの列挙順は保証されない。
	// 表示順をPath順へ固定することで
	// Refresh後もShift選択範囲が変化しないようにする
	std::ranges::sort(m_currentFolderEntryDataList, {}, &Struct::ContentBrowserEntryData::m_entryPath);

	m_isCurrentFolderEntryListDirty = false;

	// Entry一覧とSelectionは同じクラスが所有しているため、
	// Refresh直後に必ずSelection側も同期して不整合を残さない
	SynchronizeCurrentFolderEntries();
}

void FWK::Editor::ContentBrowserEditorWindowEntryController::SelectSingleEntry(const std::filesystem::path& a_entryPath)
{
	if (!ContainsCurrentFolderEntry(a_entryPath)) { return; }

	// 通常Clickでは、それまでの複数選択をすべて解除する
	m_selectedEntryPathSet.clear  ();
	m_selectedEntryPathSet.emplace(a_entryPath);

	// 通常ClickされたEntryを
	// いj会Shift選択の開始位置にする
	m_rangeAnchorEntryPath = a_entryPath;
}
void FWK::Editor::ContentBrowserEditorWindowEntryController::SelectRangeEntry(const std::filesystem::path& a_entryPath)
{
	if (!ContainsCurrentFolderEntry(a_entryPath)) { return; }

	// Shift選択のAnchorが存在しない、
	// またはRefreshによって無効になっている場合は、
	// 通常の単一選択として扱う
	if (m_rangeAnchorEntryPath.empty() ||
		!ContainsCurrentFolderEntry(m_rangeAnchorEntryPath))
	{
		SelectSingleEntry(a_entryPath);

		return;
	}

	if (m_currentFolderEntryDataList.empty()) { return; }

	std::size_t l_anchorIndex   = m_currentFolderEntryDataList.size();
	std::size_t l_selectedIndex = m_currentFolderEntryDataList.size();

	for (std::size_t l_entryIndex = 0ULL; l_entryIndex < m_currentFolderEntryDataList.size(); ++l_entryIndex)
	{
		const auto& l_entryData = m_currentFolderEntryDataList[l_entryIndex];

		// Selection対象外Entryは
		// Anchorや選択終端として扱わない
		if (!l_entryData.m_isSelectable) { continue; }

		if (l_entryData.m_entryPath == m_rangeAnchorEntryPath) 
		{
			l_anchorIndex = l_entryIndex;
		}

		if (l_entryData.m_entryPath == a_entryPath)
		{
			l_selectedIndex = l_entryIndex;
		}
	}

	// ContainsCurrentFolderEntry()で事前確認しているが、
	// 状態不整合が起きた場合でも
	// 範囲外アクセスしないよう確認する
	if (l_anchorIndex == m_currentFolderEntryDataList.size() ||
		l_selectedIndex == m_currentFolderEntryDataList.size())
	{
		return;
	}

	const auto& l_rangeBeginIndex = std::min(l_anchorIndex, l_selectedIndex);
	const auto& l_rangeEndIndex   = std::max(l_anchorIndex, l_selectedIndex);

	// 通常のShift選択では以前の選択を解除し、
	// Anchorから今回Clickされた位置までを選択し直す。
	m_selectedEntryPathSet.clear();

	for (std::size_t l_entryIndex = l_rangeBeginIndex; l_entryIndex <= l_rangeEndIndex; ++l_entryIndex)
	{
		const auto& l_entryData = m_currentFolderEntryDataList[l_entryIndex];

		// 選択可能でなければ処理を飛ばす
		if (!l_entryData.m_isSelectable) { continue; }

		m_selectedEntryPathSet.emplace(l_entryData.m_entryPath);
	}
}
void FWK::Editor::ContentBrowserEditorWindowEntryController::SelectAllEntries()
{
	m_selectedEntryPathSet.clear();

	for (const auto& l_entryData : m_currentFolderEntryDataList)
	{
		if (!l_entryData.m_isSelectable) { continue; }

		m_selectedEntryPathSet.emplace(l_entryData.m_entryPath);
	}

	// 通常ClickやCtrl + Clickによって
	// Anchorが既に決まっている場合は、
	// ユーザーが最後に操作した位置をそのまま維持する
	if (!m_rangeAnchorEntryPath.empty()) { return; }

	// Anchorがまだ存在しない場合だけ、
	// 最初の選択可能Entryを
	// Shift範囲選択用Anchorとして使用する
	for (const auto& l_entryData : m_currentFolderEntryDataList)
	{
		if (!l_entryData.m_isSelectable) { continue; }

		m_rangeAnchorEntryPath = l_entryData.m_entryPath;

		break;
	}
}

void FWK::Editor::ContentBrowserEditorWindowEntryController::ToggleEntrySelection(const std::filesystem::path& a_entryPath)
{
	if (!ContainsCurrentFolderEntry(a_entryPath)) { return; }

	// Ctrl + ClickしたEntryを、次のShift選択の基準にもする
	m_rangeAnchorEntryPath = a_entryPath;

	if (ContainsSelectedEntry(a_entryPath))
	{
		// 既に選択されているEntryなら
		// そのEntryだけ選択解除する
		m_selectedEntryPathSet.erase(a_entryPath);;

		return;
	}

	// 未使用Entryなら現在の複数選択へ追加する
	m_selectedEntryPathSet.emplace(a_entryPath);
}

void FWK::Editor::ContentBrowserEditorWindowEntryController::ClearSelectedEntries()
{
	m_selectedEntryPathSet.clear();
	m_rangeAnchorEntryPath.clear();
}

bool FWK::Editor::ContentBrowserEditorWindowEntryController::ContainsSelectedEntry(const std::filesystem::path& a_entryPath) const
{
	if (a_entryPath.empty()) { return false; }

	return m_selectedEntryPathSet.contains(a_entryPath);
}

std::size_t FWK::Editor::ContentBrowserEditorWindowEntryController::FetchVALSelectedEntryCount() const
{
	return m_selectedEntryPathSet.size();
}

bool FWK::Editor::ContentBrowserEditorWindowEntryController::ContainsCurrentFolderEntry(const std::filesystem::path& a_entryPath) const
{
	if (a_entryPath.empty()) { return false; }

	for (const auto& l_entryData : m_currentFolderEntryDataList)
	{
		// Selection対象外Entryは
		// 選択対象として存在しないものとして扱う
		if (!l_entryData.m_isSelectable) { continue; }

		if (l_entryData.m_entryPath == a_entryPath) { return true; }
	}

	return false;
}

void FWK::Editor::ContentBrowserEditorWindowEntryController::SynchronizeCurrentFolderEntries()
{
	// Explorer等からFolder/Fileを外部削除された場合や
	// CurrentFolderが変更された場合に、
	// 一覧に存在しなくなったentryを選択状態から取り除く
	auto l_itr = m_selectedEntryPathSet.begin();

	while (l_itr != m_selectedEntryPathSet.end())
	{
		if (ContainsCurrentFolderEntry(*l_itr))
		{
			++l_itr;

			continue;
		}

		l_itr = m_selectedEntryPathSet.erase(l_itr);
	}

	// Shift選択のAnchorもCurrentFolderから
	// 消えていた場合は解除する
	if (!m_rangeAnchorEntryPath.empty() &&
		!ContainsCurrentFolderEntry(m_rangeAnchorEntryPath))
	{
		m_rangeAnchorEntryPath.clear();
	}
}