#include "GameObjectCameraComponentJsonConverter.h"

void FWK::Converter::GameObjectCameraComponentJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectCameraComponent& a_gameObjectCameraComponent) const
{
    if (a_rootJson.is_null()) { return; }

    a_gameObjectCameraComponent.GameObjectComponentBase::Deserialize(a_rootJson);
}

nlohmann::json FWK::Converter::GameObjectCameraComponentJsonConverter::Serialize(const GameObjectCameraComponent& a_gameObjectCameraComponent) const
{
    nlohmann::json l_rootJson = a_gameObjectCameraComponent.GameObjectComponentBase::Serialize();

    return l_rootJson;
}