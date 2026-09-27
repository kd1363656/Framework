#pragma once

namespace FWK
{
    class GameObjectCameraComponent;
}

namespace FWK::Converter
{
    class GameObjectCameraComponentJsonConverter final
    {
    public:

         GameObjectCameraComponentJsonConverter() = default;
        ~GameObjectCameraComponentJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, GameObjectCameraComponent& a_gameObjectCameraComponent) const;

        nlohmann::json Serialize(const GameObjectCameraComponent& a_gameObjectCameraComponent) const;
    };
}