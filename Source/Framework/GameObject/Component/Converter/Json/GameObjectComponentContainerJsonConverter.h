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
    
        void DeserializeScene (const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const;
        void DeserializePrefab(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const;

        nlohmann::json Serialize(const GameObjectComponentContainer& a_gameObjectComponentContainer) const;
    };
}