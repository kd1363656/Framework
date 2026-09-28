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
    
        void DeserializeScene (const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy);
        void DeserializePrefab(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy);

        nlohmann::json Serialize(const GameObjectHierarchy& a_gameObjectHierarchy) const;

    private:

        void DeserializePrefabRemovedChildUUIDSet(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy) const;

        void DeserializeChildGameObjectList(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy) const;

        nlohmann::json SerializePrefabRemovedChildUUIDSet(const GameObjectHierarchy& a_gameObjectHierarchy) const;

        nlohmann::json SerializeChildPrefabHierarchyNodeUUIDList(const GameObjectHierarchy& a_gameObjectHierarchy) const;

        nlohmann::json SerializeChildGameObjectList(const GameObjectHierarchy& a_gameObjectHierarchy) const;

        static constexpr std::string_view k_childPrefabHierarchyNodeUUIDListJsonKey = "ChildPrefabHierarchyNodeUUIDList";
        static constexpr std::string_view k_prefabRemovedChildUUIDSetJsonKey        = "PrefabRemovedChildUUIDSet";
        static constexpr std::string_view k_childGameObjectListJsonKey              = "ChildGameObjectList";
        static constexpr std::string_view k_uuidJsonKey                             = "UUID";
    };
}