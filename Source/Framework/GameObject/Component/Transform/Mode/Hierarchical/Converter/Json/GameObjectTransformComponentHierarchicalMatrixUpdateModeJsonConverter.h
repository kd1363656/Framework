#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateHierarchicalMode;
}

namespace FWK::Converter
{
    class GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter final
    {
    public:

         GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter() = default;
        ~GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, GameObjectTransformComponentMatrixUpdateHierarchicalMode& a_gameObjectTransformComponentMatrixUpdateHierarchicalMode) const;
        
        nlohmann::json Serialize(const GameObjectTransformComponentMatrixUpdateHierarchicalMode& a_gameObjectTransformComponentMatrixUpdateHierarchicalMode) const;

    private:

        static constexpr std::string_view k_calculateParentWorldMatrixEnumBitShiftJsonKey = "CalculateParentWorldMatrixEnumBitShift";
    };
}