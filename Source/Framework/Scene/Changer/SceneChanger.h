#pragma once

namespace FWK
{
    class SceneChanger final
    {
    public:
    
         SceneChanger() = default;
        ~SceneChanger() = default;

        void INIT();

        void Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry);

        nlohmann::json Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry) const;

        bool AddNextSceneData(const boost::uuids::uuid& a_sceneUUID, const Struct::NextSceneData& a_nextSceneData);
        
        bool RemoveNextSceneData(const boost::uuids::uuid& a_sceneUUID);
        
        const Struct::NextSceneData* FetchPTRNexSceneData(const boost::uuids::uuid& a_sceneUUID) const;

        const auto& GetREFNextSceneDataMap() const { return m_nextSceneDataMap; }

        const auto& GetREFSceneChangeEventObserver() const { return m_sceneChangeEventObserver; }

        auto& GetMutableREFNextSceneDataMap() { return m_nextSceneDataMap; }

        auto& GetMutableREFSceneChangeEventObserver() { return m_sceneChangeEventObserver; }

    private:
    
        std::unordered_map<boost::uuids::uuid, Struct::NextSceneData> m_nextSceneDataMap = {};

        Observer<Enum::SceneChangeEvent> m_sceneChangeEventObserver = {};

        Converter::SceneChangerJsonConverter m_jsonConverter = {};
    };
}