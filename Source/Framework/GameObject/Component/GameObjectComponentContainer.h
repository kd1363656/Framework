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

        nlohmann::json Serialize    ()                                   const;
        nlohmann::json SerializeDiff(const nlohmann::json& a_prefabJson) const;

        void Clone(GameObjectComponentContainer& a_cloneComponentContainer) const;

        void ConvertToPrefab() const;

        void DetachFromPrefab();

        bool AddComponent                 (const std::shared_ptr<GameObjectComponentBase>& a_component);
        void AddPrefabRemovedComponentUUID(const boost::uuids::uuid&                       a_uuid);

        void SweepExpiredComponents();

        void RemoveComponent(const std::weak_ptr<GameObjectComponentBase>& a_component);

        void ClearPrefabRemovedComponentUUIDSet();
        void RemovePrefabRemovedComponentUUID  (const boost::uuids::uuid& a_uuid);

        bool IsPrefabRemovedComponentUUID(const boost::uuids::uuid& a_uuid) const;

        void ApplyComponentOrder(const std::vector<boost::uuids::uuid>& a_componentUUIDList);

        void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

        std::vector<boost::uuids::uuid> FetchVALComponentUUIDList() const;

        template <Concept::IsDerivedGameObjectComponentBaseConcept ComponentType>
        std::weak_ptr<ComponentType> FindVALUniqueComponent() const
        {
            const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

            if (auto l_uniqueComponentITR = m_uniqueComponentMap.find(l_staticTypeID);
                l_uniqueComponentITR != m_uniqueComponentMap.end())
            {
                if (auto l_component = l_uniqueComponentITR->second.lock())
                {
                    return std::static_pointer_cast<ComponentType>(l_component);
                }
            }

            return {};
        }

        template <Concept::IsDerivedGameObjectComponentBaseConcept ComponentType>
        std::vector<std::weak_ptr<ComponentType>> FindVALMultiComponent() const
        {
            const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

            std::vector<std::weak_ptr<ComponentType>> l_componentList = {};

            if (auto l_multiComponentITR = m_multiComponentMap.find(l_staticTypeID);
                l_multiComponentITR != m_multiComponentMap.end())
            {
                l_componentList.reserve(l_multiComponentITR->second.size());

                for (const auto& l_component : l_multiComponentITR->second)
                {
                    l_componentList.emplace_back(l_component);
                }
            }

            return l_componentList;
        }

        std::weak_ptr<GameObjectComponentBase> FindVALComponentByUUID(const boost::uuids::uuid& a_uuid) const;

        const auto& GetREFComponentSmartPointerVectorList() const { return m_componentSmartPointerVectorList; }

    private:

        boost::uuids::uuid GenerateVALComponentUUID() const;

        std::size_t FetchVALComponentRank(const std::unordered_map<boost::uuids::uuid, std::size_t>& a_rankMap, const std::shared_ptr<GameObjectComponentBase>& a_component) const;

        static constexpr std::size_t k_lastComponentRank = std::numeric_limits<std::size_t>::max();

        std::unordered_map<std::uint32_t, std::weak_ptr<GameObjectComponentBase>>              m_uniqueComponentMap = {};
        std::unordered_map<std::uint32_t, std::vector<std::weak_ptr<GameObjectComponentBase>>> m_multiComponentMap  = {};

        std::unordered_set<boost::uuids::uuid> m_prefabRemovedComponentUUIDSet = {};

        Utility::SmartPointerVectorList<std::shared_ptr<GameObjectComponentBase>> m_componentSmartPointerVectorList = {};

        UUIDRegistry<std::weak_ptr<GameObjectComponentBase>> m_componentUUIDRegistry = {};

        std::weak_ptr<GameObject> m_owner = {};

        Converter::GameObjectComponentContainerJsonConverter m_jsonConverter = {};
    };
}