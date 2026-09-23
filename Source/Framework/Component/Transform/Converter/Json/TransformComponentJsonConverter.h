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

        void DeserializePrefab(const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const;
        void DeserializeScene (const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const;

        nlohmann::json SerializePrefab(const TransformComponent& a_transformComponent) const;
        nlohmann::json SerializeScene (const TransformComponent& a_transformComponent) const;

    private:

        void CommonDeserialize(const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const;

        nlohmann::json CommonSerialize(const TransformComponent& a_transformComponent) const;

        static constexpr std::string_view k_scaleJsonKey                  = "Scale";
        static constexpr std::string_view k_rotationJsonKey               = "Rotation";
        static constexpr std::string_view k_positionJsonKey               = "Position";
        static constexpr std::string_view k_matrixStrategyTypeNameJsonKey = "MatrixStrategyType";
    };
}