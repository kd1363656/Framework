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
   
        bool DeserializePrefab(const nlohmann::json&                            a_rootJson,
                                     std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet,
                                     std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
                                     Scene&                                     a_scene);

        bool DeserializeScene (const nlohmann::json&                            a_rootJson,
                                     std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet,
                                     std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
                                     Scene&                                     a_scene);

        bool ApplyParent(const std::weak_ptr<GameObject>& a_self, const std::weak_ptr<GameObject>& a_child);
        bool Parent     (const std::weak_ptr<GameObject>& a_self, const std::weak_ptr<GameObject>& a_child);
        void Unparent   (const std::weak_ptr<GameObject>& a_self, const std::weak_ptr<GameObject>& a_child);

        void MarkChildForRemoval(const std::weak_ptr<GameObject>& a_child);
        void SweepRemovedChildren();

        nlohmann::json SerializePrefab() const;
        nlohmann::json SerializeScene () const;

        bool RecursiveAddChild(const std::weak_ptr<GameObject>& a_self, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene);

        void ResetParent();

        void Clear();

        void SetParent(const std::weak_ptr<GameObject>& a_set) { m_parent = a_set; }

        void SetPrefabNodeUUID(const boost::uuids::uuid& a_set) { m_prefabNodeUUID = a_set; }

        void SetMarkedForUnparent(const bool a_set) { m_isMarkedForUnparent = a_set; }

        const auto& GetREFRemovedNodeUUIDSet() const { return m_removedChildNodeUUIDSet; }

        const auto& GetREFParent() const { return m_parent; }

        const auto& GetREFChildSmartPointerVectorList() const { return m_childSmartPointerVectorList; }

        const auto& GetREFPrefabNodeUUID() const { return m_prefabNodeUUID; }

        auto& GetMutableREFRemovedNodeUUIDSet() { return m_removedChildNodeUUIDSet; }

        auto& GetMutableREFChildSmartPointerVectorList() { return m_childSmartPointerVectorList; }

        auto& GetMutableREFPrefabNodeUUID() { return m_prefabNodeUUID; }

        bool GetVALIsMarkedForUnparent() const { return m_isMarkedForUnparent; }

    private:

        bool ContainsDuplicatePrefabUUIDRecursive(const std::weak_ptr<GameObject>& a_gameObject, std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet) const;

        bool IsDescendantOf(const std::shared_ptr<GameObject>& a_ancestor) const;

        std::unordered_set<boost::uuids::uuid> m_removedChildNodeUUIDSet = {};

        std::weak_ptr<GameObject> m_parent = {};

        UUIDRegistry<std::weak_ptr<GameObject>> m_childNodeUUIDRegistry = {};

        Utility::SmartPointerVectorList<std::weak_ptr<GameObject>> m_childSmartPointerVectorList = {};
        
        Converter::GameObjectHierarchyJsonConverter m_jsonConverter = {};

        boost::uuids::uuid m_prefabNodeUUID = {};

        bool m_isMarkedForUnparent = false;
    };
}