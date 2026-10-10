#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK
{
    class GameObjectBenchmarkComponent final : public GameObjectComponentBase
    {
    public:

         GameObjectBenchmarkComponent();
        ~GameObjectBenchmarkComponent() override;

        void PostDeserialize() override;

        void Update        () override;
        void PostLateUpdate() override;

        void EditInspector() override;

        std::shared_ptr<GameObjectComponentBase> Clone() const override;

        void Attach() override;
        void Detach() override;

        void Spawn();
        void Clear();

        void SetSpacing    (const float a_set) { m_spacing     = a_set; }
        void SetMovingRatio(const float a_set) { m_movingRatio = a_set; }

        void SetSpawnCount(const int a_set) { m_spawnCount = a_set; }

        void SetIsSkeletal(const bool a_set) { m_isSkeletal = a_set; }

        const auto& GetREFSpawnedGameObjectList() const { return m_spawnedGameObjectList; }

        auto& GetMutableREFModelFilePath() { return m_modelFilePath; }

        float GetVALSpacing    () const { return m_spacing; }
        float GetVALMovingRatio() const { return m_movingRatio; }

        int GetVALSpawnCount() const { return m_spawnCount; }

        bool GetVALIsSkeletal() const { return m_isSkeletal; }

    private:

        std::shared_ptr<GameObject> CreateGameObject(const TypeAlias::Math::Vector3& a_position) const;

        void MoveSpawnedGameObjectList() const;

        TypeAlias::Math::Vector3 FetchVALSpawnPosition(const std::size_t& a_spawnIndex, const std::size_t& a_gridSideCount) const;

        static constexpr float k_defaultSpacing       = 2.0F;
        static constexpr float k_defaultMovingRatio   = 0.1F;
        static constexpr float k_movingAmplitude      = 0.5F;
        static constexpr float k_movingSpeed          = 2.0F;
        static constexpr float k_movingPhaseStep      = 0.37F;
        static constexpr float k_initialElapsedSecond = 0.0F;
        static constexpr float k_spawnPositionY       = 0.0F;

        static constexpr int k_defaultSpawnCount = 100;
        static constexpr int k_minSpawnCount     = 1;

        std::vector<std::shared_ptr<GameObject>> m_spawnedGameObjectList;

        AssetFilePath                                        m_modelFilePath;
        Utility::FetchSelfGameObjectTransformComponentHelper m_fetchSelfGameObjectTransformComponentHelper;

        GameObjectBenchmarkComponentInspector m_inspector;

        float m_spacing;
        float m_movingRatio;
        float m_elapsedSecond;

        int m_spawnCount;

        bool m_isSkeletal;
        bool m_isAttached;

        FWK_DEFINE_TYPE_INFO(GameObjectBenchmarkComponent, GameObjectComponentBase)
    };
}

FWK_REGISTER_FACTORY_METHOD                      (FWK::TypeAlias::GameObjectComponentSharedFactory, FWK::GameObjectBenchmarkComponent)
FWK_REGISTER_TAGGED_GAME_OBJECT_COMPONENT_FACTORY(FWK::Constant::k_gameObjectComponentTagBenchmark, FWK::GameObjectBenchmarkComponent)