#include "GameObjectCameraComponentInspector.h"

void FWK::GameObjectCameraComponentInspector::EditInspector(GameObjectCameraComponent& a_cameraComponent) const
{
    auto& l_camera = a_cameraComponent.GetMutableREFCamera();

    ImGui::BeginDisabled();

    ImGui::Text(k_aspectRatioLabel.data(), l_camera.GetVALAspectRatio());

    ImGui::EndDisabled();

    // 視野角の値の変更
    if (float l_fovYDegree = l_camera.GetVALFovYDegree();
        ImGui::DragFloat(k_fovYDegreeLabel.data(), &l_fovYDegree, Constant::k_imguiDefaultDragValue))
    {
        l_camera.SetFovYDegree(l_fovYDegree);
        l_camera.Setup        ();
    }

    // ニアークリップの変更
    if (float l_nearClip = l_camera.GetVALNearClip();
        ImGui::DragFloat(k_nearClipLabel.data(), &l_nearClip, Constant::k_imguiDefaultDragValue))
    {
        // 0.0以下になられるとパースペクティブFOVを作成できないから
        if (l_nearClip <= Constant::k_cameraDefaultNearClip)
        {
            l_nearClip = Constant::k_cameraDefaultNearClip;
        }

        l_camera.SetNearClip(l_nearClip);
        l_camera.Setup      ();
    }

    // ファークリップの変更
    if (float l_farClip = l_camera.GetVALFarClip();
        ImGui::DragFloat(k_farClipLabel.data(), &l_farClip, Constant::k_imguiDefaultDragValue))
    {
        l_camera.SetFarClip(l_farClip);
        l_camera.Setup     ();
    }
}