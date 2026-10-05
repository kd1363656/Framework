#include "GameObjectCameraComponent.h"
#include "../../../../Application/Application.h"

void FWK::GameObjectCameraComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectCameraComponent::PostDeserialize()
{
    m_camera.Setup();
}

void FWK::GameObjectCameraComponent::EarlyUpdate()
{
    const auto& l_application = Application::GetInstance    ();
    const auto& l_window      = l_application.GetREFWindow  ();

    // リサイズ申請がされたときのみにアスペクト比率を更新する
    if (const auto& l_resizeRequest = l_window.GetREFResizeRequest();
        !l_resizeRequest.m_isRequested)
    {
        return;
    }

    m_camera.SetAspectRatio(l_window.GetVALAspectRatio());
}
void FWK::GameObjectCameraComponent::PostLateUpdate()
{
    const auto& l_owner = GetREFOwner().lock();

    FWK_ASSERT_RETURN_IF(!l_owner);

    const auto& l_transformComponent = l_owner->GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_IF(!l_transformComponent);

    m_camera.ApplyCameraMatrix(l_transformComponent->GetREFMatrix());
}

void FWK::GameObjectCameraComponent::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::GameObjectCameraComponent::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::shared_ptr<FWK::GameObjectComponentBase> FWK::GameObjectCameraComponent::Clone() const
{
    auto l_clone = std::make_shared<GameObjectCameraComponent>();

    // jsonに乗る部分はDeserialize/Serializeで一括生成
    l_clone->Deserialize(Serialize());

    return l_clone;
}