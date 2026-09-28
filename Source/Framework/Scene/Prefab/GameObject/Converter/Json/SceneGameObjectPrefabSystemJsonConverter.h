#pragma once

namespace FWK
{
    class SceneGameObjectPrefabSystem;
    class AssetFilePathRegistry;
}

namespace FWK::Converter
{
    class ScenePrefabSystemJsonConverter final
    {
    public:

         ScenePrefabSystemJsonConverter() = default;
        ~ScenePrefabSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, SceneGameObjectPrefabSystem& a_sceneGameObjectPrefabSystem) const;

        nlohmann::json Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry, SceneGameObjectPrefabSystem& a_sceneGameObjectPrefabSystem) const;

    private:

        static constexpr std::string_view k_prefabMapJsonKey = "PrefabMap";
        static constexpr std::string_view k_uuidJsonKey      = "UUID";
    };
}