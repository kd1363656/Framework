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

        void DeserializeNextSceneLoadFilePathMap(const nlohmann::json& a_rootJson, Scene& a_scene) const;
        void DeserializeGameObjectList          (const nlohmann::json& a_rootJson, Scene& a_scene) const;

        nlohmann::json SerializeNextSceneLoadFilePathMap(const Scene& a_scene) const;
        nlohmann::json SerializeGameObjectList          (const Scene& a_scene) const;

        static constexpr std::string_view k_nextSceneLoadFilePathMapJsonKey = "NextSceneLoadFilePathMap";
        static constexpr std::string_view k_gameObjectListJsonKey           = "GameObjectList";
        static constexpr std::string_view k_prefabSystemJsonKey             = "PrefabSystem";
        static constexpr std::string_view k_assetFilePathRegistryJsonKey    = "AssetFilePathRegistry";
        static constexpr std::string_view k_sceneShiftEventObserverJsonKey  = "SceneShiftEventObserver";
        static constexpr std::string_view k_nextSceneUUIDJsonKey            = "NextSceneUUID";
        static constexpr std::string_view k_gameObjectJsonKey               = "GameObject";
        static constexpr std::string_view k_sceneNameJsonKey                = "SceneName";
    };
}