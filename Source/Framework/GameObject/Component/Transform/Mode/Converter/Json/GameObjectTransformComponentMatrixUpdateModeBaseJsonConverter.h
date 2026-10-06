#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateModeBase;
}

namespace FWK::Converter
{
    class GameObjectTransformComponentMatrixUpdateModeBaseJsonConverter final
    {
    public:

         GameObjectTransformComponentMatrixUpdateModeBaseJsonConverter() = default;
        ~GameObjectTransformComponentMatrixUpdateModeBaseJsonConverter() = default;
    
        void Deserialize(const nlohmann::json& a_rootJson, GameObjectTransformComponentMatrixUpdateModeBase& a_gameObjectTransformComponentMatrixUpdateModeBase) const;

        nlohmann::json Serialize(const GameObjectTransformComponentMatrixUpdateModeBase& a_gameObjectTransformComponentMatrixUpdateModeBase) const;

    private:

        static constexpr std::string_view k_isRotateAroundPositionJsonKey = "IsRotateAroundPosition";
    };
}