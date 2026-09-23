#include "WorldOutlinerEditorWindowClipboard.h"

void FWK::Editor::WorldOutlinerEditorWindowClipboard::Apply(const std::vector<boost::uuids::uuid>& a_gameObjectUUIDList, const Enum::WorldOutlinerClipboardOperationType a_operationType)
{
    // 現在コピー、切り取り仕様としているUUIDに過去のコピー、切り取り用の
    // UUIDが残っていては意図しない動作になるめここで削除
    Clear();

    m_operationType = a_operationType;

    // a_gameObjectUUIDListの各UUIDを順二を維持したまま格納する
    // 同じUUIDが複数渡された場合は一つだけ格納する
    for (const auto& l_gameObjectUUID : a_gameObjectUUIDList)
    {
        // 既に登録されているなら登録しない
        if (m_clipboardGameObjectUUIDSet.contains(l_gameObjectUUID)) { continue; }

        m_clipboardGameObjectUUIDList.emplace_back(l_gameObjectUUID);
        m_clipboardGameObjectUUIDSet.emplace      (l_gameObjectUUID);
    }
}

void FWK::Editor::WorldOutlinerEditorWindowClipboard::Clear()
{
    m_clipboardGameObjectUUIDList.clear();
    m_clipboardGameObjectUUIDSet.clear ();

    m_operationType = Enum::WorldOutlinerClipboardOperationType::Invalid;
}

bool FWK::Editor::WorldOutlinerEditorWindowClipboard::Contains(const boost::uuids::uuid& a_gameObjectUUID) const
{
    return m_clipboardGameObjectUUIDSet.contains(a_gameObjectUUID);
}

bool FWK::Editor::WorldOutlinerEditorWindowClipboard::IsEmpty() const
{
    return m_clipboardGameObjectUUIDList.empty();
}