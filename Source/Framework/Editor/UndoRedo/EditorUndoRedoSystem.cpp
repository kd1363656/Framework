#include "EditorUndoRedoSystem.h"

void FWK::Editor::EditorUndoRedoSystem::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::EditorUndoRedoSystem::Undo()
{
    if (m_undoList.empty()) { return; }

    // std::vectorの末尾から取り出す
    // std::moveで所有権を移動
    auto l_command = std::move(m_undoList.back());

    m_undoList.pop_back();

    // 取り出したコマンドのUndoを実行
    l_command->Undo();

    // Redoリストへ追加
    m_redoList.emplace_back(std::move(l_command));
}
void FWK::Editor::EditorUndoRedoSystem::Redo()
{
    if (m_redoList.empty()) { return; }

    // std::vectorの末尾から取り出す
    auto l_command = std::move(m_redoList.back());

    m_redoList.pop_back();

    // 取り出したコマンドのRedoを実行
    l_command->Redo();

    // Undoスタックへ戻す
    m_undoList.emplace_back(std::move(l_command));
}

void FWK::Editor::EditorUndoRedoSystem::Clear()
{
    m_undoList.clear();
    m_redoList.clear();
}

nlohmann::json FWK::Editor::EditorUndoRedoSystem::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}