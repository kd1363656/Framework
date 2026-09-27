#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK
{
    class GameObjectComponentBase
    {
    public:

                 GameObjectComponentBase() = default;
        virtual ~GameObjectComponentBase() = default;

        virtual void INIT();

        virtual void Deserialize(const nlohmann::json& a_rootJson);
        
        virtual void PostDeserialize() { /*必要に応じてオーバーライドしてください*/ };

        virtual void EarlyUpdate   () { /*必要に応じてオーバーライドしてください*/ };
        virtual void Update        () { /*必要に応じてオーバーライドしてください*/ };
        virtual void LateUpdate    () { /*必要に応じてオーバーライドしてください*/ };
        virtual void PostLateUpdate() { /*必要に応じてオーバーライドしてください*/ };

        virtual void EditInspector() { /*必要に応じてオーバーライドしてください*/ };

        virtual nlohmann::json Serialize() const;

        virtual std::shared_ptr<GameObjectComponentBase> Clone() const = 0;

        virtual bool IsAllowMultiple() const { return false; }

        void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

        void SetUUID(const boost::uuids::uuid& a_set) { m_uuid = a_set; }

        void SetIsDisable     (const bool a_set) { m_isDisable      = a_set; }
        void SetIsPrefabOrigin(const bool a_set) { m_isPrefabOrigin = a_set; }

        const auto& GetREFOwner() const { return m_owner; }

        const auto& GetREFUUID() const { return m_uuid; }

        auto& GetMutableREFUUID() { return m_uuid; }

        bool GetVALIsDisable     () const { return m_isDisable; }
        bool GetVALIsPrefabOrigin() const { return m_isPrefabOrigin; }
        
    private:

        std::weak_ptr<GameObject> m_owner = {};

        Converter::GameObjectComponentBaseJsonConverter m_jsonConverter = {};

        boost::uuids::uuid m_uuid = {};

        bool m_isDisable      = Constant::k_gameObjectComponentBaseInitialValueIsDisable;
        bool m_isPrefabOrigin = Constant::k_gameObjectComponentBaseInitialValueIsPrefabOrigin;

        FWK_DEFINE_TYPE_INFO_ROOT(GameObjectComponentBase)
    };
}