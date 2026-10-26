#include "ViewportDebugCameraEditInspector.h"

void FWK::Editor::ViewportDebugCameraEditInspector::EditInspector(ViewportDebugCamera& a_debugCamera) const
{
    ImGui::SeparatorText(k_titleLabel.data());

    if (!Constant::k_isViewportDebugCameraAvailable)
    {
        ImGui::TextUnformatted(k_unavailableLabel.data());

        return;
    }

    if (bool l_isEnabled = a_debugCamera.GetVALIsEnabled();
        ImGui::Checkbox(k_enableLabel.data(), &l_isEnabled))
    {
        a_debugCamera.SetIsEnabled(l_isEnabled);
    }

    Utility::IMGUIDelayedTooltip(k_enableTooltip);

    // ImGui::DragFloat(表示する文字列、値のアドレス、1ピクセル動かしたときに変わる量、最小値、最大値)
    if (float l_moveSpeed = a_debugCamera.GetVALMoveSpeed();
        ImGui::DragFloat(k_moveSpeedLabel.data(), &l_moveSpeed, k_moveSpeedDragStep, k_minMoveSpeed, k_maxMoveSpeed))
    {
        a_debugCamera.SetMoveSpeed(std::clamp(l_moveSpeed, k_minMoveSpeed, k_maxMoveSpeed));
    }

    if (float l_rotationSpeed = a_debugCamera.GetVALRotationSpeedDegree();
        ImGui::DragFloat(k_rotationSpeedLabel.data(), &l_rotationSpeed, k_rotationSpeedDragStep, k_minRotationSpeed, k_maxRotationSpeed))
    {
        a_debugCamera.SetRotationSpeedDegree(std::clamp(l_rotationSpeed, k_minRotationSpeed, k_maxRotationSpeed));
    }

    if (bool l_isCullingDebugVisualize = a_debugCamera.GetVALIsCullingDebugVisualize();
        ImGui::Checkbox(k_cullingVisualizeLabel.data(), &l_isCullingDebugVisualize))
    {
        a_debugCamera.SetIsCullingDebugVisualize(l_isCullingDebugVisualize);
    }

    Utility::IMGUIDelayedTooltip(k_cullingVisualizeTooltip);

    if (bool l_isDrawMainCameraFrustum = a_debugCamera.GetVALIsDrawMainCameraFrustum();
        ImGui::Checkbox(k_drawFrustumLabel.data(), &l_isDrawMainCameraFrustum))
    {
        a_debugCamera.SetIsDrawMainCameraFrustum(l_isDrawMainCameraFrustum);
    }

    if (ImGui::Button(k_moveToMainCameraLabel.data()))
    {
        MoveToMainCamera(a_debugCamera);
    }

    DrawCullingLegend();
}

void FWK::Editor::ViewportDebugCameraEditInspector::DrawCullingLegend() const
{
    ImGui::SeparatorText(k_legendTitleLabel.data());

    ImGui::TextColored(k_meshFrustumCulledColor,     "%s", k_legendMeshFrustumLabel.data());
    ImGui::TextColored(k_meshOccludedColor,          "%s", k_legendMeshOccludedLabel.data());
    ImGui::TextColored(k_meshletFrustumCulledColor,  "%s", k_legendMeshletFrustumLabel.data());
    ImGui::TextColored(k_meshletBackfaceCulledColor, "%s", k_legendMeshletBackfaceLabel.data());

    ImGui::TextDisabled("%s", k_legendOcclusionNoteLabel.data());
}

void FWK::Editor::ViewportDebugCameraEditInspector::MoveToMainCamera(ViewportDebugCamera& a_debugCamera) const
{
    const auto& l_graphicsManager       = Graphics::GraphicsManager::GetInstance                                 ();
    const auto& l_renderer              = l_graphicsManager.GetREFRenderer                                       ();
    const auto& l_renderGraph           = l_renderer.GetREFRenderGraph                                           ();
    const auto& l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<Graphics::CameraPassDrawRequest>().lock();

    if (!l_cameraPassDrawRequest) { return; }

    const auto& l_mainCameraConstantBuffer = l_cameraPassDrawRequest->GetREFSourceConstantBuffer().lock();

    if (!l_mainCameraConstantBuffer) { return; }

    // ビュー行列の逆行列がカメラ行列(カメラのワールド行列)
    a_debugCamera.MoveToCameraMatrix(l_mainCameraConstantBuffer->m_viewMatrix.Invert());
}
