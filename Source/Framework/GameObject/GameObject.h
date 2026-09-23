#pragma once

namespace FWK
{
    class Scene;
    class TransformComponent;
}

namespace FWK
{
    class GameObject final : public std::enable_shared_from_this<GameObject>
    {
    public:

         GameObject() = default;
        ~GameObject() = default;

        void INIT();

        void Deserialize(const nlohmann::json& a_rootJson, std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet, Scene& a_scene);

        bool DeserializePrefab(const nlohmann::json&                                                  a_rootJson,
                                     std::vector<Struct::ChildDeserializeData>&                       a_childDeserializeDataList,
                                     Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorList,
                                     std::unordered_set<boost::uuids::uuid>&                          a_parentPrefabUUIDSet,
                                     Scene&                                                           a_scene);

        bool DeserializePrefabInstance(const nlohmann::json& a_prefabJson, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene);

        bool DeserializeScene(const nlohmann::json&                                                  a_rootJson,
                                    std::vector<Struct::ChildDeserializeData>&                       a_childDeserializeDataList,
                                    Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorList,
                                    Scene&                                                           a_scene);

        void RecursiveAddComponent(const Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorList, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList);
        bool RecursiveAddChild    (      std::vector<Struct::ChildDeserializeData>&                       a_childDeserializeDataList,        Scene&                                     a_scene);

        void PostDeserialize();

        void EarlyUpdate   () const;
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;

        void Destroy();

        void EditInspector();

        nlohmann::json SerializeScene () const;
        nlohmann::json SerializePrefab() const;

        void SetComponentEventObserver(const std::shared_ptr<Observer<Enum::ComponentEvent>>& a_set) { m_componentEventObserver = a_set; }

        void SetPrefabUUID       (const boost::uuids::uuid& a_set) { m_prefabUUID        = a_set; }
        void SetSceneInstanceUUID(const boost::uuids::uuid& a_set) { m_sceneInstanceUUID = a_set; }

        void SetName(const std::string& a_set) { m_name = a_set; }
        
        const std::weak_ptr<Observer<Enum::ComponentEvent>> GetVALComponentEventObserver() const { return m_componentEventObserver; }

        const auto& GetREFHierarchy() const { return m_hierarchy; }

        const auto& GetREFPrefabUUID       () const { return m_prefabUUID; }
        const auto& GetREFSceneInstanceUUID() const { return m_sceneInstanceUUID; }

        const auto& GetREFName() const { return m_name; }

        auto& GetMutableREFHierarchy() { return m_hierarchy; }

        auto& GetMutableREFSceneInstanceUUID() { return m_sceneInstanceUUID; }

        auto& GetMutableREFPrefabUUID            () { return m_prefabUUID; }
        auto& GetMutableREFComponentEventObserver() { return m_componentEventObserver; }

        std::weak_ptr<TransformComponent> GetVALTransformComponent() const { return m_transformComponent; }

        bool GetVALIsDestroyed() const { return m_isDestroyed; }

    private:

        std::shared_ptr<TransformComponent> m_transformComponent = std::make_shared<TransformComponent>();

        std::shared_ptr<Observer<Enum::ComponentEvent>> m_componentEventObserver = nullptr;

        GameObjectComponentContainer m_componentContainer  = {};
        GameObjectHierarchy          m_hierarchy           = {};

        Converter::GameObjectJsonConverter m_jsonConverter = {};

        boost::uuids::uuid m_prefabUUID        = {};
        boost::uuids::uuid m_sceneInstanceUUID = {};

        std::string m_name = {};

        bool m_isDestroyed = false;
    };
}