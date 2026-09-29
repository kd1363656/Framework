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
    
        void DeserializeScene (const nlohmann::json&              a_rootJson,
                               const nlohmann::json&              a_prefabJson,
                               const SceneGameObjectPrefabSystem& a_prefabSystem,
                                     GameObjectHierarchy&         a_gameObjectHierarchy,
                                     Scene&                       a_scene) const;
 
        void DeserializePrefab(const nlohmann::json&              a_rootJson,
                               const SceneGameObjectPrefabSystem& a_prefabSystem,
                                     GameObjectHierarchy&         a_gameObjectHierarchy,
                                     Scene&                       a_scene) const;
 
        nlohmann::json Serialize    (const GameObjectHierarchy& a_gameObjectHierarchy,       SceneGameObjectPrefabSystem& a_prefabSystem)                                                     const;
        nlohmann::json SerializeDiff(const nlohmann::json&      a_prefabJson,          const GameObjectHierarchy&         a_gameObjectHierarchy, SceneGameObjectPrefabSystem& a_prefabSystem) const;
 
    private:

        void DeserializeChildList(const nlohmann::json&              a_childListJson,
                                  const SceneGameObjectPrefabSystem& a_prefabSystem,
                                        GameObjectHierarchy&         a_gameObjectHierarchy,
                                        Scene&                       a_scene) const;
 
 
        void DeserializeChildListDiff(const nlohmann::json&              a_prefabChildListJson,
                                      const nlohmann::json&              a_childListDiffJson,
                                      const SceneGameObjectPrefabSystem& a_prefabSystem,
                                            GameObjectHierarchy&         a_gameObjectHierarchy,
                                            Scene&                       a_scene) const;
 
        void DeserializeChild(const std::weak_ptr<GameObject>& a_parentGameObject,
                              const nlohmann::json&              a_childJson,
                              const nlohmann::json&              a_baseJson,
                              const SceneGameObjectPrefabSystem& a_prefabSystem,
                                    Scene&                       a_scene) const;
 
        void DeserializeRemovedUUIDList(const nlohmann::json& a_childListDiffJson, GameObjectHierarchy& a_gameObjectHierarchy) const;
 
        nlohmann::json SerializeRemovedUUIDList(const std::unordered_set<boost::uuids::uuid>& a_removedUUIDSet) const;
 
        static constexpr std::string_view k_addedJsonKey                   = "Added";
        static constexpr std::string_view k_orderUUIDListJsonKey           = "OrderUUIDList";
        static constexpr std::string_view k_removedUUIDListJsonKey         = "RemovedUUIDList";
        static constexpr std::string_view k_removedUUIDJsonKey             = "RemovedUUID";
        static constexpr std::string_view k_modifiedJsonKey                = "Modified";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDJsonKey = "PrefabHierarchyNodeUUID";
        static constexpr std::string_view k_gameObjectDataJsonKey          = "GameObjectData"
    };
}