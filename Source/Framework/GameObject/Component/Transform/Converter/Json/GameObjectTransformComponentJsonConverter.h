#pragma once

namespace FWK
{
    class GameObjectTransformComponent;
}

namespace FWK::Converter
{
    class GameObjectTransformComponentJsonConverter final
    {
    public:

         GameObjectTransformComponentJsonConverter() = default;
        ~GameObjectTransformComponentJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, GameObjectTransformComponent& a_gameObjectTransformComponent) const;

        nlohmann::json Serialize(const GameObjectTransformComponent& a_gameObjectTransformComponent) const;

    private:
    
        static constexpr std::string_view k_scaleJsonKey                = "Scale";
        static constexpr std::string_view k_rotationJsonKey             = "Rotation";
        static constexpr std::string_view k_positionJsonKey             = "Position";
        static constexpr std::string_view k_matrixUpdateModeBaseJsonKey = "MatrixUpdateModeBase";
    };
}