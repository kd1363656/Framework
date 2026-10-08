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

        void DeserializeScene(const std::weak_ptr<GameObject>&   a_gameObject,
                              const nlohmann::json&              a_rootJson,
                              const nlohmann::json&              a_baseJson,
                              const SceneGameObjectPrefabSystem& a_prefabSystem,
                                    Scene&                       a_scene) const;

        // Prefabファイル内ノード用。常にフル形式
        void DeserializePrefab(const std::weak_ptr<GameObject>& a_gameObject,
                               const nlohmann::json&              a_rootJson,
                               const SceneGameObjectPrefabSystem& a_prefabSystem,
                                     Scene&                       a_scene) const;

        nlohmann::json Serialize     (const GameObject&     a_gameObject,       SceneGameObjectPrefabSystem& a_prefabSystem)                                            const;
        nlohmann::json SerializeScene(const GameObject&     a_gameObject,       SceneGameObjectPrefabSystem& a_prefabSystem)                                            const;
        nlohmann::json SerializeDiff (const nlohmann::json& a_baseJson,   const GameObject&                  a_gameObject, SceneGameObjectPrefabSystem& a_prefabSystem) const;

    private:

       void DeserializeCommon(const nlohmann::json& a_rootJson, const nlohmann::json& a_baseJson, GameObject& a_gameObject) const;

       static constexpr std::string_view k_transformComponentJsonKey      = "TransformComponent";
       static constexpr std::string_view k_isPrefabOriginJsonKey          = "IsPrefabOrigin";
       static constexpr std::string_view k_prefabHierarchyNodeUUIDJsonKey = "PrefabHierarchyNodeUUID";
       static constexpr std::string_view k_diffJsonKey                    = "Diff";
    };
}