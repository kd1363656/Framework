#include "CameraComponentJsonConverter.h"

void FWK::Converter::CameraComponentJsonConverter::Deserialize(const nlohmann::json& a_rootJson, CameraComponent& a_cameraComponent) const
{
    if (a_rootJson.is_null()) { return; }

    a_cameraComponent.ComponentBase::Deserialize(a_rootJson);
}

nlohmann::json FWK::Converter::CameraComponentJsonConverter::Serialize(const CameraComponent& a_cameraComponent) const
{
    nlohmann::json l_rootJson = a_cameraComponent.ComponentBase::Serialize();

    return l_rootJson;
}