#include "EditorUndoRedoSystem.h"

void FWK::Editor::EditorUndoRedoSystem::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::EditorUndoRedoSystem::HandleUndoRedoShortcut()
{
    const auto& l_io = ImGui::GetIO();

    // InputText等で文字入力中はUndoRedoを処理しない
    // 入力中にCtrl + Zがテキスト編集のUndoとして消費されるため
    if (l_io.WantTextInput) { return; }

    // Ctrl + ZでUndoを実行する
    if (l_io.KeyCtrl &&
        ImGui::IsKeyPressed(ImGuiKey_Z))
    {
        Undo();
    }
    // Ctrl + Y でRedoを実行する
    else if (l_io.KeyCtrl &&
             ImGui::IsKeyPressed(ImGuiKey_Y))
    {
        Redo();
    }
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