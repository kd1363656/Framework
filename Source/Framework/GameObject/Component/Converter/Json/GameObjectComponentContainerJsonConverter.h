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
 
        bool DeserializePrefab(const nlohmann::json& a_rootJson,       GameObjectComponentContainer& a_gameObjectComponentContainer)                                                          const;
        bool DeserializeScene (const nlohmann::json& a_rootJson, const nlohmann::json&               a_prefabComponentListJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const;
        
        nlohmann::json SerializePrefab(const GameObjectComponentContainer& a_gameObjectComponentContainer)                                                                const;
        nlohmann::json SerializeScene (const nlohmann::json&               a_prefabComponentListJson, const GameObjectComponentContainer& a_gameObjectComponentContainer) const;

    private:

        static constexpr std::string_view k_componentTypeNameJsonKey        = "ComponentTypeName";
        static constexpr std::string_view k_componentPrefabJsonKey          = "Prefab";
        static constexpr std::string_view k_componentSceneJsonKey           = "Scene";
        static constexpr std::string_view k_removedComponentUUIDListJsonKey = "RemovedComponentUUIDList";

        nlohmann::json DiffJson(const nlohmann::json& a_baseJson, const nlohmann::json& a_currentJson) const;

        nlohmann::json SerializeRemovedUUIDList(const GameObjectComponentContainer& a_gameObjectComponentContainer) const;

        const nlohmann::json* FindPTRPrefabComponentJson(const nlohmann::json& a_prefabComponentListJson, const boost::uuids::uuid& a_componentUUID) const;
    };
}