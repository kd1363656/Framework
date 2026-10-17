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

    // シーンを変えるコマンドを戻したら、シーンの変更回数も1つ戻す
    if (IsSceneCommand(*l_command))
    {
        --m_sceneChangeCount;
    }

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

    if (IsSceneCommand(*l_command))
    {
        ++m_sceneChangeCount;
    }

    // Undoスタックへ戻す
    m_undoList.emplace_back(std::move(l_command));
}

void FWK::Editor::EditorUndoRedoSystem::Clear()
{
    m_undoList.clear();
    m_redoList.clear();

    // シーンを読み込み直した直後と同じく、保存済みの状態から数え直す
    m_sceneChangeCount      = k_initialSceneChangeCount;
    m_savedSceneChangeCount = k_initialSceneChangeCount;
}

void FWK::Editor::EditorUndoRedoSystem::MarkSceneSaved()
{
    // 保存した時点のシーンの変更回数を覚えておく
    // Undo/Redoで同じ回数に戻ってくれば、保存したときと同じ内容になっている
    // 例 : 3回操作して保存(保存時点 = 3) → 移動(4) → Ctrl + Z(3) → 保存時点と同じなので未保存ではない
    m_savedSceneChangeCount = m_sceneChangeCount;
}
void FWK::Editor::EditorUndoRedoSystem::MarkSceneModified()
{
    // Undo/Redoできない操作でシーンを変えたときに呼ぶ
    // どの回数とも一致しない値にして、保存するまで未保存のままにする
    m_savedSceneChangeCount = k_unreachableSavedSceneChangeCount;
}

bool FWK::Editor::EditorUndoRedoSystem::IsSceneModified() const
{
    return m_sceneChangeCount != m_savedSceneChangeCount;
}

nlohmann::json FWK::Editor::EditorUndoRedoSystem::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::EditorUndoRedoSystem::AddUndoCommand(std::unique_ptr<ICommand>&& a_command)
{
    if (!a_command) { return; }

    // 保存した時点がRedo履歴の側にある(Undoで保存より前へ戻っている)ときに新しい操作をすると、
    // Redo履歴と一緒に保存した時点へは二度と戻れなくなる
    // 例 : 2回操作して保存(保存時点 = 2) → Ctrl + Z(1) → 新しい操作(2)
    //      回数は2で同じだが中身は保存したときと違うので、保存時点をどの回数とも一致しない値にする
    if (!m_redoList.empty() &&
        m_savedSceneChangeCount > m_sceneChangeCount)
    {
        m_savedSceneChangeCount = k_unreachableSavedSceneChangeCount;
    }

    // 新規操作が入った時点でRedo履歴は破棄する
    m_redoList.clear();

    // 選択の変更のようにシーンを変えない操作は数えない
    if (IsSceneCommand(*a_command))
    {
        ++m_sceneChangeCount;
    }

    m_undoList.emplace_back(std::move(a_command));

    // 上限を超えた分は古いものから捨てる
    // 削除のコマンドは消したGameObjectのJsonを持っているため、捨てないとメモリが増え続ける
    // 例 : 上限500で501個目を積む → 一番古い1個を捨てて500個に戻す
    while (m_undoList.size() > m_capacity)
    {
        m_undoList.pop_front();
    }
}
bool FWK::Editor::EditorUndoRedoSystem::IsSceneCommand(const ICommand& a_command)
{
    // SceneCommandBaseを継承したコマンドだけをシーンの変更として数える
    // ICommandはシーンを知らない汎用の型のままにしておき、シーンを変えるかは継承した型で見分ける
    // 例 : CreateGameObjectSceneCommand(SceneCommandBaseを継承) → true
    //      ChangeSelectionCommand      (ICommandを直接継承)     → false
    // 実際の型の型情報から親を順に辿り、SceneCommandBaseの型情報と一致すればシーンを変えるコマンド
    // dynamic_castは実行時にRTTIの継承表を探すため重い。型情報なら番号の比較を親の数だけ行うだけで済む
    // 例 : CreateGameObjectSceneCommand → SceneCommandBase(一致) → true
    //      ChangeSelectionCommand       → ICommand → nullptr(一致なし) → false
    return Utility::IsDerivedTypeINFO(a_command.GetREFRuntimeTypeINFO(), SceneCommandBase::GetREFTypeINFO());
}