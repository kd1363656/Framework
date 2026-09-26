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

        virtual void INITBase();
        virtual void INIT    () { /*必要に応じてオーバーライドしてください*/ };

        virtual void Deserialize(const nlohmann::json& a_rootJson);
        
        virtual void PostDeserialize() { /*必要に応じてオーバーライドしてください*/ };

        virtual void EarlyUpdate   () { /*必要に応じてオーバーライドしてください*/ };
        virtual void Update        () { /*必要に応じてオーバーライドしてください*/ };
        virtual void LateUpdate    () { /*必要に応じてオーバーライドしてください*/ };
        virtual void PostLateUpdate() { /*必要に応じてオーバーライドしてください*/ };

        virtual void EditInspector() { /*必要に応じてオーバーライドしてください*/ };

        virtual nlohmann::json Serialize() const;

        virtual std::shared_ptr<ComponentBase> Clone() const = 0;

        virtual bool IsAllowMultiple() const { return false; }

        void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

        void SetUUID(const boost::uuids::uuid& a_set) { m_uuid = a_set; }

        void SetIsDisable      (const bool a_set) { m_isDisable       = a_set; }
        void SetIsSerializeSkip(const bool a_set) { m_isSerializeSkip = a_set; }

        const auto& GetREFOwner() const { return m_owner; }

        const auto& GetREFUUID() const { return m_uuid; }

        auto& GetMutableREFUUID() { return m_uuid; }

        bool GetVALIsDisable      () const { return m_isDisable; }
        bool GetVALIsSerializeSkip() const { return m_isSerializeSkip; }
        
    private:

        std::weak_ptr<GameObject> m_owner = {};

        Converter::ComponentBaseJsonConverter m_jsonConverter = {};

        boost::uuids::uuid m_uuid = {};

        bool m_isDisable       = Constant::k_componentBaseInitialDisable;
        bool m_isSerializeSkip = Constant::k_componentBaseInitialSerializeSkip;
        
        FWK_DEFINE_TYPE_INFO_ROOT(ComponentBase)
    };
}