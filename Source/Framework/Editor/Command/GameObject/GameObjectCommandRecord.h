#pragma once

namespace FWK
{
    class GameObject;
    class Scene;
    class SceneGameObjectPrefabSystem;
}

namespace FWK::Editor
{
    class GameObjectCommandRecord final
    {
    public:

         GameObjectCommandRecord() = default;
        ~GameObjectCommandRecord() = default;

        void Capture(const std::weak_ptr<GameObject>& a_gameObject, Scene& a_scene);

        void Remove (Scene& a_scene) const;
        void Restore(Scene& a_scene) const;

        bool IsInScene(const Scene& a_scene) const;

    private:

        nlohmann::json FetchVALPrefabNodeJson(const GameObject& a_gameObject, const SceneGameObjectPrefabSystem& a_prefabSystem) const;

        nlohmann::json m_json           = {};
        nlohmann::json m_prefabNodeJson = {};

        boost::uuids::uuid m_gameObjectUUID = {};

        Struct::GameObjectPlacement m_placement = {};
    };
}