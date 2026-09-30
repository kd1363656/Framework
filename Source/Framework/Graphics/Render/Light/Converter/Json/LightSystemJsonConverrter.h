#pragma once

namespace FWK
{
    class Graphics::LightSystem;
}

namespace FWK::Converter
{
    class LightSystemJsonConverter final
    {
    public:

         LightSystemJsonConverter() = default;
        ~LightSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::LightSystem& a_lightSystem) const;

        nlohmann::json Serialize(const Graphics::LightSystem& a_lightSystem) const;

    private:

        static constexpr std::string_view k_directionalLightJsonKey = "DirectionalLight";
        static constexpr std::string_view k_ambientLightJsonKey     = "AmbientLight";
        static constexpr std::string_view k_directionJsonKey        = "Direction";
        static constexpr std::string_view k_colorJsonKey            = "Color";
        static constexpr std::string_view k_intensityJsonKey        = "Intensity";
    };
}