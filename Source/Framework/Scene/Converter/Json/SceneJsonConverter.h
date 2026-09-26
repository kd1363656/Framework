#pragma once

namespace FWK
{
    class Scene;
}

namespace FWK::Converter
{
    class SceneJsonConverter final
    {
    public:

         SceneJsonConverter() = default;
        ~SceneJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const;

        nlohmann::json Serialize(Scene& a_scene) const;

    private:

        static constexpr std::string_view k_gameObjectListJsonKey        = "GameObjectList";
        static constexpr std::string_view k_prefabSystemJsonKey          = "PrefabSystem";
        static constexpr std::string_view k_sceneChanger                 = "SceneChanger";
        static constexpr std::string_view k_assetFilePathRegistryJsonKey = "AssetFilePathRegistry";
        static constexpr std::string_view k_gameObjectJsonKey            = "GameObject";
    };
}