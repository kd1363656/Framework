#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Converter
{
    class GameObjectSceneJsonConverter final
    {
    public:

         GameObjectSceneJsonConverter() = default;
        ~GameObjectSceneJsonConverter() = default;

        bool Deserialize(const nlohmann::json&                      a_rootJson,
                         std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet,
                         std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
                         GameObject&                                a_gameObject,
                         Scene&                                     a_scene) const;

        nlohmann::json Serialize(const GameObject& a_gameObject, const Scene& a_scene) const;

    private:

        bool DeserializeSceneComponentEventObserver(const nlohmann::json& a_rootJson, GameObject& a_gameObject)                                                                                                   const;

        nlohmann::json SerializeSceneComponentObserver(const GameObject& a_gameObject) const;
        
        static constexpr std::string_view k_nameJsonKey              = "Name";
        static constexpr std::string_view k_sceneInstanceUUIDJsonKey = "SceneInstanceUUID";
    };
}