#include "AssetBrowserEditorWindowSelectionState.h"

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

void FWK::Editor::AssetBrowserEditorWindowSelectionState::SelectSingleFolder(const std::filesystem::path& a_folderPath, AssetBrowserEditorWindow& a_editorWindow)
{
    m_selectedFilePathList.clear       ();
    m_selectedFilePathList.emplace_back(a_folderPath);
    
    // 現在フォルダを更新
    // 単一選択時は現在フォルダを選択フォルダにする
    // AssetPaneはこのm_currentFolderPathを参照して内容を表示する
    a_editorWindow.SetCurrentSelectFolderPath(a_folderPath);

    // 範囲選択の開始地点を更新
    m_rangeSelectionStartPath = a_folderPath;
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::AddSelectedFilePath(const std::filesystem::path& a_set)
{
    m_selectedFilePathList.emplace_back(a_set);
}

void FWK::Editor::AssetBrowserEditorWindowSelectionState::EraseSelectedFilePath(const std::vector<std::filesystem::path>::const_iterator& a_itr)
{
    m_selectedFilePathList.erase(a_itr);
}