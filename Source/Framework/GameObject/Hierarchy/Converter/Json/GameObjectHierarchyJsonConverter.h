#pragma once

namespace FWK
{
    class GameObjectHierarchy;
}

namespace FWK::Converter
{
    class GameObjectHierarchyJsonConverter final
    {
    public:

         GameObjectHierarchyJsonConverter() = default;
        ~GameObjectHierarchyJsonConverter() = default;
    
        void Deserialize(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy);

        nlohmann::json Serialize(const GameObjectHierarchy& a_gameObjectHierarchy) const;
    };
}