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

        nlohmann::json SerializeGameObjectList(Scene& a_scene) const;

        static constexpr std::string_view k_gameObjectListJsonKey         = "GameObjectList";
        static constexpr std::string_view k_gameObjectPrefabSystemJsonKey = "GameObjectPrefabSystem";
        static constexpr std::string_view k_sceneChanger                  = "SceneChanger";
        static constexpr std::string_view k_lightSystemJsonKey            = "LightSystem";
        
        static constexpr std::string_view k_diffJsonKey = "Diff";
    };
}