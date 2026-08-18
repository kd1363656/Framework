#include "ContentBrowserEditorWindowEntrySelection.h"

void FWK::Editor::ContentBrowserEditorWindowEntrySelection::SynchronizeCurrentFolderEntries(const std::vector<Struct::ContentBrowserEntryData>& a_currentFolderEntryList)
{
    // CurrentFolderに現在表示されているEntry一覧を保持する
    // この順番がそのままShift範囲選択の基準になるため、
    // 呼びだし側では表示順に並べた状態で渡す
    m_currentFolderEntryPathList = a_currentFolderEntryList;

    // Explorer等からFolder/Fileを外部削除された場合や、
    // CurrentFolderが変更された場合に
    // CurrentFolder内に存在しなくなったEntryを選択状態から取り除く
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

    // Shift選択のAnchorもCurrentFolderから消えていた場合は解除する
    if (!m_rangeAnchorEntryPath.empty() &&
        !ContainsCurrentFolderEntry(m_rangeAnchorEntryPath))
    {
        m_rangeAnchorEntryPath.clear();
    }
}

void FWK::Editor::ContentBrowserEditorWindowEntrySelection::SelectSingleEntry(const std::filesystem::path& a_entryPath, const std::vector<Struct::ContentBrowserEntryData>& a_currentFolderEntryList)
{
    if (!ContainsCurrentFolderEntry(a_entryPath)) { return; }

    // 通常Clickでは、それまでの複数選択をすべて解除する
    m_selectedEntryPathSet.clear();

    AddSelectedEntry(a_entryPath);

    // 通常ClickされたEntryを、
    // 次回Shift選択の開始位置にする
    m_rangeAnchorEntryPath = a_entryPath;
}
void FWK::Editor::ContentBrowserEditorWindowEntrySelection::SelectRangeEntry(const std::filesystem::path& a_entryPath, const std::vector<Struct::ContentBrowserEntryData>& a_currentFolderEntryList)
{
    if (!ContainsCurrentFolderEntry(a_entryPath)) { return; }

    // Shift選択の開始位置が存在しない場合は、
    // 通常の単一選択として扱う
    if (m_rangeAnchorEntryPath.empty() ||
        !ContainsCurrentFolderEntry(m_rangeAnchorEntryPath))
    {
        SelectSingleEntry(a_entryPath);

        return;
    }

    std::size_t l_anchorIndex   = m_currentFolderEntryPathList.size();
    std::size_t l_selectedIndex = m_currentFolderEntryPathList.size();

    for (std::size_t l_entryIndex = 0ULL; l_entryIndex < m_currentFolderEntryPathList.size(); ++l_entryIndex)
    {
        const auto& l_entryPath = m_currentFolderEntryPathList[l_entryIndex];

        if (l_entryPath == m_rangeAnchorEntryPath)
        {
            l_anchorIndex = l_entryIndex;
        }

        if (l_entryPath == a_entryPath)
        {
            l_selectedIndex = l_entryIndex;
        }
    }

    // CurrenFolde一覧に存在していることは事前確認済みだが、
    // 状態不整合が発生した場合に備えて安全のため範囲外かどうかを確認
    if (l_anchorIndex == m_currentFolderEntryPathList.size() ||
        l_selectedIndex == m_currentFolderEntryPathList.size())
    {
        return;
    }

    const auto l_rangeBeginIndex = std::min(l_anchorIndex, l_selectedIndex);
    const auto l_rangendIndex    = std::max(l_anchorIndex, l_selectedIndex);

    // 通常のShift選択では以前の選択を解除して、
    // Anchor=Click位置までを選択し直す
    m_selectedEntryPathSet.clear();

    for (std::size_t l_entryIndex = l_rangeBeginIndex; l_entryIndex <= l_rangendIndex; ++l_entryIndex)
    {
        AddSelectedEntry(m_currentFolderEntryPathList[l_entryIndex]);
    }
}
void FWK::Editor::ContentBrowserEditorWindowEntrySelection::SelectAllEntries()
{
    m_selectedEntryPathSet.clear();

    for (const auto& l_entryPath : m_currentFolderEntryPathList)
    {
        AddSelectedEntry(l_entryPath);
    }

    // Entryが一つ以上存在し
    // Shift選択Anchorがまだない場合だけ
    // 最初のEntryをAnchorとして使用する
    if (!m_currentFolderEntryPathList.empty() &&
        m_rangeAnchorEntryPath.empty())
    {
        m_rangeAnchorEntryPath = m_currentFolderEntryPathList.front();
    }
}

void FWK::Editor::ContentBrowserEditorWindowEntrySelection::ToggleEntrySelection(const std::filesystem::path& a_entryPath)
{
    if (!ContainsCurrentFolderEntry(a_entryPath)) { return; }

    // Ctrl + ClickしたEntryを
    // 次のShift選択の基準にもする
    m_rangeAnchorEntryPath = a_entryPath;

    if (ContainsSelectedEntry(a_entryPath))
    {
        // 既に選択されているEntryなら
        // そのEntryだけ選択解除する
        m_selectedEntryPathSet.erase(a_entryPath);

        return;
    }

    // 未選択Entryなら現在の複数選択へ追加する
    AddSelectedEntry(a_entryPath);
}

void FWK::Editor::ContentBrowserEditorWindowEntrySelection::ClearSelectedEntries()
{
    m_selectedEntryPathSet.clear();
    m_rangeAnchorEntryPath.clear();
}

bool FWK::Editor::ContentBrowserEditorWindowEntrySelection::ContainsSelectedEntry(const std::filesystem::path& a_entryPath) const
{
    if (a_entryPath.empty()) { return false; }

    return m_selectedEntryPathSet.contains(a_entryPath);
}

bool FWK::Editor::ContentBrowserEditorWindowEntrySelection::ContainsCurrentFolderEntry(const std::filesystem::path& a_selectedEntryPathSet) const
{
    if (a_selectedEntryPathSet.empty()) { return false; }

    return std::ranges::find(m_currentFolderEntryPathList, a_selectedEntryPathSet) != m_currentFolderEntryPathList.end();
}

std::size_t FWK::Editor::ContentBrowserEditorWindowEntrySelection::FetchVALSelectedEntryCount() const
{
    return m_selectedEntryPathSet.size();
}

void FWK::Editor::ContentBrowserEditorWindowEntrySelection::AddSelectedEntry(const std::filesystem::path& a_entryPath)
{
    if (!ContainsCurrentFolderEntry(a_entryPath)) { return; }

    // unordered_setなので、
    // 同じEntryPathが二重登録されることはない
    m_selectedEntryPathSet.emplace(a_entryPath);
}