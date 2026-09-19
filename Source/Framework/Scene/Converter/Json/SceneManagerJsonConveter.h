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

        void Load(SceneManager& a_sceneManager) const;

        void Save(const SceneManager& a_sceneManager) const;
    };
}