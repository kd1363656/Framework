#pragma once

namespace FWK
{
    class CameraComponent;
}

namespace FWK::Converter
{
    class CameraComponentJsonConverter final
    {
    public:

         CameraComponentJsonConverter() = default;
        ~CameraComponentJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, CameraComponent& a_cameraComponent) const;

        nlohmann::json Serialize(const CameraComponent& a_cameraComponent) const;
    };
}