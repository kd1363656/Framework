#pragma once

namespace FWK
{
    class GameObject;
    class AssetFilePathRegistry;
}

namespace FWK
{
    class GameObjectPrefabSystem final
    {
    public:

         GameObjectPrefabSystem() = default;
        ~GameObjectPrefabSystem() = default;

        void INIT       ();
        void Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry);

        void AddPrefab   (const boost::uuids::uuid& a_prefabUUID, const GameObjectPrefab& a_prefab);
        void RemovePrefab(const boost::uuids::uuid& a_prefabUUID);

        nlohmann::json Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry);

        const GameObjectPrefab* FindPTRPrefab(const boost::uuids::uuid& a_prefabUUID) const;

        GameObjectPrefab* FindMutablePTRPrefab(const boost::uuids::uuid& a_prefabUUID);

        const auto& GetREFPrefabMap() const { return m_prefabMap; }

        auto& GetMutableREFPrefabMap() { return m_prefabMap; }

    private:

        std::unordered_map<boost::uuids::uuid, GameObjectPrefab> m_prefabMap = {};

        Converter::PrefabSystemJsonConverter m_jsonConverter = {};
    };
}