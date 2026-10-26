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

        void Load(const SceneManager& a_sceneManager) const;

        bool Save(const SceneManager& a_sceneManager) const;
    };
}