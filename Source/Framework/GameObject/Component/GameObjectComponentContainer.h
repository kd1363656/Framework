#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK
{
    class GameObjectComponentContainer final
    {
    public:
    
         GameObjectComponentContainer() = default;
        ~GameObjectComponentContainer() = default;
    
        void INIT();

        void Deserialize(const nlohmann::json& a_rootJson);

        void PostDeserialize();

        void EarlyUpdate   () const;
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;

        void EditInspector() const;

        nlohmann::json Serialize() const;

        void CloneTo(GameObjectComponentContainer& a_cloneTarget, const std::weak_ptr<GameObject>& a_cloneOwner) const;

        bool AddComponent                 (const std::shared_ptr<ComponentBase>& a_component);
        void AddPrefabRemovedComponentUUID(const boost::uuids::uuid&             a_uuid);

        void SweepExpiredComponents();

        void RemoveComponent(const std::weak_ptr<ComponentBase>& a_component);
        
        void ClearPrefabRemovedComponentUUIDSet();

        bool IsPrefabRemovedComponentUUID(const boost::uuids::uuid& a_uuid) const;

        void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

        template <Concept::IsDerivedComponentBaseConcept ComponentType>
        std::weak_ptr<ComponentType> FindUniqueComponent() const
        {
            const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

            if (auto l_itr = m_uniqueComponentMap.find(l_staticTypeID);
                l_itr != m_uniqueComponentMap.end())
            {
                if (auto l_component = l_itr->second.lock())
                {
                    return std::static_pointer_cast<ComponentType>(l_component);
                }
            }

            return {};
        }

        template <Concept::IsDerivedComponentBaseConcept ComponentType>
        std::vector<std::weak_ptr<ComponentType>> FindMultiComponent() const
        {
            const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

            std::vector<std::weak_ptr<ComponentType>> l_list = {};

            if (auto l_itr = m_multiComponentMap.find(l_staticTypeID);
                l_itr != m_multiComponentMap.end())
            {
                l_list.reserve(l_itr->second.size());

                for (const auto& l_component : l_itr->second)
                {
                    l_list.emplace_back(l_component);
                }
            }

            return l_list;
        }

        std::weak_ptr<ComponentBase> FindVALComponentByUUID(const boost::uuids::uuid& a_uuid) const;

        const auto& GetREFComponentList() const { return m_componentSmartPointerVectorList; }

    private:
   
        boost::uuids::uuid GenerateVALComponentUUID() const;

        std::unordered_map<std::uint32_t, std::weak_ptr<ComponentBase>>              m_uniqueComponentMap = {};
        std::unordered_map<std::uint32_t, std::vector<std::weak_ptr<ComponentBase>>> m_multiComponentMap  = {};

        std::unordered_set<boost::uuids::uuid> m_prefabRemovedComponentUUIDSet = {};

        std::weak_ptr<GameObject> m_owner = {};

        Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>> m_componentSmartPointerVectorList = {};

        UUIDRegistry<std::weak_ptr<ComponentBase>> m_componentUUIDRegistry = {};

        Converter::GameObjectComponentContainerJsonConverter m_jsonConverter = {};
    };
}