#include "AssetBrowserEditorWindowClipboard.h"

void FWK::Editor::AssetBrowserEditorWindowClipboard::Apply(const std::vector<std::filesystem::path>& a_filePathList, const Enum::AssetBrowserFileClipboardOperationType a_operationType)
{
    // 現在コピー、切り取りしようとしているファイルパスに過去の
    // コピー切り取り用のファイルパスが残っていては意図しない動作になるため
    // ここで削除する
    Clear();

    m_operationType = a_operationType;

    // a_filePathListの各Pathを順序を維持したまま
    // 同じPathが複数渡された場合は1つだけ格納する
    for (const auto& l_filePath : a_filePathList)
    {
        // 既に登録されているなら登録しない
        if (m_clipboardFilePathSet.contains(l_filePath)) { continue; }

        m_clipboardFilePathSet.emplace      (l_filePath);
        m_clipboardFilePathList.emplace_back(l_filePath);
    }
}

void FWK::Editor::AssetBrowserEditorWindowClipboard::Clear()
{
    m_clipboardFilePathList.clear();
    m_clipboardFilePathSet.clear ();

    m_operationType = Enum::AssetBrowserFileClipboardOperationType::Invalid;
}

bool FWK::Editor::AssetBrowserEditorWindowClipboard::Contains(const std::filesystem::path& a_filePath) const
{
    return m_clipboardFilePathSet.contains(a_filePath);
}

bool FWK::Editor::AssetBrowserEditorWindowClipboard::IsEmpty() const
{
    return m_clipboardFilePathList.empty();
}