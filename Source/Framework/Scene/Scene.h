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

        void SetName(const std::string& a_set) { m_name = a_set; }

        std::filesystem::path FetchVALNextLoadSceneFilePath() const;

        std::string FetchVALNextSceneName() const;

        const auto& GetREFGameObjectList() const { return m_gameObjectList; }

        const auto& GetREFSceneChanger          () const { return m_sceneChanger; }
        const auto& GetREFGameObjectPrefabSystem() const { return m_gameObjectPrefabSystem; }

        const auto& GetREFLightSystem() const { return m_lightSystem; }

        const auto& GetREFName() const { return m_name; }

        auto& GetMutableREFSceneChanger          () { return m_sceneChanger; }
        auto& GetMutableREFGameObjectPrefabSystem() { return m_gameObjectPrefabSystem; }

        auto& GetMutableREFLightSystem() { return m_lightSystem; }

    private:

        void AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel);

        void RemoveDestroyedGameObjects();

        void RebuildGameObjectExecutionLevelList();

        std::size_t CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject) const;

        static constexpr std::size_t k_initialExecutionLevel = 0ULL;

        std::vector<std::shared_ptr<GameObject>>            m_gameObjectList = {};
        std::vector<std::vector<std::weak_ptr<GameObject>>> m_gameObjectExecutionLevelList = {};

        UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

        SceneChanger                m_sceneChanger           = {};
        SceneGameObjectPrefabSystem m_gameObjectPrefabSystem = {};

        Converter::SceneJsonConverter m_jsonConverter = {};

        Graphics::LightSystem m_lightSystem = {};

        std::string m_name = {};

        boost::uuids::uuid m_nextSceneUUID = {};

        bool m_hadGameObjectListChangeDirty = false;
    };
}