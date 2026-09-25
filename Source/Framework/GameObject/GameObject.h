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
        void SetSceneInstanceUUID(const boost::uuids::uuid& a_set) { m_sceneInstanceUUID = a_set; }

        const auto& GetREFName() const { return m_name; }

        const auto& GetREFPrefabUUID       () const { return m_prefabUUID; }
        const auto& GetREFSceneInstanceUUID() const { return m_sceneInstanceUUID; }


        bool GetVALIsDestroyed() const { return m_isDestroyed; }

        std::weak_ptr<TransformComponent> GetVALTransformComponent() const { return m_transformComponent; }

    private:
    
        // ポインタの共有したいが絶対に存在すべきコンポーネントなのでメンバイニシャライザで生成
        std::shared_ptr<TransformComponent> m_transformComponent = std::make_shared<TransformComponent>();

        Converter::GameObjectJsonConverter m_jsonConverter = {};

        std::string m_name = {};

        boost::uuids::uuid m_prefabUUID        = {};
        boost::uuids::uuid m_sceneInstanceUUID = {};

        bool m_isDestroyed = false;
    };
}