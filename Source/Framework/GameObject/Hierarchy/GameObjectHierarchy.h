#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK
{
    class GameObjectHierarchy final
    {
    public:

         GameObjectHierarchy() = default;
        ~GameObjectHierarchy() = default;

        void INIT();

        void DeserializeScene (const nlohmann::json&              a_rootJson,
                               const nlohmann::json&              a_prefabJson,
                               const SceneGameObjectPrefabSystem& a_prefabSystem,
                                     Scene&                       a_scene);

        void DeserializePrefab(const nlohmann::json& a_rootJson, const SceneGameObjectPrefabSystem& a_prefabSystem, Scene& a_scene);

        void PostDeserialize();

        void Destroy();

        nlohmann::json Serialize    (      SceneGameObjectPrefabSystem& a_prefabSystem)                                            const;
        nlohmann::json SerializeDiff(const nlohmann::json&              a_prefabJson, SceneGameObjectPrefabSystem& a_prefabSystem) const;

        void Clone(GameObjectHierarchy& a_cloneHierarchy, Scene& a_scene) const;

        void ConvertToPrefab(const boost::uuids::uuid& a_prefabUUID) const;

        void DetachFromPrefab(const boost::uuids::uuid& a_oldPrefabUUID);

        void ClearParent                   ();
        void ClearPrefabRemovedChildUUIDSet();

        void ConnectParentForDeserialize(const std::weak_ptr<GameObject>& a_parent);

        void AddPrefabRemovedUUID(const boost::uuids::uuid& a_uuid);

        bool ApplyParent(const std::weak_ptr<GameObject>& a_parent);

        void SetOwner (const std::weak_ptr<GameObject>& a_set) { m_owner  = a_set; }
        void SetParent(const std::weak_ptr<GameObject>& a_set) { m_parent = a_set; }

        const auto& GetREFPrefabRemovedChildUUIDSet() const { return m_prefabRemovedChildUUIDSet; }

        const auto& GetREFOwner () const { return m_owner; }
        const auto& GetREFParent() const { return m_parent; }

        const auto& GetREFChildSmartPointerVectorList() const { return m_childSmartPointerVectorList; }

        const auto& GetREFChildUUIDRegistry() const { return m_childUUIDRegistry; }

        auto& GetMutableREFPrefabRemovedChildUUIDSet() { return m_prefabRemovedChildUUIDSet; }

        auto& GetMutableREFChildSmartPointerVectorList() { return m_childSmartPointerVectorList; }

        auto& GetMutableREFChildUUIDRegistry() { return m_childUUIDRegistry; }

    private:

        void AddChild(const std::weak_ptr<GameObject>& a_child);

        void RemoveChild(const std::weak_ptr<GameObject>& a_child);

        boost::uuids::uuid GenerateVALChildUUID() const;

        bool IsAncestorChainContainsOwner(const std::weak_ptr<GameObject>& a_gameObject) const;

        std::unordered_set<boost::uuids::uuid> m_prefabRemovedChildUUIDSet = {};

        Utility::SmartPointerVectorList<std::weak_ptr<GameObject>> m_childSmartPointerVectorList = {};

        UUIDRegistry<std::weak_ptr<GameObject>> m_childUUIDRegistry = {};

        std::weak_ptr<GameObject> m_owner  = {};
        std::weak_ptr<GameObject> m_parent = {};

        Converter::GameObjectHierarchyJsonConverter m_jsonConverter = {};
    };
}