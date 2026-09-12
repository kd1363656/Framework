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

        void DeserializePrefab(const nlohmann::json& a_rootJson, CameraComponent& a_cameraComponent) const;

        nlohmann::json SerializePrefab(CameraComponent& a_cameraComponent) const;

    private:

        static constexpr std::string_view k_fovYDegreeJsonKey  = "FovYDegree";
        static constexpr std::string_view k_farClipJsonKey     = "FarClip";
        static constexpr std::string_view k_nearClipJsonKey    = "NearClip";
    };
}