#include "DetailsEditorWindow.h"

void FWK::Editor::DetailsEditorWindow::Draw(EditorManager& a_editorManager)
{
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    // このウィンドウがクリックされているかどうかを判定
    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // EditorManagerが保持するGameObject選択状態を取得
    // ヒエラルキー(Outliner)とビューポートで選択状態を共有している
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();

    // 破棄済み、参照切れのGameObjectを選択状態から取り除く
    l_gameObjectSelectionState.SweepUnavailableGameObjects();

    // 最後に選択したGameObjectを取得
    // 複数選択時は最後に選択したGameObjectのみインスペクター描画の対象にする
    const auto& l_selectedGameObject = l_gameObjectSelectionState.FindVALLastSelectedGameObject().lock();
 
    // 未選択、または無効なら何も描画しない
    if (!l_selectedGameObject ||
        l_selectedGameObject->GetVALIsDestroyed())
    {
        // 選択しているゲームオブジェクトがないことを描画
        ImGui::TextDisabled(k_noSelectionLabel.data());

        ImGui::End();

        return;
    }

    m_gameObjectDetails.Draw(*l_selectedGameObject);

    ImGui::End();
}