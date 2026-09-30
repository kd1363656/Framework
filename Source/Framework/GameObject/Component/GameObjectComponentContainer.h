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

        void DeserializeScene (const nlohmann::json& a_rootJson, const nlohmann::json& a_prefabJson = nlohmann::json{});
        void DeserializePrefab(const nlohmann::json& a_rootJson);

        void PostDeserialize();

        void EarlyUpdate   () const;
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;
        
        void EditInspector() const;

        nlohmann::json Serialize    ()                                   const;
        nlohmann::json SerializeDiff(const nlohmann::json& a_prefabJson) const;

        void ConvertToPrefab() const;

        void DetachFromPrefab();

        bool AddComponent                 (const std::shared_ptr<GameObjectComponentBase>& a_component);
        void AddPrefabRemovedComponentUUID(const boost::uuids::uuid&                       a_uuid);

        void SweepExpiredComponents();

        void RemoveComponent(const std::weak_ptr<GameObjectComponentBase>& a_component);
        
        void ClearPrefabRemovedComponentUUIDSet();

        bool IsPrefabRemovedComponentUUID(const boost::uuids::uuid& a_uuid) const;

        void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

        template <Concept::IsDerivedGameObjectComponentBaseConcept ComponentType>
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

        template <Concept::IsDerivedGameObjectComponentBaseConcept ComponentType>
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

        std::weak_ptr<GameObjectComponentBase> FindVALComponentByUUID(const boost::uuids::uuid& a_uuid) const;

        const auto& GetREFComponentSmartPointerVectorList() const { return m_componentSmartPointerVectorList; }

    private:
   
        boost::uuids::uuid GenerateVALComponentUUID() const;

        std::unordered_map<std::uint32_t, std::weak_ptr<GameObjectComponentBase>>              m_uniqueComponentMap = {};
        std::unordered_map<std::uint32_t, std::vector<std::weak_ptr<GameObjectComponentBase>>> m_multiComponentMap  = {};

        std::unordered_set<boost::uuids::uuid> m_prefabRemovedComponentUUIDSet = {};

        std::weak_ptr<GameObject> m_owner = {};

        Utility::SmartPointerVectorList<std::shared_ptr<GameObjectComponentBase>> m_componentSmartPointerVectorList = {};

        UUIDRegistry<std::weak_ptr<GameObjectComponentBase>> m_componentUUIDRegistry = {};

        Converter::GameObjectComponentContainerJsonConverter m_jsonConverter = {};
    };
}