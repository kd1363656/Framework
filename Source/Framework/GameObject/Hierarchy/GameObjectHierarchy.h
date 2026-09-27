#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK
{
    class GameObjectHierarchy
    {
    public:

         GameObjectHierarchy() = default;
        ~GameObjectHierarchy() = default;
    
        void INIT();

        void Deserialize(const nlohmann::json& a_rootJson);

        nlohmann::json Serialize() const;

        void CloneTo(GameObjectHierarchy& a_cloneTarget, const std::weak_ptr<GameObject>& a_cloneOwner) const;

        bool ApplyParent(const std::weak_ptr<GameObject>& a_parent);

        void ClearParent                   ();
        void ClearPrefabRemovedChildUUIDSet();

        void ConnectParentForDeserialize(const std::weak_ptr<GameObject>& a_parent);
        
        void AddPrefabRemovedUUID(const boost::uuids::uuid& a_uuid);

        void SetOwner (const std::weak_ptr<GameObject>& a_set) { m_owner  = a_set; }
        void SetParent(const std::weak_ptr<GameObject>& a_set) { m_parent = a_set; }

        const auto& GetREFOwner () const { return m_owner; }
        const auto& GetREFParent() const { return m_parent; }

        const auto& GetREFChildSmartPointerVectorList() const { return m_childSmartPointerVectorList; }

        const auto& GetREFChildUUIDRegistry() const { return m_childUUIDRegistry; }

        const auto& GetREFPrefabRemovedChildUUIDSet() const { return m_prefabRemovedChildUUIDSet; }

        auto& GetMutableREFChildUUIDRegistry() { return m_childUUIDRegistry; }

        auto& GetMutableREFPrefabRemovedChildUUIDSet() { return m_prefabRemovedChildUUIDSet; }


    private:

        void AddChild(const std::weak_ptr<GameObject>& a_child);

        void RemoveChild(const std::weak_ptr<GameObject>& a_child);

        bool IsAncestorChainContainsOwner(const std::weak_ptr<GameObject>& a_gameObject) const;

        boost::uuids::uuid GenerateVALChildUUID() const;

        std::weak_ptr<GameObject> m_owner  = {};
        std::weak_ptr<GameObject> m_parent = {};

        Utility::SmartPointerVectorList<std::weak_ptr<GameObject>> m_childSmartPointerVectorList = {};

        UUIDRegistry<std::weak_ptr<GameObject>> m_childUUIDRegistry = {};

        std::unordered_set<boost::uuids::uuid> m_prefabRemovedChildUUIDSet = {};
        
        Converter::GameObjectHierarchyJsonConverter m_jsonConverter = {};
    };
}