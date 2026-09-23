#pragma once

namespace FWK
{
    class GameObject;
    class AssetFilePathRegistry;
}

namespace FWK
{
    class PrefabSystem final
    {
    private:

        using PrefabMap = std::unordered_map<boost::uuids::uuid, Prefab>;

    public:

         PrefabSystem() = default;
        ~PrefabSystem() = default;

        void INIT                         ();
        void Deserialize                  (const nlohmann::json&            a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry);
        void CachePrefabGameObjectIfNeeded(const std::weak_ptr<GameObject>& a_gameObject);

        void AddPrefab   (const boost::uuids::uuid& a_prefabUUID, const Prefab& a_prefab);
        void RemovePrefab(const boost::uuids::uuid& a_prefabUUID);

        nlohmann::json Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry);

        const Prefab* FindPTRPrefab(const boost::uuids::uuid& a_prefabUUID) const;

        Prefab* FindMutablePTRPrefab(const boost::uuids::uuid& a_prefabUUID);

        const auto& GetREFPrefabMap() const { return m_prefabMap; }

        auto& GetMutableREFPrefabMap() { return m_prefabMap; }

    private:

        PrefabMap m_prefabMap = {};

        Converter::PrefabSystemJsonConverter m_jsonConverter = {};
    };
}