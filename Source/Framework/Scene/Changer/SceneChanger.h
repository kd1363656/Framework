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

        bool AddNextScene   (const boost::uuids::uuid& a_sceneUUID, const NextScene& a_nextScene);        
        bool RemoveNextScene(const boost::uuids::uuid& a_sceneUUID);
        
        const NextScene* FetchPTRNexScene(const boost::uuids::uuid& a_sceneUUID) const;

        const auto& GetREFNextSceneMap() const { return m_nextSceneMap; }

        const auto& GetREFSceneChangeEventObserver() const { return m_sceneChangeEventObserver; }

        auto& GetMutableREFNextSceneMap() { return m_nextSceneMap; }

        auto& GetMutableREFSceneChangeEventObserver() { return m_sceneChangeEventObserver; }

    private:
    
        std::unordered_map<boost::uuids::uuid, NextScene> m_nextSceneMap = {};

        Observer<Enum::SceneChangeEvent> m_sceneChangeEventObserver = {};

        Converter::SceneChangerJsonConverter m_jsonConverter = {};
    };
}