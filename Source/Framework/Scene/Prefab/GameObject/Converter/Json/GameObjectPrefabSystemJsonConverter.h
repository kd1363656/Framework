#pragma once

namespace FWK
{
    class GameObjectPrefabSystem;
    class AssetFilePathRegistry;
}

namespace FWK::Converter
{
    class PrefabSystemJsonConverter final
    {
    public:

         PrefabSystemJsonConverter() = default;
        ~PrefabSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, GameObjectPrefabSystem& a_gameObjectPrefabSystem) const;

        nlohmann::json Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry, GameObjectPrefabSystem& a_gameObjectPrefabSystem) const;

    private:

        static constexpr std::string_view k_prefabMapJsonKey = "PrefabMap";
        static constexpr std::string_view k_uuidJsonKey      = "UUID";
    };
}