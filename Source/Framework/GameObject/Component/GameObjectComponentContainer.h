#pragma once

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

        void CloneTo(GameObjectComponentContainer& a_cloneTarget, const std::weak_ptr<GameObject> a_gloneOwner) const;

    private:
   
        std::unordered_map<std::uint32_t, std::weak_ptr<ComponentBase>>              m_uniqueComponentMap = {};
        std::unordered_map<std::uint32_t, std::vector<std::weak_ptr<ComponentBase>>> m_multiComponentMap  = {};

        std::unordered_set<boost::uuids::uuid> m_removedComponentUUIDSet = {};

        Utility::SmartPointerVectorList<std::shared_ptr<ComponentBase>> m_componentSmartPointerVectorList = {};

        UUIDRegistry<std::weak_ptr<ComponentBase>> m_componentUUIDRegistry = {};

        Converter::GameObjectComponentContainerJsonConverter m_jsonConverter = {};
    };
}