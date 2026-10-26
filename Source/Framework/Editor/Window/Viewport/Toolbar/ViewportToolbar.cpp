#include "ViewportToolbar.h"

void FWK::Editor::ViewportToolbar::Draw(EditorManager& a_editorManager)
{
    // ▶と■は、ビューポートのウィンドウ全体で見た左右の真ん中に置く
    // (BeginChildの後はGetWindowPosが子ウィンドウの位置を返すため、子ウィンドウを始める前に求めておく)
    const float l_windowCenterX = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() * k_half;

    if (const auto& l_toolbarSize = ImVec2{ ImGui::GetContentRegionAvail().x, k_toolbarHeight };
        !ImGui::BeginChild(k_toolbarChildID.data(),
                           l_toolbarSize,
                           false,
                           ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::EndChild();
        return;
    }

    DrawDisplayOptionsButton();

    DrawPlayButtonGroup(a_editorManager, l_windowCenterX);

    ImGui::EndChild();
}

void FWK::Editor::ViewportToolbar::DrawDisplayOptionsButton() const
{
    // ▶と■と高さを揃えるため、高さだけ指定する(幅は文字に合わせる)
    if (ImGui::Button(k_displayOptionsButtonLabel.data(), { k_fitToLabelWidth, k_toolbarButtonSize }))
    {
        ImGui::OpenPopup(k_displayOptionsPopupID.data());
    }

    DrawDisplayOptionsPopup();
}
void FWK::Editor::ViewportToolbar::DrawDisplayOptionsPopup() const
{
    if (!ImGui::BeginPopup(k_displayOptionsPopupID.data())) { return; }

    DrawCollisionCheckbox ();
    DrawRenderPassSettings();

    ImGui::EndPopup();
}
void FWK::Editor::ViewportToolbar::DrawCollisionCheckbox() const
{
    auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

    // 描かない設定(IsDisableDebugDraw)の逆が、チェックを付ける状態(可視化する)になる
    bool l_isEnabled = !l_physicsManager.GetVALIsDisableDebugDraw();

    // ImGui::Checkbox(表示する文字列、チェックの状態を読み書きする変数のアドレス)
    // クリックされてチェックの状態が変わったフレームだけtrueを返す
    if (ImGui::Checkbox(k_collisionCheckboxLabel.data(), &l_isEnabled))
    {
        l_physicsManager.TogglePhysicsDebugDraw();
    }
}
void FWK::Editor::ViewportToolbar::DrawRenderPassSettings() const
{
    auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance ();
    auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer();
    auto& l_renderGraph     = l_renderer.GetMutableREFRenderGraph    ();

    // 描画の設定を持つPassだけ、設定を編集できるようにする
    auto* const l_staticModelStandardLitPass = l_renderGraph.FindMutablePTRPass<Graphics::StaticModelStandardLitPass>();

    if (!l_staticModelStandardLitPass) { return; }

    ImGui::Separator();

    l_staticModelStandardLitPass->EditInspector();
}

void FWK::Editor::ViewportToolbar::DrawPlayButtonGroup(EditorManager& a_editorManager, const float a_windowCenterX) const
{
    // 表示ボタンの右へ続けて置く位置(行の高さと、表示ボタンにぶつからない一番左のX座標が分かる)
    ImGui::SameLine();

    const ImVec2 l_afterDisplayButtonPosition = ImGui::GetCursorScreenPos();

    // ボタン2つの幅と、ボタンの間1つ分の隙間
    const float l_playGroupWidth = k_iconButtonSize.x * k_playButtonCount + k_buttonSpacing * k_playButtonGapCount;

    // 真ん中から半分の幅だけ左へずらすと、まとまりの中心がウィンドウの真ん中に来る
    // ウィンドウが狭い時は、表示ボタンに重ならないよう表示ボタンの右で止める
    const float l_playGroupLeftX = std::max(a_windowCenterX - l_playGroupWidth * k_half, l_afterDisplayButtonPosition.x);

    ImGui::SetCursorScreenPos({ l_playGroupLeftX, l_afterDisplayButtonPosition.y });

    const auto& l_sceneManager = SceneManager::GetInstance();
    const bool  l_isPlaying    = l_sceneManager.FetchVALIsPlaying();

    // ▶ : 再生中は停止中の■と同じく、暗くして押せないようにする
    if (Utility::IMGUIIconButton(k_playIconLabel,
                                 k_playTooltip,
                                 k_iconButtonSize,
                                 k_isNotHighlighted,
                                 !l_isPlaying))
    {
        BeginPlay();
    }

    ImGui::SameLine(k_sameLineOffsetFromStartX, k_buttonSpacing);

    // ■ : ▶を押した後(再生中)だけ押せる
    if (Utility::IMGUIIconButton(k_stopIconLabel,
                                 k_stopTooltip,
                                 k_iconButtonSize,
                                 k_isNotHighlighted,
                                 l_isPlaying))
    {
        EndPlay(a_editorManager);
    }
}

void FWK::Editor::ViewportToolbar::BeginPlay() const
{
    auto& l_sceneManager = SceneManager::GetInstance();

    l_sceneManager.BeginPlay();
}
void FWK::Editor::ViewportToolbar::EndPlay(EditorManager& a_editorManager) const
{
    auto& l_sceneManager             = SceneManager::GetInstance                         ();
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();

    // シーンを作り直すと選択していたGameObjectは別物になるため、UUIDで覚えておき、作り直した後に選び直す
    std::vector<boost::uuids::uuid> l_selectedUUIDList = {};
    boost::uuids::uuid              l_anchorUUID       = {};

    Utility::FetchEditorGameObjectSelectionSnapshot(l_gameObjectSelectionState, l_selectedUUIDList, l_anchorUUID);

    if (!l_sceneManager.EndPlay()) { return; }

    l_gameObjectSelectionState.RestoreState(l_selectedUUIDList, l_anchorUUID);
}
