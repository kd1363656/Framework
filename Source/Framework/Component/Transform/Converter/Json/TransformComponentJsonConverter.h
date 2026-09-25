#pragma once

namespace FWK
{
    class TransformComponent;
}

namespace FWK::Converter
{
    class TransformComponentJsonConverter final
    {
    public:

         TransformComponentJsonConverter() = default;
        ~TransformComponentJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const;

        nlohmann::json Serialize(const TransformComponent& a_transformComponent) const;

    private:
    
        static constexpr std::string_view k_scaleJsonKey                  = "Scale";
        static constexpr std::string_view k_rotationJsonKey               = "Rotation";
        static constexpr std::string_view k_positionJsonKey               = "Position";
        static constexpr std::string_view k_matrixStrategyTypeNameJsonKey = "MatrixStrategyType";
    };
}