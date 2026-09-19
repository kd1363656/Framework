#pragma once

namespace FWK
{
    class SceneManager;
}

namespace FWK::Converter
{
    class SceneManagerJsonConverter final
    {
    public:

         SceneManagerJsonConverter() = default;
        ~SceneManagerJsonConverter() = default;

        static void DeserializeScene(const nlohmann::json& a_rootJson, Scene& a_scene);

        static nlohmann::json SerializeScene(Scene& a_scene);

        void Load(SceneManager& a_sceneManager) const;

        void Save(const SceneManager& a_sceneManager) const;
    };
}