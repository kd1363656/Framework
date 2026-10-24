#pragma once

namespace FWK
{
    class Scene final
    {
    private:

        // ※負荷テスト専用 : 並べるモデルの種類
        // Static              : StaticModel(アニメーションなし)を500体
        // SkeletalGPUSkinning : SkeletalAnimationModelを500体、GPUスキニングでアニメーションさせる
        enum class PerformanceTestModelKind
        {
            Static,
            SkeletalGPUSkinning,
        };

        // ※負荷テスト専用 : モデルの大きさから決めた、並べ方の情報
        struct PerformanceTestLayout final
        {
            TypeAlias::Math::Vector3 m_boundsMIN  = {};
            TypeAlias::Math::Vector3 m_boundsSize = {};

            float m_spacing   = 0.0F;
            float m_gridWidth = 0.0F;
            float m_gridDepth = 0.0F;

            std::size_t m_meshCount     = 0ULL;
            std::size_t m_triangleCount = 0ULL;
            std::size_t m_meshletCount  = 0ULL;
            std::size_t m_maxLODCount   = 0ULL;

            std::vector<std::size_t> m_lodTriangleCountList = {};
        };

    public:

         Scene() = default;
        ~Scene() = default;

        void INIT           ();
        void Deserialize    (const nlohmann::json& a_rootJson);
        void PostDeserialize();

        void EarlyUpdate   ();
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;

        nlohmann::json Serialize();

        void RebuildGameObjectExecutionLevelList();

        void AddGameObject(const std::shared_ptr<GameObject>& a_gameObject);

        void RemoveGameObject(const std::weak_ptr<GameObject>& a_gameObject);

        void SetName(const std::string& a_set) { m_name = a_set; }

        std::weak_ptr<GameObject> FindVALGameObject(const boost::uuids::uuid& a_sceneInstanceUUID) const;

        std::filesystem::path FetchVALNextLoadSceneFilePath() const;

        std::string FetchVALNextSceneName() const;

        const auto& GetREFGameObjectList() const { return m_gameObjectList; }

        const auto& GetREFSceneChanger          () const { return m_sceneChanger; }
        const auto& GetREFGameObjectPrefabSystem() const { return m_gameObjectPrefabSystem; }

        const auto& GetREFLightSystem() const { return m_lightSystem; }

        const auto& GetREFName() const { return m_name; }

        auto& GetMutableREFGameObjectList() { return m_gameObjectList; }

        auto& GetMutableREFSceneChanger          () { return m_sceneChanger; }
        auto& GetMutableREFGameObjectPrefabSystem() { return m_gameObjectPrefabSystem; }

        auto& GetMutableREFLightSystem() { return m_lightSystem; }

    private:

        void AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel);

        void RemoveDestroyedGameObjects();
        void AddPendingGameObjects     ();

