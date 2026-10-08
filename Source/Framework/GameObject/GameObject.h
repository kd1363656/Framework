#pragma once

namespace FWK
{
    class GameObject final : public std::enable_shared_from_this<GameObject>
    {
    public:

         GameObject() = default;
        ~GameObject() = default;

        void INIT();

        void DeserializeScene(const nlohmann::json&              a_rootJson,
                              const nlohmann::json&              a_baseJson,
                              const SceneGameObjectPrefabSystem& a_prefabSystem,
                                    Scene&                       a_scene);

        void DeserializePrefab(const nlohmann::json& a_rootJson, const SceneGameObjectPrefabSystem& a_prefabSystem, Scene& a_scene);

        void PostDeserialize();

        void EarlyUpdate   () const;
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;


        nlohmann::json Serialize     (      SceneGameObjectPrefabSystem& a_prefabSystem)                                          const;
        nlohmann::json SerializeScene(      SceneGameObjectPrefabSystem& a_prefabSystem)                                          const;
        nlohmann::json SerializeDiff (const nlohmann::json&              a_baseJson, SceneGameObjectPrefabSystem& a_prefabSystem) const;

        void Destroy();

        std::shared_ptr<GameObject> Clone(const std::weak_ptr<GameObject>& a_newParent,
                                                Scene&                     a_scene,
                                          const boost::uuids::uuid&        a_prefabHierarchyNodeUUID = {},
                                                bool                       a_isCloneSubtreeRoot      = true) const;

        void ConvertToPrefab(const boost::uuids::uuid& a_prefabUUID);

        void DetachFromPrefab();

        void ClearAllPrefabRemovedUUIDSet();

        void SetName(const std::string& a_set) { m_name = a_set; }

        void SetPrefabUUID             (const boost::uuids::uuid& a_set) { m_prefabUUID              = a_set; }
        void SetPrefabHierarchyNodeUUID(const boost::uuids::uuid& a_set) { m_prefabHierarchyNodeUUID = a_set; }
        void SetSceneInstanceUUID      (const boost::uuids::uuid& a_set) { m_sceneInstanceUUID       = a_set; }

        void SetIsPrefabOrigin(const bool a_set) { m_isPrefabOrigin = a_set; }

        bool FetchVALIsPrefabInternalChild() const;

        const auto& GetREFHierarchy         () const { return m_hierarchy; }
        const auto& GetREFComponentContainer() const { return m_componentContainer; }

        const auto& GetREFName() const { return m_name; }

        const auto& GetREFPrefabUUID             () const { return m_prefabUUID; }
        const auto& GetREFPrefabHierarchyNodeUUID() const { return m_prefabHierarchyNodeUUID; }
        const auto& GetREFSceneInstanceUUID      () const { return m_sceneInstanceUUID; }

        auto& GetMutableREFHierarchy         () { return m_hierarchy; }
        auto& GetMutableREFComponentContainer() { return m_componentContainer; }

        std::weak_ptr<GameObjectTransformComponent> GetVALTransformComponent() const { return m_transformComponent; }

        bool GetVALIsDestroyed   () const { return m_isDestroyed; }
        bool GetVALIsPrefabOrigin() const { return m_isPrefabOrigin; }

    private:

        // ポインタの共有したいが絶対に存在すべきコンポーネントなのでメンバイニシャライザで生成
        std::shared_ptr<GameObjectTransformComponent> m_transformComponent = std::make_shared<GameObjectTransformComponent>();

        std::string m_name = {};

        GameObjectHierarchy          m_hierarchy          = {};
        GameObjectComponentContainer m_componentContainer = {};

        Converter::GameObjectJsonConverter m_jsonConverter = {};

        // このゲームオブジェクトが子でPrefabの内部にある子である場合
        // デシリアライズで復元時に子配列のどの部分に当たる子かを示すために
        // boost::uuids::uuid m_prefabHierarchyNodeUUIDを使用
        boost::uuids::uuid m_prefabUUID              = {};
        boost::uuids::uuid m_prefabHierarchyNodeUUID = {};
        boost::uuids::uuid m_sceneInstanceUUID       = {};

        bool m_isDestroyed    = false;
        bool m_isPrefabOrigin = Constant::k_gameObjectInitialValueIsPrefabOrigin;
    };
}