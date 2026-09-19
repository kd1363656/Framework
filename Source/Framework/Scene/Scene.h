#pragma once

namespace FWK
{
    class Scene final
    {
    private:

        using NextSceneLoadFilePathMap = std::unordered_map<boost::uuids::uuid, std::filesystem::path>;

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

        void SetIsGameObjectExecutionLevelListDirty(const bool a_set) { m_isGameObjectExecutionLevelListDirty = a_set; }

        std::filesystem::path FetchVALNextLoadSceneFilePath() const;

        std::weak_ptr<GameObject> FindVALGameObject(const boost::uuids::uuid& a_uuid) const;

        const auto& GetREFNextSceneLoadFilePathMap() const { return m_nextSceneLoadFilePathMap; }

        const auto& GetREFGameObjectList() const { return m_gameObjectList; }

        const auto& GetREFSceneShiftEventObserver() const { return m_sceneShiftEventObserver; }

        const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

        const auto& GetREFPrefabSystem() const { return m_prefabSystem; }

        const auto& GetREFSceneName() const { return m_sceneName; }

        auto& GetMutableREFSceneShiftEventObserver() { return m_sceneShiftEventObserver; }

        auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }

        auto& GetMutableREFPrefabSystem() { return m_prefabSystem; }

    private:

        void RemoveDestroyedGameObjects();

        void RefreshGameObjectExecutionLevelListIfNeeded();

        void RebuildGameObjectExecutionLevelList();

        void CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject, std::size_t& a_executionLevel) const;

        void AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel);

        static constexpr std::size_t k_initialExecutionLevel = 0ULL;

        NextSceneLoadFilePathMap m_nextSceneLoadFilePathMap = {};

        std::vector<std::shared_ptr<GameObject>>            m_gameObjectList               = {};
        std::vector<std::vector<std::weak_ptr<GameObject>>> m_gameObjectExecutionLevelList = {};

        UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

        Observer<Enum::SceneShiftEvent> m_sceneShiftEventObserver = {};

        AssetFilePathRegistry m_assetFilePathRegistry = {};

        PrefabSystem m_prefabSystem = {};

        Converter::SceneJsonConverter m_jsonConverter = {};

        Graphics::LightSystem m_lightSystem = {};

        std::string m_sceneName = {};

        boost::uuids::uuid m_nextSceneUUID = {};

        bool m_isGameObjectExecutionLevelListDirty = false;
    };
}