#pragma once

namespace FWK
{
    class SceneChanger
    {
    private:

        using NextSceneLoadFilePathMap = std::unordered_map<boost::uuids::uuid, std::filesystem::path>;

    public:
    
         SceneChanger() = default;
        ~SceneChanger() = default;

        void INIT();

        bool AddNextSceneLoadFilePath(const AssetFilePathRegistry& a_assetFilePathRegistry, const boost::uuids::uuid& a_sceneUUID);
        bool AddNextSceneLoadFilePath(const std::filesystem::path& a_filePath,              const boost::uuids::uuid& a_sceneUUID, AssetFilePathRegistry& a_assetFilePathRegistry);

        bool RemoveNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID, AssetFilePathRegistry& a_assetFilePathRegistry);

        bool ReplaceSceneFilePath(const std::filesystem::path& a_oldSceneFilePath, 
                                  const std::filesystem::path& a_newSceneFilePath, 
                                  const boost::uuids::uuid&    a_sceneUUID,
                                        AssetFilePathRegistry& a_assetFilePathRegistry);

        const auto& GetREFNextSceneLoadFilePathMap() const { return m_nextSceneLoadFilePathMap; }

        const auto& GetREFSceneChangeEventObserver() const { return m_sceneChangeEventObserver; }

        auto& GetMutableREFSceneChangeEventObserver() { return m_sceneChangeEventObserver; }

    private:
    
        NextSceneLoadFilePathMap m_nextSceneLoadFilePathMap = {};

        Observer<Enum::SceneChangeEvent> m_sceneChangeEventObserver = {};
    };
}