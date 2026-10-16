#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Converter
{
    class GameObjectJsonConverter final
    {
    public:

         GameObjectJsonConverter() = default;
        ~GameObjectJsonConverter() = default;
    
        void DeserializeScene(const nlohmann::json&              a_rootJson,
                              const nlohmann::json&              a_baseJson,
                              const std::weak_ptr<GameObject>&   a_gameObject, 
                              const SceneGameObjectPrefabSystem& a_prefabSystem,
                                    Scene&                       a_scene) const;
 
        // Prefabファイル内ノード用。常にフル形式
        void DeserializePrefab(const nlohmann::json&              a_rootJson,
                               const std::weak_ptr<GameObject>& a_gameObject,
                               const SceneGameObjectPrefabSystem& a_prefabSystem,
                                     Scene&                       a_scene) const;
 
        nlohmann::json Serialize     (const GameObject&     a_gameObject,       SceneGameObjectPrefabSystem& a_prefabSystem)                                            const;
        nlohmann::json SerializeScene(const GameObject&     a_gameObject,       SceneGameObjectPrefabSystem& a_prefabSystem)                                            const;
        nlohmann::json SerializeDIFF (const nlohmann::json& a_baseJson,   const GameObject&                  a_gameObject, SceneGameObjectPrefabSystem& a_prefabSystem) const;

        static const nlohmann::json* FindPTRPrefabNodeJson(const nlohmann::json& a_prefabJson, const boost::uuids::uuid& a_nodeUUID);
 
    private:

       void DeserializeCommon(const nlohmann::json& a_rootJson, const nlohmann::json& a_baseJson, GameObject& a_gameObject) const;

       static constexpr std::string_view k_transformComponentJsonKey      = "TransformComponent";
       static constexpr std::string_view k_isPrefabOriginJsonKey          = "IsPrefabOrigin";
       static constexpr std::string_view k_prefabUUIDJsonKey              = "PrefabUUID";
       static constexpr std::string_view k_prefabHierarchyNodeUUIDJsonKey = "PrefabHierarchyNodeUUID";
       static constexpr std::string_view k_diffJsonKey                    = "Diff";
    };
}