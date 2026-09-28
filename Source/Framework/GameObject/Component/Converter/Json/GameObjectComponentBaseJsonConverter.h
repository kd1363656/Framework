#pragma once

namespace FWK
{
    class GameObjectComponentBase;
}

namespace FWK::Converter
{
    class GameObjectComponentBaseJsonConverter final
    {
    public:

         GameObjectComponentBaseJsonConverter() = default;
        ~GameObjectComponentBaseJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, GameObjectComponentBase& a_gameObjectComponentBase) const;
        
        nlohmann::json Serialize(const GameObjectComponentBase& a_gameObjectComponentBase) const;
        
    private:

        static constexpr std::string_view k_isDisableJsonKey      = "IsDisable";
        static constexpr std::string_view k_isPrefabOriginJsonKey = "IsPrefabOrigin";
    };
}