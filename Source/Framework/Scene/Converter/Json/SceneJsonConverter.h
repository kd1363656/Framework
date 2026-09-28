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

        void DeserializeGameObjectList(const nlohmann::json& a_rootJson, Scene& a_scene) const;
 
        nlohmann::json SerializeGameObjectList(const Scene& a_scene) const;
 
        void ApplyJsonDiff(const nlohmann::json& a_diffJson, nlohmann::json& a_baseJson) const;

        nlohmann::json DetectPrefabDiff(const GameObject& a_gameObject, const nlohmann::json& a_prefabJson) const;
 
        nlohmann::json DetectJsonDiff(const nlohmann::json& a_baseJson, const nlohmann::json& a_currentJson) const;
 
        static constexpr std::string_view k_gameObjectListJsonKey         = "GameObjectList";
        static constexpr std::string_view k_gameObjectPrefabSystemJsonKey = "GameObjectPrefabSystem";
        static constexpr std::string_view k_sceneChanger                  = "SceneChanger";
        static constexpr std::string_view k_assetFilePathRegistryJsonKey  = "AssetFilePathRegistry";
        static constexpr std::string_view k_sceneNameJsonKey              = "SceneName";

        static constexpr std::string_view k_diffJsonKey = "Diff";
    };
}