        std::size_t CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject) const;

        // ※負荷テスト専用の処理
        // 10万ポリゴン以上のモデルを500体並べて描画し、CPU、GPUの処理負荷を計測するために
        // シーンへ直接書いている(コンポーネントは作らない)
        void SetupPerformanceTest                   ();
        void SetupPerformanceTestStaticModel        ();
        void SetupPerformanceTestSkeletalModel      ();
        void SetupPerformanceTestCamera             (const PerformanceTestLayout& a_layout);
        void AddPerformanceTestReportNote           (const PerformanceTestLayout& a_layout) const;
        void UpdatePerformanceTestCameraAspectRatio ();
        void UpdatePerformanceTestAnimationTime     ();

        // 並びの何番目かから、モデルを置く位置を求める
        TypeAlias::Math::Vector3 CalculatePerformanceTestPosition(const PerformanceTestLayout& a_layout, const std::size_t a_indexX, const std::size_t a_indexZ) const;

        // 確認用のモーションを持たないモデルのために、全ボーンをゆらゆら回転させるモーションを作って追加する
        void AddPerformanceTestProceduralMotion(Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData) const;

        // モデル全体を囲む箱(AABB : 軸に沿った直方体)と三角形の数を、CPU側に残っている頂点、インデックスから求める
        // StaticModelとSkeletalAnimationModelのModelMeshは別の型だが、使う変数名が同じなのでtemplateで共通化する
        template <typename ModelMeshListType>
        PerformanceTestLayout CalculatePerformanceTestLayout(const ModelMeshListType& a_modelMeshList) const
        {
            PerformanceTestLayout l_layout = {};

            TypeAlias::Math::Vector3 l_boundsMAX = TypeAlias::Math::Vector3(-std::numeric_limits<float>::max());

            l_layout.m_boundsMIN = TypeAlias::Math::Vector3(std::numeric_limits<float>::max());
            l_layout.m_meshCount = a_modelMeshList.size();

            for (const auto& l_modelMesh : a_modelMeshList)
            {
                for (const auto& l_modelVertex : l_modelMesh.m_modelVertexList)
                {
                    l_layout.m_boundsMIN = TypeAlias::Math::Vector3::Min(l_layout.m_boundsMIN, l_modelVertex.m_position);
                    l_boundsMAX          = TypeAlias::Math::Vector3::Max(l_boundsMAX,          l_modelVertex.m_position);
                }

                l_layout.m_triangleCount += l_modelMesh.m_indexList.size() / Constant::k_triangleVertexCount;
                l_layout.m_meshletCount  += l_modelMesh.m_modelMeshletData.m_meshletList.size();
                l_layout.m_maxLODCount    = std::max(l_layout.m_maxLODCount, l_modelMesh.m_modelMeshletData.m_lodList.size());
            }

            // 頂点が無い場合は、大きさ0のまま返す(呼び出し側でエラーにする)
            if (l_layout.m_boundsMIN.x > l_boundsMAX.x) { return l_layout; }

            l_layout.m_boundsSize = l_boundsMAX - l_layout.m_boundsMIN;
            l_layout.m_spacing    = std::max(l_layout.m_boundsSize.x, l_layout.m_boundsSize.z) * k_performanceTestInstanceSpacingScale;
            l_layout.m_gridWidth  = l_layout.m_spacing * static_cast<float>(k_performanceTestInstanceCountX - 1ULL);
            l_layout.m_gridDepth  = l_layout.m_spacing * static_cast<float>(k_performanceTestInstanceCountZ - 1ULL);

            // LOD数が少ないMeshは、そのMeshの一番粗いLODの三角形数で数える
            for (std::size_t l_lodIndex = 0ULL; l_lodIndex < l_layout.m_maxLODCount; ++l_lodIndex)
            {
                std::size_t l_lodTriangleCount = 0ULL;

                for (const auto& l_modelMesh : a_modelMeshList)
                {
                    const auto& l_lodList = l_modelMesh.m_modelMeshletData.m_lodList;

                    if (l_lodList.empty()) { continue; }

                    l_lodTriangleCount += l_lodList[std::min(l_lodIndex, l_lodList.size() - 1ULL)].m_triangleCount;
                }

                l_layout.m_lodTriangleCountList.emplace_back(l_lodTriangleCount);
            }

            return l_layout;
        }

        inline static const std::filesystem::path k_performanceTestModelFilePath = "Asset/Model/Antike.fbx";

        static constexpr PerformanceTestModelKind k_performanceTestModelKind = PerformanceTestModelKind::Static;

        static constexpr float k_performanceTestMotionDurationSecond    = 2.0F;
        static constexpr float k_performanceTestMotionAmplitudeRadian   = 0.1F;
        static constexpr float k_performanceTestMotionBonePhaseRadian   = 0.5F;

        static constexpr float k_performanceTestAnimationTimeOffsetSecond = 0.37F;

        static constexpr std::size_t k_initialExecutionLevel = 0ULL;

        static constexpr std::size_t k_performanceTestInstanceCountX = 25ULL;
        static constexpr std::size_t k_performanceTestInstanceCountZ = 20ULL;

        static constexpr float k_performanceTestInstanceSpacingScale = 1.5F;

        static constexpr float k_performanceTestCameraHeightScale       = 3.0F;
        static constexpr float k_performanceTestCameraLookAtHeightScale = 0.5F;

        static constexpr float k_performanceTestCameraBackOffsetCount = 2.0F;

        static constexpr float k_performanceTestCameraNearClipScale = 0.01F;
        static constexpr float k_performanceTestCameraFarClipScale  = 1.5F;

        static constexpr float k_performanceTestWorldMaxScale = 1.0F;

        static constexpr float k_performanceTestOccluderScale = 12.0F;

        static constexpr bool k_isPerformanceTestOccluderEnabled = false;

        static constexpr float k_half = 0.5F;

        std::vector<std::shared_ptr<GameObject>>            m_gameObjectList               = {};
        std::vector<std::shared_ptr<GameObject>>            m_pendingAddGameObjectList     = {};
        std::vector<std::vector<std::weak_ptr<GameObject>>> m_gameObjectExecutionLevelList = {};

        // 負荷テストの描画申請データ(描画申請側はweak_ptrで参照するため、ここで所有しておく)
        std::vector<std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>>          m_performanceTestDrawRequestDataList         = {};
        std::vector<std::shared_ptr<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>> m_performanceTestSkeletalDrawRequestDataList = {};

        UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

        SceneChanger                m_sceneChanger           = {};
        SceneGameObjectPrefabSystem m_gameObjectPrefabSystem = {};

        Converter::SceneJsonConverter m_jsonConverter = {};

        Graphics::LightSystem m_lightSystem = {};

        Graphics::StaticModel            m_performanceTestStaticModel            = {};
        Graphics::SkeletalAnimationModel m_performanceTestSkeletalAnimationModel = {};
        Graphics::Camera                 m_performanceTestCamera                 = {};

        std::string m_name = {};

        boost::uuids::uuid m_nextSceneUUID = {};
    };
}