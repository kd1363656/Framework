#include "GameObjectCameraComponentJsonConverter.h"

void FWK::Converter::GameObjectCameraComponentJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectCameraComponent& a_gameObjectCameraComponent) const
{
    if (a_rootJson.is_null()) { return; }

    a_gameObjectCameraComponent.GameObjectComponentBase::Deserialize(a_rootJson);

    auto& l_camera = a_gameObjectCameraComponent.GetMutableREFCamera();

    l_camera.SetFovYDegree (a_rootJson.value(k_fovYDegreeJsonKey,  Constant::k_cameraDefaultFOVYDegree));
    l_camera.SetFarClip    (a_rootJson.value(k_farClipJsonKey,     Constant::k_cameraDefaultFarClip));
    l_camera.SetNearClip   (a_rootJson.value(k_nearClipJsonKey,    Constant::k_cameraDefaultNearClip));
}

nlohmann::json FWK::Converter::GameObjectCameraComponentJsonConverter::Serialize(const GameObjectCameraComponent& a_gameObjectCameraComponent) const
{
    nlohmann::json l_rootJson = a_gameObjectCameraComponent.GameObjectComponentBase::Serialize();

    const auto& l_camera = a_gameObjectCameraComponent.GetREFCamera();

    l_rootJson[k_fovYDegreeJsonKey] = l_camera.GetVALFovYDegree();
    l_rootJson[k_farClipJsonKey]    = l_camera.GetVALFarClip   ();
    l_rootJson[k_nearClipJsonKey]   = l_camera.GetVALNearClip  ();

    return l_rootJson;
}