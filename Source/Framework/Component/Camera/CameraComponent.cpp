#include "CameraComponent.h"
#include "../../../Application/Application.h"

void FWK::CameraComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }
}

void FWK::CameraComponent::PostDeserialize()
{
    m_camera.Setup();
}

void FWK::CameraComponent::EarlyUpdate()
{
    const auto& l_application = Application::GetInstance    ();
    const auto& l_window = l_application.GetREFWindow  ();

    // リサイズ申請がされたときのみにアスペクト比率を更新する
    if (const auto& l_resizeRequest = l_window.GetREFResizeRequest();
        !l_resizeRequest.m_isRequested)
    {
        return;
    }

    m_camera.SetAspectRatio(l_window.GetVALAspectRatio());
}
void FWK::CameraComponent::PostLateUpdate()
{

}

void FWK::CameraComponent::EditInspector()
{

}

nlohmann::json FWK::CameraComponent::Serialize()
{
    return nlohmann::json();
}