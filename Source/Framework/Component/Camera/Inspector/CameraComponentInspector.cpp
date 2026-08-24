#include "CameraComponentInspector.h"

void FWK::CameraComponentInspector::EditInspector(CameraComponent& a_cameraComponent) const
{
	auto& l_camera = a_cameraComponent.GetMutableREFCamera();

	ImGui::BeginDisabled();

	ImGui::Text(k_aspectoRatioLabel.data(), l_camera.GetVALAspectRatio());

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
		l_camera.SetFarClip(l_nearClip);
		l_camera.Setup     ();
	}

	// ファークリップの変更
	if (float l_farClip = l_camera.GetVALFarClip();
		ImGui::DragFloat(k_farClipLabel.data(), &l_farClip, Constant::k_imguiDefaultDragValue))
	{
		l_camera.SetFarClip(l_farClip);
		l_camera.Setup     ();
	}
}