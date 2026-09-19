#include "AssetBrowserEditorWindowSelectionState.h"

void FWK::Editor::AssetBrowserEditorWindowSelectionState::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::ClearSelection()
{
    ClearSelectedFilePathList();

    // 範囲選択の開始地点をクリアする
    m_rangeSelectionStartPath.clear();
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::ClearSelectedFilePathList()
{
    // 選択中のファイルパスリストをクリアする
    m_selectedFilePathList.clear();
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::SelectSingleFolder(const std::filesystem::path& a_folderPath, AssetBrowserEditorWindow& a_editorWindow, const bool a_updateCurrentFolderPath)
{
    m_selectedFilePathList.clear       ();
    m_selectedFilePathList.emplace_back(a_folderPath);

    // a_updateCurrentFolderPathがfalseなら実行しない
    // 現在フォルダを更新
    // 単一選択時は現在フォルダを選択フォルダにする
    // AssetPaneはこのm_currentFolderPathを参照して内容を表示する
    if (a_updateCurrentFolderPath)
    {
        a_editorWindow.SetCurrentSelectFolderPath(a_folderPath);
    }

    // 範囲選択の開始地点を更新
    m_rangeSelectionStartPath = a_folderPath;
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::SelectAll(const std::vector<std::filesystem::path>& a_displayedFilePathList)
{
    // 選択リストをクリアする
    // 全選択時は既存の選択状態を破棄して表示リスト全要素を選択する
    ClearSelectedFilePathList();

    // 表示リストの全要素を直訳リストへ追加する
    // a_displayedFilePathListは表示順(上から下)に並んでいるため
    // 追加順も表示順になる
    for (const auto& l_filePath : a_displayedFilePathList)
    {
        m_selectedFilePathList.emplace_back(l_filePath);
    }

    // 範囲選択の開始地点をクリアする
    // 全選択時は範囲選択の基準が不要なため
    // 次回Shift + クリック時は単一選択として扱われる
    m_rangeSelectionStartPath.clear();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindowSelectionState::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::AddSelectedFilePath(const std::filesystem::path& a_set)
{
    m_selectedFilePathList.emplace_back(a_set);
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::EraseSelectedFilePath(const std::vector<std::filesystem::path>::const_iterator& a_itr)
{
    m_selectedFilePathList.erase(a_itr);
}