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
    
        void DeserializeScene (const nlohmann::json& a_rootJson, const std::weak_ptr<GameObject>& a_gameObject) const;
        void DeserializePrefab(const nlohmann::json& a_rootJson, const std::weak_ptr<GameObject>& a_gameObject) const;

        nlohmann::json Serialize(const GameObject& a_gameObject) const;

    private:

        void DeserializeCommon(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const;

        static constexpr std::string_view k_nameJsonKey                    = "Name";
        static constexpr std::string_view k_prefabUUIDJsonKey              = "PrefabUUID";
        static constexpr std::string_view k_prefabHierarchyNodeUUIDJsonKey = "PrefabHierarchyNodeUUID";
        static constexpr std::string_view k_sceneInstanceUUIDJsonKey       = "SceneInstanceUUID";
        static constexpr std::string_view k_isPrefabOriginJsonKey          = "IsPrefabOrigin";
        static constexpr std::string_view k_transformJsonKey               = "Transform";
        static constexpr std::string_view k_componentContainerJsonKey      = "ComponentContainer";
        static constexpr std::string_view k_hierarchyJsonKey               = "Hierarchy";
    };
}