#include "ViewportOverlayToolbar.h"

void FWK::Editor::ViewportOverlayToolbar::Draw(const Struct::ViewportScreenRect& a_viewportRect, Enum::ViewportGizmoOperation& a_gizmoOperation, ViewportDebugCamera& a_debugCamera) const
{
    // 絵の上端から少し内側に並べる
    const float l_buttonTopY = a_viewportRect.m_min.y + k_overlayPadding;

    // 右上 : 右端にカメラ▽(デバッグビルドのみ)、その左に移動、回転、拡大
    float l_rightEdgeX = a_viewportRect.m_min.x + a_viewportRect.m_size.x - k_overlayPadding;

    if (Constant::k_isViewportDebugCameraAvailable)
    {
        l_rightEdgeX -= k_debugCameraButtonSize.x;

        DrawDebugCameraButton({ l_rightEdgeX, l_buttonTopY }, a_debugCamera);

        l_rightEdgeX -= k_groupSpacing;
    }

    // ボタン3つの幅と、ボタンの間2つ分の隙間
    const float l_gizmoGroupWidth = k_iconButtonSize.x * k_gizmoOperationButtonCount + k_buttonSpacing * k_gizmoOperationButtonGapCount;

    DrawGizmoOperationButtonGroup({ l_rightEdgeX - l_gizmoGroupWidth, l_buttonTopY }, a_gizmoOperation);
}

void FWK::Editor::ViewportOverlayToolbar::DrawGizmoOperationButtonGroup(const ImVec2& a_position, Enum::ViewportGizmoOperation& a_gizmoOperation) const
{
    ImGui::SetCursorScreenPos(a_position);

    // 選んでいる操作のボタンを青くする
    if (Utility::IMGUIIconButton(k_translateIconLabel,
                                 k_translateTooltip,
                                 k_iconButtonSize,
                                 a_gizmoOperation == Enum::ViewportGizmoOperation::Translate,
                                 k_isEnabled))
    {
        a_gizmoOperation = Enum::ViewportGizmoOperation::Translate;
    }

    ImGui::SameLine(k_sameLineOffsetFromStartX, k_buttonSpacing);

    if (Utility::IMGUIIconButton(k_rotateIconLabel,
                                 k_rotateTooltip,
                                 k_iconButtonSize,
                                 a_gizmoOperation == Enum::ViewportGizmoOperation::Rotate,
                                 k_isEnabled))
    {
        a_gizmoOperation = Enum::ViewportGizmoOperation::Rotate;
    }

    ImGui::SameLine(k_sameLineOffsetFromStartX, k_buttonSpacing);

    if (Utility::IMGUIIconButton(k_scaleIconLabel,
                                 k_scaleTooltip,
                                 k_iconButtonSize,
                                 a_gizmoOperation == Enum::ViewportGizmoOperation::Scale,
                                 k_isEnabled))
    {
        a_gizmoOperation = Enum::ViewportGizmoOperation::Scale;
    }
}

void FWK::Editor::ViewportOverlayToolbar::DrawDebugCameraButton(const ImVec2& a_position, ViewportDebugCamera& a_debugCamera) const
{
    ImGui::SetCursorScreenPos(a_position);

    // カメラと▽を1つのボタンにして、どちらを押しても設定が開くようにする(デバッグカメラを使っている間は青くする)
    if (Utility::IMGUIIconButton(k_debugCameraIconLabel,
                                 k_debugCameraTooltip,
                                 k_debugCameraButtonSize,
                                 a_debugCamera.FetchVALIsActive(),
                                 k_isEnabled))
    {
        ImGui::OpenPopup(k_debugCameraPopupID.data());
    }

    if (!ImGui::BeginPopup(k_debugCameraPopupID.data())) { return; }

    a_debugCamera.EditInspector();

    ImGui::EndPopup();
}
