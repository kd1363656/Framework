#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK
{
    class ComponentBase
    {
    public:

                 ComponentBase() = default;
        virtual ~ComponentBase() = default;

        virtual void INIT() { /*必要に応じてオーバーライドしてください*/ };

        // UUIDのデシリアライズ、シリアライズの書き忘れが発生しないように関数を分けておく
        // こうすると派生クラスで書き直す必要がない
                void DeserializePrefabUUID(const nlohmann::json& a_rootJson);
        virtual void DeserializePrefab    (const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };
        virtual void DeserializeScene     (const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };

        virtual void PostDeserialize() { /*必要に応じてオーバーライドしてください*/ };

        virtual void EarlyUpdate   () { /*必要に応じてオーバーライドしてください*/ };
        virtual void Update        () { /*必要に応じてオーバーライドしてください*/ };
        virtual void LateUpdate    () { /*必要に応じてオーバーライドしてください*/ };
        virtual void PostLateUpdate() { /*必要に応じてオーバーライドしてください*/ };

        virtual void EditInspector() { /*必要に応じてオーバーライドしてください*/ };

                nlohmann::json SerializePrefabUUID();
        virtual nlohmann::json SerializePrefab    () { return {}; }
        virtual nlohmann::json SerializeScene     () { return {}; }

        virtual bool IsAllowMultiple() const { return false; }

        void Enable ();
        void Disable();

        void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

        void SetUUID(const boost::uuids::uuid& a_set) { m_uuid = a_set; }

        void SetIsPrefabOrigin   (const bool a_set) { m_isPrefabOrigin     = a_set; }
        void SetMarkedForRemoval (const bool a_set) { m_isMarkedForRemoval = a_set; }

        const auto& GetREFOwner() const { return m_owner; }

        const auto& GetREFUUID() const { return m_uuid; }

        auto& GetMutableREFUUID() { return m_uuid; }

        bool GetVALIsDisable         () const { return m_isDisable; }
        bool GetVALIsPrefabOrigin    () const { return m_isPrefabOrigin; }
        bool GetVALIsMarkedForRemoval() const { return m_isMarkedForRemoval; }

    private:

        std::weak_ptr<GameObject> m_owner = {};

        Converter::ComponentBaseJsonConverter m_jsonConverter = {};

        boost::uuids::uuid m_uuid = {};

        bool m_isDisable          = false;
        bool m_isPrefabOrigin     = false;
        bool m_isMarkedForRemoval = false;

        FWK_DEFINE_TYPE_INFO_ROOT(ComponentBase)
    };
}