#pragma once

namespace FWK
{
    class GameObjectComponentContainer final
    {
    public:

         GameObjectComponentContainer() = default;
        ~GameObjectComponentContainer() = default;
    
        void Add           (const std::shared_ptr<ComponentBase>& a_component);
        void Remove        (const std::weak_ptr<ComponentBase>&   a_component);
        void MarkForRemoval(const std::weak_ptr<ComponentBase>&   a_component);

        void SweepRemoved();
        void Clear       ();

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
        std::weak_ptr<ComponentType> FindComponentFromUUID(const boost::uuids::uuid& a_uuid) const
        {
            const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

            if (l_componentDataList.empty()) { return {}; }

            const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

            for (const auto& l_componentData : l_componentDataList)
            {
                const auto& l_component = l_componentData.m_type;

                if (!l_component                                                  ||
                    l_component->GetREFUUID().is_nil()                            ||
                    l_component->GetREFUUID()                           != a_uuid ||
                    l_component->GetREFRuntimeTypeINFO().k_staticTypeID != l_staticTypeID) 
                {
                    continue; 
                }

                return std::static_pointer_cast<ComponentType>(l_component);
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
                    l_list.emplace_back(std::static_pointer_cast<ComponentType>(l_component));
                }
            }

            return l_list;
        }
       
    private:
    
        std::unordered_map<std::uint32_t, std::weak_ptr<ComponentBase>>              m_uniqueComponentMap = {};
        std::unordered_map<std::uint32_t, std::vector<std::weak_ptr<ComponentBase>>> m_multiComponentMap  = {};

        std::unordered_set<boost::uuids::uuid> m_removedComponentUUIDSet = {};

        Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>> m_componentSmartPointerVectorList = {};

        UUIDRegistry<std::weak_ptr<ComponentBase>> m_componentUUIDRegistry = {};
    };
}