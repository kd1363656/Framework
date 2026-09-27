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
    
        void Deserialize(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer);

        nlohmann::json Serialize(const GameObjectComponentContainer& a_gameObjectComponentContainer) const;

    private:

        static constexpr std::string_view k_componentListJsonKey = "ComponentList";
        static constexpr std::string_view k_componentTypeJsonKey = "ComponentType";
        static constexpr std::string_view k_componentDataJsonKey = "ComponentData";
    };
}