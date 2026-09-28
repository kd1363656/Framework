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

    private:

        static constexpr std::string_view k_fovYDegreeJsonKey = "FovYDegree";
        static constexpr std::string_view k_farClipJsonKey    = "FarClip";
        static constexpr std::string_view k_nearClipJsonKey   = "NearClip";
    };
}