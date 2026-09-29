#pragma once

namespace FWK
{
    class GameObjectComponentContainer;
}

namespace FWK::Converter
{
    class GameObjectComponentContainerJsonConverter final
    {
    public:

         GameObjectComponentContainerJsonConverter() = default;
        ~GameObjectComponentContainerJsonConverter() = default;
    
        void DeserializeScene (const nlohmann::json& a_rootJson, const nlohmann::json&               a_prefabJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const;
        void DeserializePrefab(const nlohmann::json& a_rootJson,       GameObjectComponentContainer& a_gameObjectComponentContainer)                                             const;
 
        nlohmann::json Serialize    (const GameObjectComponentContainer& a_gameObjectComponentContainer)                                                   const;
        nlohmann::json SerializeDiff(const nlohmann::json&               a_prefabJson, const GameObjectComponentContainer& a_gameObjectComponentContainer) const;
 
    private:

        void DeserializeComponentList  (const nlohmann::json& a_componentListJson,     GameObjectComponentContainer& a_gameObjectComponentContainer) const;
        void DeserializeRemovedUUIDList(const nlohmann::json& a_componentListDiffJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const;

        nlohmann::json SerializeComponentList  (const GameObjectComponentContainer&           a_gameObjectComponentContainer) const;
        nlohmann::json SerializeRemovedUUIDList(const std::unordered_set<boost::uuids::uuid>& a_removedUUIDSet)               const;

        nlohmann::json DetectComponentListDiff(const nlohmann::json& a_baseJson, const nlohmann::json& a_currentJson) const;
 
        void ApplyComponentListDiff(const nlohmann::json& a_diffJson, nlohmann::json& a_baseJson) const;

        static constexpr std::string_view k_addedJsonKey           = "Added";
        static constexpr std::string_view k_orderUUIDListJsonKey   = "OrderUUIDList";
        static constexpr std::string_view k_removedUUIDListJsonKey = "RemovedUUIDList";
        static constexpr std::string_view k_removedUUIDJsonKey     = "RemovedUUID";
        static constexpr std::string_view k_modifiedJsonKey        = "Modified";
        static constexpr std::string_view k_componentUUIDJsonKey   = "ComponentUUID";
        static constexpr std::string_view k_componentListJsonKey   = "ComponentList";
        static constexpr std::string_view k_componentTypeJsonKey   = "ComponentType";
        static constexpr std::string_view k_componentDataJsonKey   = "ComponentData";

        static constexpr std::size_t k_initialComponentListOrderIndex = 0U;
    };
}