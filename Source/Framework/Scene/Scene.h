#pragma once

namespace FWK
{
    class Scene final
    {
    public:

         Scene() = default;
        ~Scene() = default;

        void INIT           ();
        void Deserialize    (const nlohmann::json& a_rootJson);
        void PostDeserialize() const;

        void EarlyUpdate   ();
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;

        nlohmann::json Serialize();

        void AddGameObject(const std::shared_ptr<GameObject>& a_gameObject);

        bool AddNextSceneLoadFilePath(const boost::uuids::uuid&    a_sceneUUID);
        bool AddNextSceneLoadFilePath(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID);

        bool RemoveNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID);

        bool ReplaceSceneFilePath(const std::filesystem::path& a_oldSceneFilePath, const std::filesystem::path& a_newSceneFilePath, const boost::uuids::uuid& a_sceneUUID);

        void SetSceneName(const std::string& a_set) { m_sceneName = a_set; }

        std::filesystem::path FetchVALNextLoadSceneFilePath() const;

        const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

        const auto& GetREFPrefabSystem() const { return m_prefabSystem; }

        const auto& GetREFSceneName() const { return m_sceneName; }

        auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }

        auto& GetMutableREFPrefabSystem() { return m_prefabSystem; }

    private:

        void AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel);

        void RemoveDestroyedGameObjects();

        void RebuildGameObjectExecutionLevelList();

        std::size_t CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject) const;

        static constexpr std::size_t k_initialExecutionLevel = 0ULL;

        std::vector<std::shared_ptr<GameObject>>            m_gameObjectList = {};
        std::vector<std::vector<std::weak_ptr<GameObject>>> m_gameObjectExecutionLevelList = {};

        UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

        AssetFilePathRegistry m_assetFilePathRegistry = {};

        PrefabSystem m_prefabSystem = {};

        Converter::SceneJsonConverter m_jsonConverter = {};

        Graphics::LightSystem m_lightSystem = {};

        std::string m_sceneName = {};

        boost::uuids::uuid m_nextSceneUUID = {};

        bool m_hadGameObjectListChangeDirty = false;
    };
}