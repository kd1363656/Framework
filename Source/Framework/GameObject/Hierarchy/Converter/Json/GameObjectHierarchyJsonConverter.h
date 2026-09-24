#pragma once

namespace FWK
{
    class GameObject;
    class GameObjectHierarchy;
    class Scene;
}

namespace FWK::Converter
{
    class GameObjectHierarchyJsonConverter final
    {
    public:

         GameObjectHierarchyJsonConverter() = default;
        ~GameObjectHierarchyJsonConverter() = default;

        bool DeserializePrefab(const nlohmann::json&                      a_rootJson,
                               std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet,
                               std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
                               Scene&                                     a_scene) const;

        bool DeserializeScene(const nlohmann::json&                      a_rootJson,
                              std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet,
                              std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
                              GameObjectHierarchy&                       a_gameObjectHierarchy,
                              Scene&                                     a_scene) const;

        nlohmann::json SerializePrefab(const GameObjectHierarchy& a_gameObjectHierarchy) const;
        nlohmann::json SerializeScene (const GameObjectHierarchy& a_gameObjectHierarchy) const;

    private:

        nlohmann::json SerializeRemovedUUIDList(const GameObjectHierarchy& a_gameObjectHierarchy) const;

        static constexpr std::string_view k_removedChildNodeUUIDListJsonKey = "RemovedChildNodeUUIDList";
    };
}