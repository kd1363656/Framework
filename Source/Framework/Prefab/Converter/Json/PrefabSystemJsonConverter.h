#pragma once

namespace FWK
{
    class PrefabSystem;
    class AssetFilePathRegistry;
}

namespace FWK::Converter
{
    class PrefabSystemJsonConverter final
    {
    public:

         PrefabSystemJsonConverter() = default;
        ~PrefabSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, PrefabSystem& a_prefabSystem) const;

        nlohmann::json Serialize  (const AssetFilePathRegistry& a_assetFilePathRegistry, PrefabSystem& a_prefabSystem) const;

    private:

        static constexpr std::string_view k_prefabMapJsonKey = "PrefabMap";
        static constexpr std::string_view k_uuidJsonKey      = "UUID";
    };
}