#include "ViewportToolbar.h"

void FWK::Editor::ViewportToolbar::Draw(ViewportEditorWindow& a_viewportEditorWindow)
{
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

    DrawDisplayOptionsButton(a_viewportEditorWindow);

    ImGui::SameLine();

    DrawCameraButton(a_viewportEditorWindow);

    ImGui::EndChild();
}

void FWK::Editor::ViewportToolbar::DrawDisplayOptionsButton(ViewportEditorWindow& a_viewportEditorWindow)
{
    if (ImGui::Button(k_displayOptionsButtonLabel.data()))
    {
        ImGui::OpenPopup(k_displayOptionsPopupID.data());
    }

    DrawDisplayOptionsPopup(a_viewportEditorWindow);
}
void FWK::Editor::ViewportToolbar::DrawDisplayOptionsPopup(ViewportEditorWindow& a_viewportEditorWindow)
{
    if (!ImGui::BeginPopup(k_displayOptionsPopupID.data())) { return; }

    DrawCollisionMenuItem   ();
    DrawFrustumMenuItem     (a_viewportEditorWindow);
    DrawCulledResultMenuItem(a_viewportEditorWindow);

    ImGui::EndPopup();
}
void FWK::Editor::ViewportToolbar::DrawCollisionMenuItem() const
{
    // PhysicsManagerは「デバッグ描画が無効かどうか」(IsDisable)を保持しているため
    // メニューのチェック表示(有効かどうか)として使うには反転させる
          auto& l_physicsManager = Physics::PhysicsManager::GetInstance      ();
    const bool  l_isEnabled      = !l_physicsManager.GetVALIsDisableDebugDraw();

    if (ImGui::MenuItem(k_collisionMenuItemLabel.data(), nullptr, l_isEnabled))
    {
        l_physicsManager.TogglePhysicsDebugDraw();
    }
}
void FWK::Editor::ViewportToolbar::DrawFrustumMenuItem(ViewportEditorWindow& a_viewportEditorWindow) const
{
    if (const bool l_isDrawFrustum = a_viewportEditorWindow.GetVALIsDrawFrustum();
        ImGui::MenuItem(k_drawFrustumMenuItemLabel.data(), nullptr, l_isDrawFrustum))
    {
        a_viewportEditorWindow.SetIsDrawFrustum(!l_isDrawFrustum);
    }
}
void FWK::Editor::ViewportToolbar::DrawCulledResultMenuItem(ViewportEditorWindow& a_viewportEditorWindow) const
{
    if (const bool l_isDrawCulledResult = a_viewportEditorWindow.GetVALIsDrawCulledResult();
        ImGui::MenuItem(k_drawCulledResultMenuItemLabel.data(), nullptr, l_isDrawCulledResult))
    {
        a_viewportEditorWindow.SetIsDrawCulledResult(!l_isDrawCulledResult);
    }
}

void FWK::Editor::ViewportToolbar::DrawCameraButton(ViewportEditorWindow& a_viewportEditorWindow)
{
    if (ImGui::Button(k_cameraOptionsButtonLabel.data()))
    {
        ImGui::OpenPopup(k_cameraOptionsPopupID.data());
    }

    if (const auto& l_editorCamera = a_viewportEditorWindow.GetMutableREFEditorCamera();
        l_editorCamera)
    {
        DrawCameraPopup(*l_editorCamera);
    }
}
void FWK::Editor::ViewportToolbar::DrawCameraPopup(EditorCamera& a_editorCamera)
{
    if (!ImGui::BeginPopup(k_cameraOptionsPopupID.data())) { return; }

    const auto& l_camera = a_editorCamera.GetREFCamera();

    // 視野角の変更
    if (float l_fovYDegree = l_camera.GetVALFovYDegree();
        ImGui::DragFloat(k_fovYDegreeLabel.data(),
                         &l_fovYDegree,
                         Constant::k_imguiDefaultDragValue,
                         k_fovYDegreeMIN,
                         k_fovYDegreeMAX))
    {
        a_editorCamera.ApplyFovYDegree(l_fovYDegree);
    }

    // カメラ移動速度の変更
    if (float l_moveSpeed = a_editorCamera.GetVALMoveSpeed();
        ImGui::DragFloat(k_moveSpeedLabel.data(),
                         &l_moveSpeed,
                         Constant::k_imguiDefaultDragValue,
                         k_moveSpeedMIN,
                         k_moveSpeedMAX))
    {
        a_editorCamera.SetMoveSpeed(l_moveSpeed);
    }

    // カメラ回転速度の変更
    if (float l_rotateSpeed = a_editorCamera.GetVALRotateSpeed();
        ImGui::DragFloat(k_rotateSpeedLabel.data(),
                         &l_rotateSpeed,
                         k_rotateSpeedDragValue,
                         k_rotateSpeedMIN,
                         k_rotateSpeedMAX,
                         "%.4f"))
    {
        a_editorCamera.SetRotateSpeed(l_rotateSpeed);
    }

    ImGui::EndPopup();
}