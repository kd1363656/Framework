#pragma once

namespace FWK
{
    class GameObject final : std::enable_shared_from_this<GameObject>
    {
    public:

         GameObject() = default;
        ~GameObject() = default;
    
        void INIT();

        void Deserialize(const nlohmann::json& a_rootJson);

        void PostDeserialize();

        void EarlyUpdate   () const;
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;

        void Destroy();

        void EditInspector();

        nlohmann::json Serialize() const;

        void SetName(const std::string& a_set) { m_name = a_set; }

        void SetPrefabUUID       (const boost::uuids::uuid& a_set) { m_prefabUUID        = a_set; }
        void SetPrefabHierarchyNodeUUID(const boost::uuids::uuid& a_set) { m_prefabHierarchyNodeUUID = a_set; }
        void SetSceneInstanceUUID(const boost::uuids::uuid& a_set) { m_sceneInstanceUUID = a_set; }

        void SetIsPrefabOrigin(const bool a_set) { m_isPrefabOrigin = a_set; }

        const auto& GetREFHierarchy() const { return m_hierarchy; }

        const auto& GetREFName() const { return m_name; }

        const auto& GetREFPrefabUUID             () const { return m_prefabUUID; }
        const auto& GetREFPrefabHierarchyNodeUUID() const { return m_prefabHierarchyNodeUUID; }
        const auto& GetREFSceneInstanceUUID      () const { return m_sceneInstanceUUID; }

        auto& GetMutableREFHierarchy() { return m_hierarchy; }

        bool GetVALIsDestroyed   () const { return m_isDestroyed; }
        bool GetVALIsPrefabOrigin() const { return m_isPrefabOrigin; }

        std::weak_ptr<GameObjectTransformComponent> GetVALTransformComponent() const { return m_transformComponent; }

    private:
    
        // ポインタの共有したいが絶対に存在すべきコンポーネントなのでメンバイニシャライザで生成
        std::shared_ptr<GameObjectTransformComponent> m_transformComponent = std::make_shared<GameObjectTransformComponent>();

        GameObjectHierarchy          m_hierarchy          = {};
        GameObjectComponentContainer m_componentContainer = {};

        Converter::GameObjectJsonConverter m_jsonConverter = {};

        std::string m_name = {};

        // このゲームオブジェクトが子でPrefabの内部にある子である場合
        // デシリアライズで復元時に子配列のどの部分に当たる子かを示すために
        // boost::uuids::uuid m_prefabHierarchyNodeUUIDを使用
        boost::uuids::uuid m_prefabUUID              = {};
        boost::uuids::uuid m_prefabHierarchyNodeUUID = {};
        boost::uuids::uuid m_sceneInstanceUUID       = {};

        bool m_isDestroyed    = false;
        bool m_isPrefabOrigin = false;
    };
}