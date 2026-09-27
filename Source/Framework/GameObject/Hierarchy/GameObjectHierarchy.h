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

        void ApplyRoot  (const std::weak_ptr<GameObject>& a_gameObject);
        bool ApplyParent(const std::weak_ptr<GameObject>& a_parent);

        void ClearParent();

        void SetOwner (const std::weak_ptr<GameObject>& a_set) { m_owner  = a_set; }
        void SetParent(const std::weak_ptr<GameObject>& a_set) { m_parent = a_set; }

        const auto& GetREFOwner () const { return m_owner; }
        const auto& GetREFParent() const { return m_parent; }

    private:

        void AddChild(const std::weak_ptr<GameObject>& a_child);

        void RemoveChild(const std::weak_ptr<GameObject>& a_child);

        bool IsAncestorChainContainsOwner(const std::weak_ptr<GameObject>& a_gameObject) const;

        std::weak_ptr<GameObject> m_owner  = {};
        std::weak_ptr<GameObject> m_root   = {};
        std::weak_ptr<GameObject> m_parent = {};

        Utility::SmartPointerVectorList<std::weak_ptr<GameObject>> m_childSmartPointerVectorList = {};

        std::shared_ptr<FWK::Struct::GameObjectPrefabChildHierarchy> m_prefabChildHierarchy = nullptr;

        Converter::GameObjectHierarchyJsonConverter m_jsonConverter = {};
    };
}